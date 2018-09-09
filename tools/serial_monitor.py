#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import serial
import time
import argparse


def main():
    parse = argparse.ArgumentParser(description="serial log tool")
    parse.add_argument('-p', '--port', action='store', type=str,
                       dest='port', help="serial port")
    parse.add_argument('-b', '--baudrate', action='store', dest='baudrate',
                       type=int, default=115200, help="serial baudrate")

    results = parse.parse_args()
    if results.port is None:
        print("please input the serial port")
        return

    s = serial.Serial(results.port, baudrate=results.baudrate, timeout=500)
    while True:
        data = s.read(1)
        if len(data) == 0:
            continue
        try:
            print(str(data, 'utf8'), end='')
        except UnicodeDecodeError:
            print(" %02X" % data[0], end='')


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("over")
