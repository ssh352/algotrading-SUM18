# stackoverflow link I used VVV
# https://stackoverflow.com/questions/47999982/websocket-to-useable-data-in-python-get-price-from-gdax-websocket-feed
# websocket dependency VVV
# https://pypi.org/project/websocket-client/

import time
import logging
import os

from CBP_adaptions import CoinbaseProAdaptedWS, COINBASE_PRO_MAX_QUERY_FREQ
from json import dumps, loads
from pprint import pprint


def on_open(ws: CoinbaseProAdaptedWS):
    if not ws.symbol:
        raise RuntimeError("You must set a symbol to subscribe to")  # TODO make this work for multiple symbols
    params = {
        "type": "subscribe",
        "product_ids": [
            ws.symbol
        ],
        "channels": [
            "level2",
            "heartbeat",
        ]
    }
    ws.send(dumps(params))

    logging.info("Coinbase connection opened")


# TODO make this automatically handle the change to output file on date rollover (ie file should change at 12AM UTC)
def on_message(ws: CoinbaseProAdaptedWS, message: str):
    j = loads(message)
    if j["type"] == "heartbeat":
        ws.last_heartbeat = time.time()
    elif j["type"] == "l2update":
        for change in j["changes"]:
            print(",".join([str(i) for i in change]), file=ws.f)  # save each update in csv format with trailing \n
    pprint(j)


def on_error(ws: CoinbaseProAdaptedWS, error):
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
                              on_close=on_close)
    ws.set_symbol("BTC-USD")
    try:
        while True:
            ws.run_forever()
    except (KeyboardInterrupt, SystemExit) as e:
        logging.info("{} Interrupt processed, exiting...".format(type(e).__name__))