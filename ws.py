# websocket dependency VVV
# https://pypi.org/project/websocket-client/

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
from pprint import pprint


def sig_handler(signal, frame):
    """
    :param signal: signal, ie SIGKILL, SIGTERM, SIGABRT, etc.
    :param frame: the current stackframe at signal receive time
    :return: 
    """
    # this is an example of the new special formatted strings in python
    logging.warning(f"Possible loss of connection, exiting with signal {signal.Signals(signal).name}")
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
    j = loads(message)
    t = j["type"]
    # Close current files and open new ones for a new day once midnight comes
    if t in ws.full_msg_types and dparser.parse(j["time"]).day != ws.save_date.day:
        logging.warning("Msg day differs from save day, attempting rollover")
        n = dt.utcnow()
        ws.save_date = d(n.year, n.month, n.day)
        for sym, dic in ws.f_dict.items():
            for k, f in dic.items():
                f.close()
            if not os.path.exists("{s}/{d}".format(s=sym, d=ws.save_date.strftime("%Y%m%d"))):
                os.mkdir("{s}/{d}".format(s=sym, d=ws.save_date.strftime("%Y%m%d")))
            ws.f_dict[sym] = {m: open("{symbol}/{date}/CBP_{symbol}_full_{m}_{date}.csv".format(
                    m=m, symbol=sym, date=ws.save_date.strftime("%Y%m%d")), "w")
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
    signal.signal(signal.SIGKILL, sig_handler)

    logname = "testing.log"  # Change this to whatever you want
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