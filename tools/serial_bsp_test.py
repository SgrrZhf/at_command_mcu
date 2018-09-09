#!/usr/bin/env python3
#-*-coding:utf-8-*-
import serial
import time
from random import Random

DEBUG = True
SIZE = 256


def random_str(randomlength=8):
    str = ''
    chars = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789'
    length = len(chars) - 1
    random = Random()
    for i in range(randomlength):
        str += chars[random.randint(0, length)]
    return str


def read_frame(ser):
    """ get frame data from ser """
    frame = bytearray()
    received_flag = False
    while True:
        data = ser.read(1)
        if len(data) == 0 and received_flag:
            return frame
        elif len(data) > 0:
            received_flag = True
        frame += data


def cmp_write_read(src, dst):
    """ 忽略dst最后一个字符（换行符） """
    if len(src) > SIZE:
        src = src[len(src) - SIZE:]
    if DEBUG:
        print(src, ':', dst)

    if src == dst[:-1]:
        return True
    else:
        return False


def write_read_cmp(ser, ser_log, data):
    frame = read_frame(ser)
    ser_log.readall()
    data1 = bytes(data, 'utf8')
    ser.write(data1)
    data2 = ser_log.read(1024)
    if cmp_write_read(data1, data2):
        if DEBUG:
            print("\033[0;32m%s\033[0m" % 'ok')
        return True
    else:
        if DEBUG:
            print("\033[0;31m%s\033[0m" % 'error')
        return False


def write_full_buffer(ser, ser_log):
    if write_read_cmp(ser, ser_log, random_str(64)):
        print("\033[0;32m%s\033[0m" % 'write_full_buffer 1 ok')
    if write_read_cmp(ser, ser_log, random_str(SIZE - 64)):
        print("\033[0;32m%s\033[0m" % 'write_full_buffer 2 ok')
    if write_read_cmp(ser, ser_log, random_str(100)):
        print("\033[0;32m%s\033[0m" % 'write_full_buffer 3 ok')
    if write_read_cmp(ser, ser_log, random_str(SIZE - 100)):
        print("\033[0;32m%s\033[0m" % 'write_full_buffer 4 ok')


def write_over_size(ser, ser_log):
    if write_read_cmp(ser, ser_log, random_str(63)):
        print("\033[0;32m%s\033[0m" % 'write_over_size 1 ok')
    if write_read_cmp(ser, ser_log, random_str(100)):
        print("\033[0;32m%s\033[0m" % 'write_over_size 2 ok')
    if write_read_cmp(ser, ser_log, random_str(55)):
        print("\033[0;32m%s\033[0m" % 'write_over_size 3 ok')
    if write_read_cmp(ser, ser_log, random_str(200)):
        print("\033[0;32m%s\033[0m" % 'write_over_size 4 ok')


def write_overflow_size(ser, ser_log):
    if write_read_cmp(ser, ser_log, random_str(256)):
        print("\033[0;32m%s\033[0m" % 'write_overflow_size 1 ok')
    else:
        print("\033[0;31m%s\033[0m" % 'write_overflow_size 1 error')
    if write_read_cmp(ser, ser_log, random_str(700)):
        print("\033[0;32m%s\033[0m" % 'write_overflow_size 2 ok')
    else:
        print("\033[0;31m%s\033[0m" % 'write_overflow_size 2 error')
    if write_read_cmp(ser, ser_log, random_str(500)):
        print("\033[0;32m%s\033[0m" % 'write_overflow_size 3 ok')
    else:
        print("\033[0;31m%s\033[0m" % 'write_overflow_size 3 error')
    if write_read_cmp(ser, ser_log, random_str(300)):
        print("\033[0;32m%s\033[0m" % 'write_overflow_size 4 ok')
    else:
        print("\033[0;31m%s\033[0m" % 'write_overflow_size 4 error')


def main():
    ser = serial.Serial('/dev/ttyUSB1', baudrate=9600, timeout=0.02)
    ser_log = serial.Serial('/dev/ttyUSB0', baudrate=115200 * 8, timeout=1)

    write_full_buffer(ser, ser_log)
    write_over_size(ser, ser_log)
    write_overflow_size(ser, ser_log)

    return


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        quit()
