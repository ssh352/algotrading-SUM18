# websocket dependency VVV
# https://pypi.org/project/websocket-client/

import time
import logging
import os

from datetime import date as d
from datetime import datetime as dt
from dateutil import parser as dparser
from CBP_adaptions import CoinbaseProAdaptedWS, COINBASE_PRO_MAX_QUERY_FREQ
from json import dumps, loads
from pprint import pprint


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


# TODO make this automatically handle the change to output file on date rollover (ie file should change at 12AM UTC)
def on_message(ws: CoinbaseProAdaptedWS, message: str):
    j = loads(message)
    t = j["type"]
    # Close current files and open new ones for a new day once midnight comes
    if t in ws.full_msg_types:
        if dparser.parse(j["time"]).day != ws.save_date.day:
            n = dt.utcnow()
            ws.save_date = d(n.year, n.month, n.day)
            for sym, f in ws.f_dict:
                f.close()
                if not os.path.exists("{s}/{d}".format(s=sym, d=ws.save_date.strftime("%Y%m%d"))):
                    os.mkdir("{s}/{d}".format(s=sym, d=ws.save_date.strftime("%Y%m%d")))
                ws.f_dict = {
                    sym: {m: open("{symbol}/{date}/CBP_{symbol}_full_{m}_{date}.csv".format(
                        m=m, symbol=sym, date=ws.save_date.strftime("%Y%m%d")), "w")
                        for m in ws.full_msg_types
                    }
                    for sym in ws.symbols
                }
    if t == "heartbeat":
        ws.last_heartbeat = time.time()
    elif t == "l2update":
        for change in j["changes"]:
            # save each update in corresponding csv, formatting with trailing \n via print()
            print(",".join([str(i) for i in change]), file=ws.f_dict[j["product_id"]])
    elif t == "received":
        ws.handle_received(j)
    elif t == "open":
        ws.handle_open(j)
    elif t == "done":
        ws.handle_done(j)
    elif t == "match":
        ws.handle_match(j)
    pprint(j)


def on_error(ws: CoinbaseProAdaptedWS, error: BaseException):
    logging.warning(error)
    time.sleep(COINBASE_PRO_MAX_QUERY_FREQ)
    if not isinstance(error, KeyboardInterrupt) and not isinstance(error, SystemExit):
        logging.warning("Attempting reconnect")
    else:
        logging.warning("Interrupt caught")


def on_close(ws: CoinbaseProAdaptedWS):
    logging.info("Coinbase connection closed")


if __name__ == "__main__":
    logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"),
                        format="%(asctime)s;%(levelname)s;%(message)s")
    ws = CoinbaseProAdaptedWS(url="wss://ws-feed.pro.coinbase.com",
                              on_open=on_open,
                              on_message=on_message,
                              on_error=on_error,
                              on_close=on_close,
                              symbols=["BTC-USD", "ETH-USD", "LTC-USD", "BCH-USD"],
                              subtype="full")
    try:
        while True:
            ws.run_forever()
    except (KeyboardInterrupt, SystemExit) as e:
        logging.info("{} Interrupt processed, exiting...".format(type(e).__name__))