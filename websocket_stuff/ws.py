import time
import logging
import lzma
import os
import signal
import sys
import shutil
from datetime import date as d
from datetime import datetime as dt
from dateutil import parser as dparser
from CBP_adaptions import CoinbaseProAdaptedWS, COINBASE_PRO_MAX_QUERY_FREQ
from json import dumps, loads

# defining the root_logger log instance as global so that we don't need to pass it around a bunch, bad style? maybe
global root_logger


# detects signals from the process and logs accordingly
def sig_handler(sig, frame):
    """
    :param signal: signal, ie SIGTERM, SIGABRT, etc. (note you can't actually catch SIGKILL)
    :param frame: the current stackframe at signal receive time
    :return:
    """
    # this is an example of the new special formatted strings in python
    logging.warning(f"Possible loss of connection, exiting with signal {signal.Signals(sig).name}")
    sys.exit(1)

# here we are going to specify the two (or more) channels that we
# want to subscribe to, where subtype is "level 2" currently
# and send that info to our websocket
def on_open(ws: CoinbaseProAdaptedWS):
    params = {
        "type": "subscribe",
        "channels": [
            {
                "name": ws.subtype,
                "product_ids":
                    ws.symbols
            },
            {
                "name": "heartbeat",
                "product_ids": [
                    "ETH-USD"
                ],
            },
        ]
    }
    ws.send(dumps(params))
    logging.info("Coinbase connection opened")


def transfer_folder_to_bucket(folder_name, bucket_name):
    s3 = boto3.resource("s3")
    bucket = s3.Bucket(bucket_name)
    for root, dirs, files in os.walk(folder_name):  # I'm honestly not entirely sure how os.walk works, sorry guys
        for file in files:
            full_path = os.path.join(root, file)
            with open(full_path, 'rb') as dat:
                print(f"uploading {'full' + full_path[len(folder_name):]}")
                bucket.put_object(Key="full" + full_path[len(folder_name):], Body=dat)


def upload_files(ws: CoinbaseProAdaptedWS):
    os.chdir(ws.SOCKET_PATH)
    os.chdir("full")
    FULL_PATH = os.getcwd()
    pairs = [file for file in os.listdir() if os.path.isdir(file)]
    print(pairs)
    for pair in pairs:
        os.chdir(os.path.join(FULL_PATH, pair))
        date_dirs = [obj for obj in os.listdir() if os.path.isdir(obj)]
        for folder in date_dirs:
            print(folder)
            transfer_folder_to_bucket(folder, "cryptoorderbookdata")
            shutil.rmtree(folder)

def manage_directories(ws: CoinbaseProAdaptedWS):
    # We have two log handlers, the one printing to stderr and the one printing to our file,
    # we want to roll the file handler over as well just so that each log file doesn't blow up
    # in size, and so we can safely check past days' logs
    # (the file will appear blank until python is done writing to it)
    os.chdir(ws.SOCKET_PATH)
    for handler in root_logger.handlers:
        if isinstance(handler, logging.FileHandler):
            root_logger.removeHandler(handler)
    root_logger.addHandler(logging.FileHandler(f"full/{now.strftime('%Y%m%d')}.log"))

    # closing and opening new files for each of the four message types for each of the
    # currencies we sub to returns a list of (key, value) tuples, for f_dict,
    # tuples will be (currency_name, dictionary)
    for sym, dic in ws.f_dict.items():

        # dic.items() returns a list of (key, value) tuples,
        # for dic, the tuples will be (msg_type, file_obj)
        for k, f in dic.items():
            f.close()

        # making sure that we don't accidentally create a subdirectory full/full
        if not os.path.exists("{s}/{d}".format(s=sym, d=ws.save_date.strftime("%Y%m%d"))):
            os.mkdir("{s}/{d}".format(s=sym, d=ws.save_date.strftime("%Y%m%d")))

        # for each message type m, let the dictionary for each currency be the set {m : file_obj}
        ws.f_dict[sym] = {m: lzma.open("{symbol}/{date}/CBP_{symbol}_full_{m}_{date}_{midday}.xz".format(
            m=m, symbol=sym, date=ws.save_date.strftime("%Y%m%d"), midday=ws.midday), "a", preset=7)
            for m in ws.full_msg_types
        }

