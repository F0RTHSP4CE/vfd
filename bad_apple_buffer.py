#!/usr/bin/env python3

import itertools
from time import sleep
from cv2 import VideoCapture, resize, CAP_PROP_POS_MSEC
from os import path
import numpy as np

cap = VideoCapture(path.join(path.dirname(__file__), "bad_apple.mp4"))

entire_buffer = bytearray()
simulated_time = 0
fps = 30

while cap.isOpened():
    # read frame
    ok, frame = cap.read()
    if not ok:
        break

    # drop frames (display is slower)
    pos = cap.get(CAP_PROP_POS_MSEC)
    if pos < simulated_time * 1000:
        continue

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
        
    entire_buffer.extend(framebuffer)
    simulated_time += 1 / fps

print(f"{len(entire_buffer) // 1024} KiB")

# with open("esp/bad_apple.h", "w") as f:
#     f.write(f"#define VIDEO_FRAME_SIZE {1400 // 8}\r\n")
#     f.write(f"#define VIDEO_PERIOD {int(1_000_000 / fps)}\r\n\r\n")
#     f.write("static const uint8_t video_data[] PROGMEM = {\r\n")
#     for i in range(len(entire_buffer) // 25):
#         piece = entire_buffer[i * 25 : (i + 1) * 25]
#         f.write("  ")
#         for b in piece:
#             f.write(f"0x{b:02x}, ")
#         f.write("\r\n")
#     f.write("};\r\n")

with open("esp/data/bad_apple.bin", "wb") as f:
    f.write(entire_buffer)
