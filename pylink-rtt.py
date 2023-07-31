#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
How to use:
    1. if not install pylink: pip install pylink-square
    2. edit the define of this file to adapt your target
    2. python.exe ./pylink-rtt.py
"""

import pylink
from datetime import datetime
import os

TARGET_DEV = "STM32G473RC"
CONN_IF = pylink.enums.JLinkInterfaces.SWD

MAX_FILE_SIZE = 20 * 1024 * 1024    # max size of LOG file(20M)
BUFFER_SIZE_UP = 1024               # define at MCU in file "SEGGER_RTT_Conf.h"

def get_time(with_ms):
    """
    return current time
        with_ms=1   return format: "20230731 08:19:55.265"
        with_ms=0   return format: "20230731_081955"
    """
    time = ""
    if with_ms:
        time =  datetime.utcnow().strftime('%Y%m%d %H:%M:%S.%f')[:-3]   # "20230731 08:19:55.265"
    else:
        time =  datetime.utcnow().strftime('%Y%m%d_%H%M%S')             # "20230731_081955"
    return time

def RTT_read_string(link):
    try:
        if link.target_connected():
            readdata = link.rtt_read(0, BUFFER_SIZE_UP)
            if len(readdata) > 0:
                readdata = ''.join(map(chr, readdata))
        else:
            readdata = []
    except pylink.errors.JLinkException:
        readdata = []
        pass
    return readdata


jlink = pylink.JLink()
jlink.open()
jlink.set_tif(CONN_IF)
jlink.connect(TARGET_DEV)
jlink.rtt_start()

file_name = ["", "", "", "", "", ""]    # if "IndexError: list index out of range", need add the length of this list.
file_name[0] = get_time(0) + ".txt"
fp = open(file_name[0], "a", newline='\n')

while True:
    try:
        rttOut = RTT_read_string(jlink)
        rtt_len = len(rttOut)
        if rtt_len > 0:
            idx = 0
            while idx < rtt_len:
                try:
                    if idx == 0 or rttOut[idx-1] == '\n':
                        fp.write("[" + get_time(1) + "] ")
                    fp.write(rttOut[idx])
                    idx += 1
                except UnicodeEncodeError:        # 'ÿ' exception
                    fp.close()

                    FN_idx = int(rttOut[idx+1])
                    if file_name[FN_idx] == "":
                        file_name[FN_idx] = get_time(0) + "_T" + rttOut[idx+1] + ".txt"
                    fp = open(file_name[FN_idx], "a", newline='\n')

                    file_stat = os.stat(file_name[FN_idx])
                    if file_stat.st_size > MAX_FILE_SIZE:
                        fp.close()
                        file_name[FN_idx] = get_time(0) + "_T" + rttOut[idx+1] + ".txt" 
                        fp = open(file_name[FN_idx], "a", newline='\n')
                    idx += 2
                except:
                    print("[", get_time(1), "]", "except")

            file_stat = os.stat(file_name[0])
            if file_stat.st_size > MAX_FILE_SIZE:
                fp.close()
                file_name[0] = get_time(0) + ".txt"
                fp = open(file_name[0], "a", newline='\n')
    except KeyboardInterrupt:
        print("KeyInterrupt")
        fp.close()
        jlink.rtt_stop()
        jlink.close()
        break
