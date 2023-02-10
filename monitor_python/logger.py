import logging
from logging.handlers import RotatingFileHandler
import os


class Logger:
    def __init__(self, log_name, level=logging.DEBUG):
        self.__logger = logging.getLogger(log_name)
        self.__logger.setLevel(level)
        log_format = logging.Formatter(
            '[%(asctime)s] [%(name)s] [%(levelname)s] [%(filename)s:%(lineno)d]: %(message)s')

        # console logger
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(log_format)

        # log_path = os.path.dirname(os.getcwd()) + "/logs"
        # if not os.path.isdir(log_path):
        #     print("log dir not exist, so create it.")
        #     # 递归创建目录
        #     os.makedirs(log_path)
        # log_name = log_path + "/log.log"
        if not os.path.exists("logs"):
            self.__logger.warning("log folder not exist")
            try:
                os.makedirs("logs")
            except FileExistsError as err:
                self.__logger.error(f"{err}")
        log_name = "logs/log.log"

        # rotate logger
        file_handler = RotatingFileHandler(filename=log_name, mode='a', maxBytes=1024*1024*5,
                                           backupCount=3, encoding="UTF-8", delay=False)

        file_handler.setFormatter(log_format)
        self.__logger.addHandler(console_handler)
        self.__logger.addHandler(file_handler)

    # 使用此封装会导致格式化输出 文件名及行号将一直是logger.py及固定值
    # def debug(self, message):
    #     self.__logger.debug(message)
    #
    # def info(self, message):
    #     self.__logger.info(message)
    #
    # def warn(self, message):
    #     self.__logger.warning(message)
    #
    # def error(self, message):
    #     self.__logger.error(message)
    #
    # def critical(self, message):
    #     self.__logger.critical(message)

    def get_logger(self):
        return self.__logger


if __name__ == '__main__':
    pass