def on_message(ws: CoinbaseProAdaptedWS, message: str):

    # JSON style object represented as a Python dictionary (hashmap)
    json_res = loads(message)

    # the type of message we are sent, reference Coinbase docs for list of possible values
    message_type = json_res["type"]

    # if we receive a valid message type and the date differs from the most recent date
    # Close current files and open new ones for a new day once midnight comes
    now = dt.utcnow()  # the current datetime, "now"
    if message_type in ws.full_msg_types and ws.midday == 0 and now.hour > 12:
        logging.warning("Msg time past midday, attempting rollover")
        ws.midday = 1
        upload_files(ws)
        manage_directories(ws)
    elif message_type in ws.full_msg_types and dparser.parse(json_res["time"]).day != ws.save_date.day:
        logging.warning("Msg day differs from save day, attempting rollover")
        ws.midday = 0
        upload_files(ws)
        # store date for next roll-over
        ws.save_date = d(now.year, now.month, now.day)
        manage_directories(ws)
    # if message type was a heartbeat we want to store the time for referencing later
    if message_type == "heartbeat":
        logging.info("heartbeat")
        if type(ws.last_heartbeat) == float:
            ws.last_heartbeat = time.time()
        else:
            ws.last_heartbeat.value = time.time()

    # if the update is from the level 2 channel we want to store the data in the correct csv
    elif message_type == "l2update":
        # logging.info("G5")
        for change in json_res["changes"]:
            # TODO comment and remove prev
            # save each update in corresponding csv, formatting with trailing \n via print()
            print(",".join([str(i) for i in change]), file=ws.f_dict[json_res["product_id"]])
            ws.f_dict[json_res["product_id"]].write((",".join([str(i) for i in change]) + "\n").encode("utf-8"))

    # for info about the handling of the following messages see declarations in CBP_adaptions.py
    elif message_type == "received":
        # logging.info("G1")
        ws.handle_received(json_res)

    elif message_type == "open":
        # logging.info("G2")
        ws.handle_open(json_res)

    elif message_type == "done":
        # logging.info("G3")
        ws.handle_done(json_res)

    elif message_type == "match":
        # logging.info("G4")
        ws.handle_match(json_res)
    # pprint(json_res)


# logs any errors that are encountered in our file/in the console
def on_error(ws: CoinbaseProAdaptedWS, error: BaseException):
    logging.warning(error)
    time.sleep(COINBASE_PRO_MAX_QUERY_FREQ)
    if not isinstance(error, KeyboardInterrupt) and not isinstance(error, SystemExit):
        logging.warning("Attempting reconnect")
    else:
        logging.warning("Interrupt caught")

# log whenever the connection is terminated
def on_close(ws: CoinbaseProAdaptedWS):
    logging.info("Coinbase connection closed")


def main(**kwargs):
    # registering the handler to handle killing of child
    signal.signal(signal.SIGTERM, sig_handler)

    # if we do not have a "full" output directory create it
    if not os.path.exists("full"):
        os.mkdir("full")

    # name log using current time
    logname = f"full/{dt.utcnow().strftime('%Y%m%d')}.log"

    # sets the extent of the logging (level) we want to do
    # the format in which we will write, mode (in this case 'a' means appending and writing)
    # and finally the date format
    logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"),
                        format="%(asctime)s|%(levelname)s|%(message)s",
                        filename=logname,
                        datefmt="%H:%M:%S")

    # we now want to make this logger we have set up relavant so it is accessible
    # in all of our classes (because petur made a global variable :( )
    root_logger = logging.getLogger()

    # we specify the handler (in this case stream handler because we are writing)
    root_logger.addHandler(logging.StreamHandler())

    # we create an instance of our websocket
    # the **kwargs here unpacks any keyword arguments we passed as a dictionary (hashmap) to main()
    ws = CoinbaseProAdaptedWS(url="wss://ws-feed.pro.coinbase.com",
                              on_open=on_open,
                              on_message=on_message,
                              on_error=on_error,
                              on_close=on_close,
                              symbols=["BTC-USD", "ETH-USD", "LTC-USD", "BCH-USD"],
                              subtype="full",
                              **kwargs)
    logging.info("Socket initialized")

    # we want the websocket to call the method run_forever
    # which will take care of handling exceptions should we have downtime
    try:
        while True:
            logging.info("Entered run loop")
            ws.run_forever()
    except (KeyboardInterrupt, SystemExit) as e:
        logging.info("{} Interrupt processed, exiting...".format(type(e).__name__))
        on_close(ws=ws)


if __name__ == "__main__":
    main()
