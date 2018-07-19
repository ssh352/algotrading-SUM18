import ws
import time
import multiprocessing as mp
from multiprocessing import Process
from ctypes import c_double
from typing import Tuple


def SocketLoopFactory() -> Tuple[mp.Process, mp.Value]:
    """
    Used to reset the Process and Value for our socket upon loss of connection. 
    Necessary because multiprocessing process's cannot be started more than once
    
    v (value) is a thread that tracks a double (variable from c) that represents time, 
    return: tuple of listening process and the variable we will store the time of last heartbeat in
    """
    v = mp.Value(c_double, time.time())
    return mp.Process(target=ws.main(), kwargs={"shared_beat": v}), v

def main():
    #p is actual ws process thread, beat is thread to store last heartbeat time
    p, beat = SocketLoopFactory()
    try:
        p.start()
        while p.is_alive():
            time.sleep(1)
            #if time between last heartbeat and current time is over 10, 
            #probable loss of heartbeat, must kill child and spawn new process
            if time.time() - beat.value > 10 and p.is_alive():
                p.terminate()
                p, beat = SocketLoopFactory()
                p.start()
    except Exception as e:
        print(e)
    p.terminate()


if __name__ == "__main__":
    main()