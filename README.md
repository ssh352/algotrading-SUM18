Backtester stuff will be in `backtester_stuff` and will be entirely C++ at least for the time being (CLI is fine).
Dependencies for Backtester include Boost for its high precision decimal class. It's also just a great library overall, so install it.

Websocket stuff is in the aptly named corresponding folder.

Running driver.py
    0. Check if any new dependencies have been added
    1. MacOS: Type `python3 driver.py` in Terminal. To stop it, you'll have to Ctrl+C stop the
    process.
    2. Linux: Similar to MacOS I think?
    3. Windows: I think the command should be `py -3 driver.py` to start it, and the same as Linux/Unix  to stop it.

We are using Python ver. 3.7?
List of dependencies (use `pip` or `pip3` to install):
1. webSocketApp
    https://pypi.org/project/websocket-client/
    websocket library: pip package name is `websocket-client`

2. python-dateutil
    https://pypi.org/project/python-dateutil/
    dateutil (parsing) library: pip package name is `python-dateutil`

3. Boto3
https://boto3.readthedocs.io/en/latest/
    AWS interfacing API: pip package name is 'boto3'
    Notes: Add AWS credentials to be able to use boto3 (for macs)
    1.Go to root directory, ~/ for macs
        a.For windows maybe? if c:/User/yourusername/
        b. See https://docs.aws.amazon.com/cli/latest/userguide/cli-config-files.html
    2. Create folder called .aws thru mkdir,
    windows has to do some funky shit because it doesnt allow folders that start with '.'
    3. Change directory into that muthafucka .aws
    4. Create a file called credentials via touch or some shit
        Write stuff between quotes into file, keys are below in 'Credentials for AWS'
        "
        [default]
        aws_access_key_id = YOUR_ACCESS_KEY
        aws_secret_access_key = YOUR_SECRET_KEY
        "
    5. You done bitch we outtie, use boto3 as u want
    4. See link below if confused
        https://boto3.readthedocs.io/en/latest/guide/quickstart.html#installation

Credentials for AWS:
general_credentials.csv contains the public and private api keys.
Also contains user and pass, will rewrite here:
User: hftboi
Password:summer2018
Link: https://558727307631.signin.aws.amazon.com/console
Public and private keys in csv

Other Notes:
    1.Importing modules for pyhcharm:
        https://stackoverflow.com/questions/26069254/importerror-no-module-named-bottle-pycharm
    2.Use system interpreter instead of virtual env, messes with the library dependencies