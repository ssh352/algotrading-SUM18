# websocket dependency VVV
# https://pypi.org/project/websocket-client/

# Dependencies installation notes:
# websocket library: pip package name is "websocket-client"
# dateutil (parsing) library: pip package name is "python-dateutil"

import time
import logging
import os
import signal
import sys

from datetime import date as d
from datetime import datetime as dt
from dateutil import parser as dparser
from CBP_adaptions import CoinbaseProAdaptedWS, COINBASE_PRO_MAX_QUERY_FREQ
from json import dumps, loads

# defining the root_logger log instance as global so that we don't need to pass it around a bunch, bad style? maybe
global root_logger

def sig_handler(sig, frame):
    """
    :param signal: signal, ie SIGTERM, SIGABRT, etc. (note you can't actually catch SIGKILL)
    :param frame: the current stackframe at signal receive time
    :return: 
    """
    # this is an example of the new special formatted strings in python
    logging.warning(f"Possible loss of connection, exiting with signal {signal.Signals(sig).name}")
    sys.exit(1)


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


def on_message(ws: CoinbaseProAdaptedWS, message: str):
    j = loads(message)  # JSON style object represented as a Python dictionary (hashmap)
    t = j["type"]  # the type of message we are sent, reference Coinbase docs for list of possible values

    # Close current files and open new ones for a new day once midnight comes
    if t in ws.full_msg_types and dparser.parse(j["time"]).day != ws.save_date.day:
        logging.warning("Msg day differs from save day, attempting rollover")
        n = dt.utcnow()  # the current datetime, "now"
        ws.save_date = d(n.year, n.month, n.day)
        # We have two log handlers, the one printing to stderr and the one printing to our file, we want to roll the
        # file handler over as well just so that each log file doesn't blow up in size, and so we can safely check
        # past days' logs (the file will appear blank until python is done writing to it)
        for handler in root_logger.handlers:
            if isinstance(handler, logging.FileHandler):
                root_logger.removeHandler(handler)
        root_logger.addHandler(logging.FileHandler(f"{n.strftime('%Y%m%d')}.log"))
        # closing and opening new files for each of the four message types for each of the currencies we sub to
        # returns a list of (key, value) tuples, for f_dict, tuples will be (currency_name, dictionary)
        for sym, dic in ws.f_dict.items():
            # dic.items() returns a list of (key, value) tuples, for dic, the tuples will be (msg_type, file_obj)
            for k, f in dic.items():
                f.close()
            if not os.path.exists("{s}/{d}".format(s=sym, d=ws.save_date.strftime("%Y%m%d"))):
                os.mkdir("{s}/{d}".format(s=sym, d=ws.save_date.strftime("%Y%m%d")))
            # for each message type m, let the dictionary for each currency be the set {m : file_obj}
            ws.f_dict[sym] = {m: open("{symbol}/{date}/CBP_{symbol}_full_{m}_{date}.csv".format(
                    m=m, symbol=sym, date=ws.save_date.strftime("%Y%m%d")), "a")
                    for m in ws.full_msg_types
                }

    if t == "heartbeat":
        logging.info("heartbeat")
        if type(ws.last_heartbeat) == float:
            ws.last_heartbeat = time.time()
        else:
            ws.last_heartbeat.value = time.time()
    elif t == "l2update":
        # logging.info("G5")
        for change in j["changes"]:
            # save each update in corresponding csv, formatting with trailing \n via print()
            print(",".join([str(i) for i in change]), file=ws.f_dict[j["product_id"]])
    elif t == "received":
        # logging.info("G1")
        ws.handle_received(j)
    elif t == "open":
        # logging.info("G2")
        ws.handle_open(j)
    elif t == "done":
        # logging.info("G3")
        ws.handle_done(j)
    elif t == "match":
        # logging.info("G4")
        ws.handle_match(j)
    # pprint(j)


def on_error(ws: CoinbaseProAdaptedWS, error: BaseException):
    logging.warning(error)
    time.sleep(COINBASE_PRO_MAX_QUERY_FREQ)
    if not isinstance(error, KeyboardInterrupt) and not isinstance(error, SystemExit):
        logging.warning("Attempting reconnect")
    else:
        logging.warning("Interrupt caught")


def on_close(ws: CoinbaseProAdaptedWS):
    logging.info("Coinbase connection closed")


def main(**kwargs):
    # registering the handler to handle killing of child
    signal.signal(signal.SIGTERM, sig_handler)

    logname = f"{dt.utcnow().strftime('%Y%m%d')}.log"
    logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"),
                        format="%(asctime)s|%(levelname)s|%(message)s",
                        filename=logname, filemode="a",
                        datefmt="%H:%M:%S")
    root_logger = logging.getLogger()
    root_logger.addHandler(logging.StreamHandler())
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
    try:
        while True:
            logging.info("Entered run loop")
            ws.run_forever()
    except (KeyboardInterrupt, SystemExit) as e:
        logging.info("{} Interrupt processed, exiting...".format(type(e).__name__))
        on_close(ws=ws)


if __name__ == "__main__":
    main()