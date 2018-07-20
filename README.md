Websocket stuff is in the aptly named corresponding folder.

To run the socket on MacOS, simply type `python3 driver.py` in Terminal. To stop it, you'll have to Ctrl+C stop the
process.

Linux is similar I think.

To run it on Windows, I think the command should be `py -3 driver.py` to start it, and the same as Linux/Unix  to stop it.

Python ver. 3.7 or something
List of dependencies (use `pip` or `pip3` to install):

1. webSocketApp
https://pypi.org/project/websocket-client/
websocket library: pip package name is `websocket-client`

2. python-dateutil
https://pypi.org/project/python-dateutil/
dateutil (parsing) library: pip package name is `python-dateutil`

Backtester stuff will be in `backtester_stuff` and will be entirely C++ at least for the time being (CLI is fine).
Dependencies for Backtester include Boost for its high precision decimal class. It's also just a great library overall, so install it.
