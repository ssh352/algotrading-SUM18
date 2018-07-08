#stackoverflow link I used VVV
#https://stackoverflow.com/questions/47999982/websocket-to-useable-data-in-python-get-price-from-gdax-websocket-feed
#websocket dependency VVV
#https://pypi.org/project/websocket-client/

from websocket import WebSocketApp
from json import dumps, loads
from pprint import pprint

def on_open(ws):
	params = {
		"type": "subscribe",
		"product_ids": [
			"BTC-USD",
			"ETH-USD"
		],
		"channels": [
			"level2",
			"heartbeat",
			{
				"name": "ticker",
				"product_ids": [
					"BTC-USD",
					"ETH-USD"
				]
			}
		]
	}
	ws.send(dumps(params))

	print("######### Coinbase connection opened #########")

def on_message(ws, message):
	pprint(loads(message))

def on_error(ws, error):
	print(error)

def on_close(ws):
	print("######### Coinbase connection closed #########")

if __name__ == "__main__":
	ws = WebSocketApp("wss://ws-feed.pro.coinbase.com",
								on_open = on_open,
								on_message = on_message, 
								on_error = on_error,
								on_close = on_close)
	ws.run_forever()
