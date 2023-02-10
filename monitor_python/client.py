#
#   Hello World client in Python
#   Connects REQ socket to tcp://localhost:5555
#   Sends "Hello" to server, expects "World" back
#
import json
import os
import time
import zmq
import cProfile


def block_socket():
    context = zmq.Context()

    #  Socket to talk to server
    print("Connecting to hello world server…")
    socket = context.socket(zmq.REQ)
    # socket.connect("tcp://localhost:20777")
    socket.connect("tcp://10.2.15.10:20777")

    #  Do 10 requests, waiting each time for a response
    for request in range(2):
        print(f"Sending request {request} …")
        socket.send_string("DisksInfo")
        # socket.send(b"DisksOverview")

        #  Get the reply.
        message = socket.recv()

        if message == "":
            print("timeout")
        else:
            data = json.loads(message)
            print(f"Received reply {request}  {data}")

        time.sleep(2)


def no_block_socket():
    context = zmq.Context()

    # Socket to talk to server
    print("Connecting to hello world server…")
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://localhost:20777")

    poller = zmq.Poller()
    poller.register(socket, zmq.POLLIN)
    # 2s 超时
    TIMEOUT = 2000

    # Do 10 requests, waiting each time for a response
    for i in range(10):
        socket.send(b"Hello")
        # 1.
        # socks = dict(poller.poll(TIMEOUT))
        # # 如果socket在socks里 且 socks[socket]的事件等于zmq.POLLIN
        # if socket in socks and socks[socket] == zmq.POLLIN:
        #     reply = socket.recv()
        #     print("Received reply ", i, "[", reply, "]")

        # 2.
        if poller.poll(TIMEOUT):
            reply = socket.recv()
            # print("Received reply ", i, "[", reply, "]")
        else:
            print("maybe timeout")
            break
    print("quit range")
    poller.unregister(socket)
    # socket.close()
    # context.term()
    # context.destroy()
    print("quit range")
    return "fa"


if __name__ == '__main__':
    # cProfile.run('no_block_socket()')

    # print(no_block_socket())
    # pass
    block_socket()

