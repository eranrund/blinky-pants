from udp_client import N_LEDS, send_buf, send_buf2, sleep, hsv
from random import randint
import itertools
from threading import Thread, Lock

class Pixel(object):
    def __init__(self, r=0, g=0, b=0):
        self._r = self._g = self._b = 0

        self.r = r
        self.g = g
        self.b = b

    @property
    def g(self):
        return self._g
    @g.setter
    def g(self, val):
        if val < 0: val = 0
        if val > 255: val = 255
        self._g = val
        
    @property
    def r(self):
        return self._r
    @r.setter
    def r(self, val):
        if val < 0: val = 0
        if val > 255: val = 255
        self._r = val
        
    @property
    def b(self):
        return self._b
    @b.setter
    def b(self, val):
        if val < 0: val = 0
        if val > 255: val = 255
        self._b = val

    def to_list(self):
        return [self.r, self.g, self.b]

class LedStrip(object):
    def __init__(self, num_leds, fps=60):
        self.num_leds = num_leds
        self.fps = fps
        self.lock = Lock()

        self.pixels = [Pixel()] * num_leds
        self.thread = Thread(target=self.refresh_thread)
        self.thread.daemon = True
        self.thread.start()

    def refresh_thread(self):
        while True:
            buf = None
            with self.lock:
                buf = list(itertools.chain(*[pix.to_list() for pix in self.pixels]))

            send_buf(buf)
            sleep(1.0 / self.fps)

    def set_pixel_rgb(self, idx, r, g, b):
        self.pixels[idx] = Pixel(r,g,b)

    def set_pixel_hsv(self, idx, h, s, v):
        self.pixels[idx] = Pixel(*hsv(h,s,v))


ls = LedStrip(N_LEDS)

cnt = 0
while True:
    for i in range(N_LEDS):
        ls.set_pixel_hsv((i + cnt) % N_LEDS, i, 255, 255)
    cnt += 1
    sleep(0.01)


def random_walk(buf, i, c, max_val, change_amount, dirs):
    walk = randint(0, dirs-1)
    if walk == 0:
        if buf[i][c] >= change_amount:
            buf[i][c] -= change_amount
        else:
            buf[i][c] = 0
    elif walk == 1:
        if buf[i][c] <= (max_val - change_amount):
            buf[i][c] += change_amount
        else:
            buf[i][c] = max_val


def warm_white_shimmer(loops, max_brightness=120, change_amount=2, delay=0.005):
    buf = [[0,0,0]] * N_LEDS
    
    for cnt in range(loops):
        flag = cnt > (loops - 70)

        for i in range(0, N_LEDS, 2):
            random_walk(buf, i, 0, max_brightness, change_amount, 1 if flag else 2)

            buf[i][1] = buf[i][0]*4/5
            buf[i][2] = buf[i][0] >> 3

            if (i + 1) < N_LEDS:
                buf[i+1] = [
                    buf[i][0] >> 2,
                    buf[i][1] >> 2,
                    buf[i][2] >> 2,
                ]

        send_buf2(buf)
        sleep(delay)



#warm_white_shimmer(1000)


def color_explosion(max_loops=630, delay=0.01):
    buf = [[0,0,0]] * N_LEDS

    def fade(buf, l1, fade_time):
        l1_idx, l1_color = l1
        
        c = buf[l1_idx][l1_color]
        if c != 0:
            sub_amt = c >> fade_time
            if sub_amt < 1:
                sub_amt = 1
            buf[l1_idx][l1_color] -= sub_amt


    def bright_twinkle(buf, l1):
        l1_idx, l1_color = l1

        c = buf[l1_idx][l1_color]
        if c == 255:
            buf[l1_idx][l1_color] = 254
        elif c % 2:
            buf[l1_idx][l1_color] = (c * 2 + 1)  % 255
        elif c > 0:
            fade(buf, l1, 4)
            if buf[l1_idx][l1_color] % 2:
                buf[l1_idx][l1_color] -= 1

    def adj(buf, l1, prop_chance, left, right):
        l1_idx, l1_color = l1

        if (buf[l1_idx][l1_color] == 31) and randint(0, prop_chance) != 0:
            if left:
                left_idx, left_color = left
                if buf[left_idx][left_color] == 0:
                    buf[left_idx][left_color] = 1

            if right:
                right_idx, right_color = right
                if buf[right_idx][right_color] == 0:
                    buf[right_idx][right_color] = 1

        bright_twinkle(buf, l1)


    for loop_cnt in range(max_loops):
        no_new_bursts = (loop_cnt % 200) > 150 or (loop_cnt > (max_loops - 100))


        adj(buf, (0, 0), 9, None, (1, 0))
        adj(buf, (0, 1), 9, None, (1, 1))
        adj(buf, (0, 2), 9, None, (1, 2))

        for i in range(1, N_LEDS - 1):
            adj(buf, (i, 0), 9, (i-1, 0), (i+1, 0))
            adj(buf, (i, 1), 9, (i-1, 0), (i+1, 1))
            adj(buf, (i, 2), 9, (i-1, 0), (i+1, 2))

            
        adj(buf, (N_LEDS-1, 0), 9, (N_LEDS-2, 0), None)
        adj(buf, (N_LEDS-1, 1), 9, (N_LEDS-2, 1), None)
        adj(buf, (N_LEDS-1, 2), 9, (N_LEDS-2, 2), None)

        if not no_new_bursts:
            j = randint(0, N_LEDS-1)
            r = randint(0, 6)

            if r == 0 or r == 1:
                if buf[j][0] == 0:
                    buf[j][0] == 1

            elif r == 2 or r == 3:
                if buf[j][1] == 0:
                    buf[j][1] == 1

            elif r == 4 or r == 5:
                if buf[j] == [0,0,0]:
                    buf[j] = [1,1,1]

            elif r == 6:
                if buf[j][2] == 0:
                    buf[j][2] == 1

        send_buf2(buf)
        sleep(delay)
    

#color_explosion()

def genz():
    center_idx = N_LEDS / 2
    buf = [[0,0,0]] * N_LEDS
    hue = 200

    block = 50

    centers = [40, 80, 120, 160]
    sats = [255] * len(centers)
    hues = [randint(0, 255) for i in range(len(centers))]

    while True:
        sub_step = 0

        for i in range(len(centers)):
            hues[i] += randint(-1, 1)
            if hues[i] < 0: hues[i] == 0
            if hues[i] > 255: hues[i] = 255

        for i in range(30): 
            for segment, center_idx in enumerate(centers):
                buf[center_idx] = hsv(hues[segment], 255, 255)
            
                sat = sats[segment]

                #for i in range(20):
                if True:
                    i = sub_step
                    buf[center_idx - i] = hsv(hues[segment], 255, sat)
                    buf[center_idx + i] = hsv(hues[segment], 255, sat)
                    
                    sat -= 1 
                    if sat < 10:
                        sat = 255

                    sats[segment] = sat

                    #send_buf2(buf)
                    #sleep(0.002)
            sub_step += 1
            if sub_step == 20:
                sub_step = 0

            send_buf2(buf)
            sleep(0.002)


#genz()
