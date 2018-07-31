import time
import logging
import lzma

from websocket import *
from datetime import datetime as dt
from datetime import date as d
from typing import List
import threading
import six
import os

global SOCKET_PATH

# Highest frequency we're allowed to query at before potential IP ban
COINBASE_PRO_MAX_QUERY_FREQ = 4


# Simple modification of the default run_forever() method
# transfers timeout handling from ping-pong to heartbeat messages
# (coinbase does not seem to support ping-pong)
class CoinbaseProAdaptedWS(WebSocketApp):
    def __init__(self, heartbeat_timeout=10, *args, **kwargs):
        self.SOCKET_PATH = os.getcwd()  # working directory of websocket, we'll use this to avoid nested dirs
        # see for explanation of arg/kwarg vvv
        # https://pythontips.com/2013/08/04/args-and-kwargs-in-python-explained/
        # check ws for passed in, all are kwargs
        if "symbols" not in kwargs:
            raise RuntimeError("You must provide the websocket with a list of symbols to subscribe to")
        if "subtype" not in kwargs:
            raise RuntimeError("You must provide the subscription type from: level2, full")
        if "shared_beat" in kwargs:
            self.last_heartbeat = kwargs["shared_beat"]
        else:
            self.last_heartbeat = time.time()

        # copy/set list of symbols and what level we want into instance
        self.symbols: List[str] = kwargs["symbols"]
        self.subtype: str = kwargs["subtype"]

        # these will be used to write the contents of the json response in correct order to csv
        self.rec_fields = ["type", "time", "product_id", "sequence", "order_id", "funds", "size", "price", "side",
                           "order_type"]
        self.open_fields = ["type", "time", "product_id", "sequence", "order_id", "price", "remaining_size", "side"]
        self.done_fields = ["type", "time", "product_id", "sequence", "price", "order_id", "reason", "side",
                            "remaining_size"]
        self.match_fields = ["type", "trade_id", "sequence", "maker_order_id", "taker_order_id", "time",
                             "product_id", "size", "price", "side"]

        # base class doesn't have symbols and subtype for required kwargs
        # delete unnecessary kwarg stuff before we init the base webSocketApp class
        if "shared_beat" in kwargs:
            del kwargs["shared_beat"]
        del kwargs["symbols"], kwargs["subtype"] 
        
        # init the base class to bind our on_open, on_message etc. functions
        super(CoinbaseProAdaptedWS, self).__init__(*args, **kwargs)
        
        # variables used in file saving
        utcnow = dt.utcnow()
        self.save_date = d(utcnow.year, utcnow.month, utcnow.day)
        self.heartbeat_timeout: int = heartbeat_timeout

        # initializing file dictionary based on level2 vs full
        # full is 2D because of the diff message types
        if self.subtype == "level2":
            if not os.path.exists("level2"):
                os.mkdir("level2")
                os.chdir("level2")
            for sym in self.symbols:
                if not os.path.exists(sym):
                    os.mkdir(sym)
            # for all symbols, open corresponding file write stream to csv file and store in f_dict
            self.f_dict = {sym: open("symbol/CBP_{symbol}_level2_{date}.csv".format(
                        symbol=sym, date=self.save_date.strftime("%Y%m%d")), "a") 
                        for sym in self.symbols}
        else:
            if not os.path.exists("full"):
                os.mkdir("full")
            os.chdir("full")
            self.full_msg_types = ["received", "open", "done", "match"]
            # Setting up folder structure st. each sym has a folder for each day containing each of the msg types
            for sym in self.symbols:
                if not os.path.exists(sym):
                    os.mkdir(sym)
                if not os.path.exists("{s}/{d}".format(s=sym, d=self.save_date.strftime("%Y%m%d"))):
                    os.mkdir("{s}/{d}".format(s=sym, d=self.save_date.strftime("%Y%m%d")))

            self.f_dict = {
                # lzma.open is like the regular open except we write compressed data, preset (0-9) is measure of
                # compression, with 9 as maximum compression at expense of CPU & RAM load
                sym: {m: open("{symbol}/{date}/CBP_{symbol}_full_{m}_{date}.csv".format(
                                 m=m, symbol=sym, date=self.save_date.strftime("%Y%m%d")), "a", 500)
                      for m in self.full_msg_types
                }
                for sym in self.symbols
            }
            # Writing the header for each csv
            for sym in self.symbols:
                for msg_type in self.full_msg_types:
                    self.f_dict[sym][msg_type].write((",".join(self.rec_fields) + "\n"))

    # handlers for each type of message that may be sent via the 'full' channel (excepting change, accept)
    def handle_received(self, json_obj):
        # handles messages from 'full' channel of type 'received'
        # json_obj: the JSON object received by the web socket, should be of type dict

        # attempting to access values by sequential key, if key doesn't exist, add empty value
        # loop stitch together all values returned in json_obj, print to correct file as stored in f_dict
        # the final encode call on the string converts str -> bytes which is what the LZMAFile expects
        self.f_dict[json_obj["product_id"]][json_obj["type"]].write(
            (",".join(["" if k not in json_obj else str(json_obj[k]) for k in self.rec_fields]) + "\n")
        )

    def handle_open(self, json_obj):
        # handles messages from 'full' channel of type 'open', see handle_received
        self.f_dict[json_obj["product_id"]][json_obj["type"]].write(
            (",".join([str(json_obj[k]) for k in self.open_fields]) + "\n")
        )

    def handle_done(self, json_obj):
        # handles messages from 'full' channel of type 'done', see handle_received
        self.f_dict[json_obj["product_id"]][json_obj["type"]].write(
            (",".join(["" if k not in json_obj else str(json_obj[k]) for k in self.done_fields]) + "\n")
        )

    def handle_match(self, json_obj):
        # handles messages from 'full' channel of type 'match', see handle_received
        self.f_dict[json_obj["product_id"]][json_obj["type"]].write(
            (",".join([str(json_obj[k]) for k in self.match_fields]) + "\n")
        )

    """
    run event loop for WebSocket framework.
    This loop is infinite loop and is alive during websocket is available.
    sockopt: values for socket.setsockopt.
        sockopt must be tuple
        and each element is argument of sock.setsockopt.
    sslopt: ssl socket optional dict.
    ping_interval: automatically send "ping" command
        every specified period(second)
        if set to 0, not send automatically.
    ping_timeout: timeout(second) if the pong message is not received.
    http_proxy_host: http proxy host name.
    http_proxy_port: http proxy port. If not set, set to 80.
    http_no_proxy: host names, which doesn't use proxy.
    skip_utf8_validation: skip utf8 validation.
    host: update host header.
    origin: update origin header.
    """
    def run_forever(self, sockopt=None, sslopt=None,
                    ping_interval=0, ping_timeout=None,
                    http_proxy_host=None, http_proxy_port=None,
                    http_no_proxy=None, http_proxy_auth=None,
                    skip_utf8_validation=False,
                    host=None, origin=None, dispatcher=None):
        # if ping_timeout is not none OR <= zero, set to none
        if not ping_timeout or ping_timeout <= 0:
            ping_timeout = None
        # to make sure the WS doesn't timeout before its ping interval
        if ping_timeout and ping_interval and ping_interval <= ping_timeout:
            raise WebSocketException("Ensure ping_interval > ping_timeout")
        if sockopt is None:
            sockopt = []
        if sslopt is None:
            sslopt = {}
        if self.sock:
            raise WebSocketException("socket is already opened")
        thread = None
        close_frame = None
        self.keep_running = True
        self.last_ping_tm = 0
        self.last_pong_tm = 0

        def teardown():
            if thread and thread.isAlive():
                event.set()
                thread.join()
            self.keep_running = False
            self.sock.close()
            close_args = self._get_close_args(
                close_frame.data if close_frame else None)
            self._callback(self.on_close, *close_args)
            self.sock = None

        try:
            self.sock = WebSocket(
                self.get_mask_key, sockopt=sockopt, sslopt=sslopt,
                fire_cont_frame=self.on_cont_message and True or False,
                skip_utf8_validation=skip_utf8_validation)
            self.sock.settimeout(getdefaulttimeout())
            self.sock.connect(
                self.url, header=self.header, cookie=self.cookie,
                http_proxy_host=http_proxy_host,
                http_proxy_port=http_proxy_port, http_no_proxy=http_no_proxy,
                http_proxy_auth=http_proxy_auth, subprotocols=self.subprotocols,
                host=host, origin=origin)
            if not dispatcher:
                dispatcher = self.create_dispatcher(ping_timeout)

            self._callback(self.on_open)
            # self.last_heartbeat = time.time()

            if ping_interval:
                event = threading.Event()
                thread = threading.Thread(
                    target=self._send_ping, args=(ping_interval, event))
                thread.setDaemon(True)
                thread.start()

            def read():
                if not self.keep_running:
                    return teardown()
                print("brecv")
                op_code, frame = self.sock.recv_data_frame(True)
                print("arecv")
                if op_code == ABNF.OPCODE_CLOSE:
                    close_frame = frame
                    return teardown()
                elif op_code == ABNF.OPCODE_PING:
                    self._callback(self.on_ping, frame.data)
                elif op_code == ABNF.OPCODE_PONG:
                    self.last_pong_tm = time.time()
                    self._callback(self.on_pong, frame.data)
                elif op_code == ABNF.OPCODE_CONT and self.on_cont_message:
                    self._callback(self.on_data, frame.data,
                                   frame.opcode, frame.fin)
                    self._callback(self.on_cont_message,
                                   frame.data, frame.fin)
                else:
                    data = frame.data
                    if six.PY3 and op_code == ABNF.OPCODE_TEXT:
                        data = data.decode("utf-8")
                    self._callback(self.on_data, data, frame.opcode, True)
                    self._callback(self.on_message, data)

                return True

            def check():
                if ping_timeout and self.last_ping_tm \
                        and time.time() - self.last_ping_tm > ping_timeout \
                        and self.last_ping_tm - self.last_pong_tm > ping_timeout:
                    raise WebSocketTimeoutException("ping/pong timed out")
                return True

            dispatcher.read(self.sock.sock, read, check)

        except (KeyboardInterrupt, SystemExit, Exception) as e:
            self._callback(self.on_error, e)
            if isinstance(e, SystemExit) or isinstance(e, KeyboardInterrupt):
                # propagate SystemExit further
                raise
            teardown()
