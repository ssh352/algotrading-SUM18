import shutil
import os
import multiprocessing as mp
import datetime
import signal
import sys
import time


# TODO TEST THIS!

# THIS ASSUMES THAT websocket_stuff/full EXISTS! MAKE SURE IT DOES OR STUFF WILL GO WEIRD!


def init_worker():
    signal.signal(signal.SIGTERM, sig_handler)


def sig_handler(sig):
    sys.exit(0)


def roller(currency_dir):
    """
    periodically polls the directories inside a currency's directory and compresses them if safe to, ie if the directory
    is from yesterday or farther back in the past and the socket is done writing to them. When roller compresses a
    directory, it also removes the uncompressed directory
    :param currency_dir: directory corresponding to a currency pair, eg ETH-EUR
    :return:
    """
    os.chdir(currency_dir)
    while True:
        date_dirs = [obj for obj in os.listdir() if os.path.isdir(obj)]  # creates a list of directory names
        now_day = datetime.datetime.utcnow().day
        for folder in date_dirs:
            if now_day - int(folder[-2:]) >= 1:  # if the folder is from yesterday or farther back
                shutil.make_archive(folder, "xztar", folder)  # makes a compressed tarball as folder.tar.xz
                shutil.rmtree(folder)  # remove the directory after we create a compressed archive of it
        time.sleep(3600)  # wait an hour before polling again, compression isn't urgent


def main():
    signal.signal(signal.SIGTERM, sig_handler)

    WEBSOCKET_STUFF_PATH = os.getcwd()
    os.chdir('full')
    pairs = [file for file in os.listdir()]
    # TODO the signal handler setting is actually a race condition... how to fix?
    with mp.Pool(processes=len(pairs), initializer=init_worker) as pool:
        pool.map(func=roller, iterable=pairs, chunksize=1)

if __name__ == "main":
    main()