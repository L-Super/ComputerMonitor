
import zmq
from logger import Logger
from monitor import Monitor

if __name__ == '__main__':
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind("tcp://*:20777")

    logger = Logger("main").get_logger()

    logger.info(f"launch server. version is {zmq.zmq_version()}")

    monitor = Monitor()
    while True:
        #  Wait for next request from client
        message = socket.recv()
        logger.info(f"Received request: {message}")

        if message == b"DisksOverview":
            # 返回的re是str类型，故用send_json()发送，会导致客户端解析json失败崩溃
            re = monitor.json_disk_overview()
            socket.send_string(re)
            # socket.send_json(re)
            logger.info(f"DisksOverview: {re}")
        elif message == b"DisksInfo":
            re = monitor.json_disks_info()
            socket.send_string(re)
            logger.info(f"DisksInfo: {re}")
        elif message == b"DiskIO":
            re = monitor.json_all_disks_io()
            socket.send_string(re)
            logger.info(f"DiskIO: {re}")
        else:
            socket.send(b"invalid request")
            logger.warning(f"invalid request")

    socket.close()
    context.term()
    logger.error("quit while func. Application dead.")
