import audioop
import numpy
import struct
import pyaudio
import wave
import sys

CHUNK = 1024

if len(sys.argv) < 2:
    print("Plays a wave file.\n\nUsage: %s filename.wav" % sys.argv[0])
    sys.exit(-1)

wf = wave.open(sys.argv[1], 'rb')

p = pyaudio.PyAudio()

stream = p.open(format=p.get_format_from_width(wf.getsampwidth()),
                channels=wf.getnchannels(),
                rate=wf.getframerate(),
                output=True)

data = wf.readframes(CHUNK)


chunk      = 2**11 # Change if too fast/slow, never less than 2**11
scale      = 50 #4    # Change if too dim/bright
exponent   = 5     # Change if too little/too much difference between loud and quiet sounds
samplerate = 44100 

import udp_client

def calculate_levels(data, n_levels):
 
    # Convert raw sound data to Numpy array
    fmt = "%dH"%(len(data)/2)
    data2 = struct.unpack(fmt, data)
    data2 = numpy.array(data2, dtype='h')
 
    # Apply FFT
    fourier = numpy.fft.fft(data2)
    ffty = numpy.abs(fourier[0:len(fourier)/2])/1000
    ffty1=ffty[:len(ffty)/2]
    ffty2=ffty[len(ffty)/2::]+2
    ffty2=ffty2[::-1]
    ffty=ffty1+ffty2
    ffty=numpy.log(ffty)-2
    
    fourier = list(ffty)[4:-4]
    fourier = fourier[:len(fourier)/2]
    
    size = len(fourier)
 
    # Add up for 6 lights
    #levels = [sum(fourier[i:(i+size/6)]) for i in xrange(0, size, size/6)][:6]
    levels = [sum(fourier[i:(i+size/n_levels)]) for i in xrange(0, size, size/n_levels)][:n_levels]
    
    return levels

from udp_client import send_buf2,hsv,N_LEDS
n_levels =28 
old_levels = [0] * n_levels
while data != '':
    #out = []
    #for i in range(0, len(data), 4):
    #    out.append(data[i:i+2])

    #rms   = audioop.rms(data, 2) 
    #level = min(rms / (2.0 ** 16) * scale, 1.0) 
    #level = level**exponent 
    #level = int(level * 255)
    #print level    

    #levels = calculate_levels(''.join(out), n_levels)     

    mono_data = audioop.tomono(data, 2, 0.5, 0.5)
    levels = calculate_levels(mono_data, n_levels)

    scale = 20
    for level in levels:
        level = max(min(level / scale, 1.0), 0.0)
        level = level**exponent 
        level = int(level * 255)
        print level,
    print


    num_segs = len(levels)
    leds_per_seg = udp_client.N_LEDS / num_segs

    levelz = []
    for level in levels:
        level = max(min(level / scale, 1.0), 0.0)
        level = level**exponent 
        level = int(level * 255)
        levelz.append(level)
    levels = levelz

    # compare to old
    for i, new_level in enumerate(levels):
        if new_level > old_levels[i]:
            old_levels[i] = new_level
        else:
            level = (old_levels[i] * 7 + new_level) / 8
            old_levels[i] = level
            levels[i] = level


    center_idx = udp_client.N_LEDS / 2
    hue_min = 0
    hue_max = 60
    buf = [[0,0,0]] * udp_client.N_LEDS
    leds_per_seg = (udp_client.N_LEDS / n_levels) 
    idx_left = center_idx
    idx_right = center_idx
    for level_idx, level in enumerate(levels):
        h = level_idx * leds_per_seg / 2
        for i in range(leds_per_seg / 2):            
            c = hsv(h, 255, level)
            buf[idx_left] = c
            buf[idx_right] = c

            idx_left -= 1
            idx_right += 1

#        idx_from = center_idx - (((level_idx + 1) * leds_per_seg) / 2)
#        idx_to = 


        #buf[i + center_idx] = hsv(60, 255, level)


    send_buf2(buf)

    
    # buf = []
    # led_cnt = 0
    # for level in levels:
    #    level = max(min(level / scale, 1.0), 0.0)
    #    level = level**exponent 
    #    level = int(level * 255)
    #    for led in range(leds_per_seg):
    #         buf.append(udp_client.hsv(led_cnt, 255, level))
    #         led_cnt += 1

    # while len(buf) < udp_client.N_LEDS:
    #     buf.append([0,0,0])
    # udp_client.send_buf2(reversed(buf))

    stream.write(data)
    data = wf.readframes(CHUNK)

stream.stop_stream()
stream.close()

p.terminate()
