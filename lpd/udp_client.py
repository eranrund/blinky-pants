import socket
import colorsys

def hsv(h, s, v):
    x = colorsys.hsv_to_rgb(
        float(h) / 255,
        float(s) / 255,
        float(v) / 255
    )

    return [int(x * 255) for x in x]

print hsv(50, 255, 255)

UDP_IP = "192.168.0.103"
UDP_PORT = 9022
N_LEDS = 226

RED = "\xFF\x00\x00" * N_LEDS
GREEN = "\x00\xFF\x00" * N_LEDS

from time import sleep

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) 
cnt = 0

def send_buf(buf):
    assert len(buf) == N_LEDS * 3
    b = ''.join(chr(ch) for ch in buf)
    sock.sendto(b, (UDP_IP, UDP_PORT))
 
def rainbow():
    while True:
        buf = []
        for led in range(255):
            buf += hsv((led + cnt) % 255, 255, 255)

        b = ''.join(chr(ch) for ch in buf)
        sock.sendto(b, (UDP_IP, UDP_PORT))
        cnt += 1

        sleep(0.005)

import random, itertools
def flicker():
    while True:
        bright = random.randint(0, 255)
        delay = random.randint(10, 100)
        rrr = random.randint(0, bright)
        if rrr < 10:
            buf = []
            for i in range(N_LEDS):
                buf += hsv(160, 50, bright)
            send_buf(buf)
            sleep(float(delay)/1000)

    pass

def sym_simple_hsv():
    cnt = 0
    while True:
        buf = []
        for i in range(N_LEDS / 2):
            buf.append(hsv( (i+cnt) % 255, 255, 255))
        buf += reversed(buf)
        buf = list(itertools.chain(*buf))
        send_buf(buf)
        sleep(0.005)
        cnt += 1


import atexit
def clear():
    buf = [0] * 3 * N_LEDS
    send_buf(buf)
atexit.register(clear)

sym_simple_hsv()
#flicker()

#    sock.sendto(RED, (UDP_IP, UDP_PORT))
#    sleep(1.0 / 30)
#    sock.sendto(GREEN, (UDP_IP, UDP_PORT))
#    sleep(1.0 / 30)


