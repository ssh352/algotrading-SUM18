import ws
import time
import multiprocessing as mp
from multiprocessing import Process
from ctypes import c_double
from typing import Tuple


def SocketLoopFactory() -> Tuple[mp.Process, mp.Value]:
    """
    Used to reset the Process and Value for our socket upon loss of connection. Necessary because multiprocessing
    Process 's cannot be started more than once
    :return: tuple of listening process and the variable we will store the time of least heartbeat in
    """
    v = mp.Value(c_double, time.time())
    return mp.Process(target=ws.main(), kwargs={"shared_beat": v}), v

def main():
    p, beat = SocketLoopFactory()
    try:
        p.start()
        while p.is_alive():
            time.sleep(1)
            # this almost certainly signals loss of heartbeat, thus we must kill child and spawn new process
            if time.time() - beat.value > 10 and p.is_alive():
                p.terminate()
                p, beat = SocketLoopFactory()
                p.start()
    except Exception as e:
        print(e)
    p.terminate()


if __name__ == "__main__":
    main()