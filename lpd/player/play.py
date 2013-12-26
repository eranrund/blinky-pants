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

def calculate_levels(data):
 
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
    levels = [sum(fourier[i:(i+size/10)]) for i in xrange(0, size, size/10)][:10]
    
    return levels

while data != '':
    out = []
    for i in range(0, len(data), 4):
        out.append(data[i:i+2])

    #rms   = audioop.rms(data, 2) 
    #level = min(rms / (2.0 ** 16) * scale, 1.0) 
    #level = level**exponent 
    #level = int(level * 255)
    #print level    

    levels = calculate_levels(''.join(out))     
    for level in levels:
        level = max(min(level / scale, 1.0), 0.0)
        level = level**exponent 
        level = int(level * 255)
        print level,
    print


    num_segs = len(levels)
    leds_per_seg = udp_client.N_LEDS / num_segs
    
    buf = []
    led_cnt = 0
    for level in levels:
       level = max(min(level / scale, 1.0), 0.0)
       level = level**exponent 
       level = int(level * 255)
       for led in range(leds_per_seg):
            buf.append(udp_client.hsv(led_cnt, 255, level))
            led_cnt += 1

    while len(buf) < udp_client.N_LEDS:
        buf.append([0,0,0])
    udp_client.send_buf2(reversed(buf))

    stream.write(data)
    data = wf.readframes(CHUNK)

stream.stop_stream()
stream.close()

p.terminate()
