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

#UDP_IP = "192.168.0.103"
UDP_IP = '127.0.0.1'
UDP_PORT = 9022
N_LEDS = 228

RED = "\xFF\x00\x00" * N_LEDS
GREEN = "\x00\xFF\x00" * N_LEDS

from time import sleep

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) 
cnt = 0

def send_buf(buf):
    assert len(buf) == N_LEDS * 3
    b = ''.join(chr(ch) for ch in buf)
    sock.sendto(b, (UDP_IP, UDP_PORT))

def send_buf2(buf):
    return send_buf(list(itertools.chain(*buf)))



import atexit
def clear():
    buf = [0] * 3 * N_LEDS
    send_buf(buf)
atexit.register(clear)


 
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


def matrix(hue=95, sat=255):
    buf = [[0,0,0]] * N_LEDS
    buf_copy = list(buf)

    while True:
        rand = random.randint(0, 100)
        if rand > 90:
            buf[0] = hsv(hue, sat, 255)
        else:
            buf[0] = hsv(hue, sat, 0)

        buf_copy = list(buf)
        for i in range(1, N_LEDS):
            buf[i] = buf_copy[i-1]

        send_buf2(buf)
        sleep(0.020)





if __name__=='__main__':
    #sym_simple_hsv()
    #flicker()
    matrix()

#    sock.sendto(RED, (UDP_IP, UDP_PORT))
#    sleep(1.0 / 30)
#    sock.sendto(GREEN, (UDP_IP, UDP_PORT))
#    sleep(1.0 / 30)


