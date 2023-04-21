import os
import math
import time
import serial
import logging
from ymodem.Modem import Modem


class TaskProgressBar:
    def __init__(self):
        self.bar_width = 50
        self.last_task_name = ""
        self.current_task_start_time = -1

    def show(self, task_index, task_name, total, success, failed):
        if task_name != self.last_task_name:
            self.current_task_start_time = time.perf_counter()
            if self.last_task_name != "":
                print('\n', end="")
            self.last_task_name = task_name

        success_width = math.ceil(success * self.bar_width / total)

        a = "#" * success_width
        b = "." * (self.bar_width - success_width)
        progress = (success_width / self.bar_width) * 100
        cost = time.perf_counter() - self.current_task_start_time

        print(
            f"\r{task_index} - {task_name} {progress:.2f}% [{a}->{b}]{cost:.2f}s", end="")


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG, format='%(message)s')
    logging.info('Ymodem started!')

    serial_io = serial.Serial()
    serial_io.port = "/dev/cu.usbserial-22310"
    serial_io.baudrate = "115200"
    serial_io.parity = "N"
    serial_io.bytesize = 8
    serial_io.stopbits = 1
    serial_io.timeout = 2

    try:
        serial_io.open()
    except Exception as e:
        raise Exception("Failed to open serial port!")

    def receiver_read(size, timeout=3):
        serial_io.timeout = timeout
        data = serial_io.read(size) or None
        if data:
            print(data.hex(), end="")
        return data

    def receiver_write(data, timeout=3):
        serial_io.writeTimeout = timeout
        return serial_io.write(data)

    logging.info('Receiving...!')
    receiver = Modem(receiver_read, receiver_write)
    # os.chdir(sys.path[0])
    folder_path = os.path.abspath("data.csv")
    progress_bar = TaskProgressBar()
    received = receiver.recv(folder_path, callback=progress_bar.show)

    serial_io.close()
    logging.info('Done!')
