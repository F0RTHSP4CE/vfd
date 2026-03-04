#!/usr/bin/env python3

import itertools
from time import time, sleep
from cv2 import VideoCapture, resize, CAP_PROP_POS_MSEC
from os import path
import numpy as np
import serial

port = serial.Serial("/dev/serial/by-id/usb-1a86_USB2.0-Serial-if00-port0", 115200)
sleep(3)

cap = VideoCapture(path.join(path.dirname(__file__), "bad_apple.mp4"))
start = time()

while cap.isOpened():
    # read frame
    ok, frame = cap.read()
    if not ok:
        break

    # drop frames (display is slower)
    cur_time = time()
    pos = cap.get(CAP_PROP_POS_MSEC)
    if pos < (cur_time - start) * 1000:
        continue
    delay = (pos / 1000) - (cur_time - start)
    if delay > 0:
        sleep(delay)

    # resize to fit display
    assert frame.shape == (360, 480, 3)
    frame = resize(frame, (138, 103))
    frame = frame[45:61, 0:138]

    # draw frame
    framebuffer = [0] * (1400 // 8)
    for dot in range(1400):
        column = dot // (5 * 14)
        col_dot = dot % (5 * 14)
        col_x = col_dot % 5
        col_y = col_dot // 5
        x = (column * 7) + col_x
        y = col_y
        if y >= 7:
            y += 1
        if frame[y][x][0] >= 128:
            framebuffer[dot // 8] |= 1 << (dot % 8)
    port.write(bytes(framebuffer))

    print(f"time={int(pos)}")
