from flask import Flask, request, session, g, redirect, url_for, abort, render_template, flash, jsonify
import socket

# configuration
DEBUG = True
SECRET_KEY = 'fds95498djsxfzdxn890c437n8p;fdx8rhw35nv7wp5sdsdfas4c32nj74-2p['
HTTPD_PORT = 8048
LPD_SERVER = ('192.168.0.26', 9022)
NUM_LEDS = 228

# init
app = Flask(__name__)
app.config.from_object(__name__)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# views
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/color/<c>')
def set_color(c):
    r, g, b = int(c[0:2], 16), int(c[2:4], 16), int(c[4:6], 16)
    buf = [r, g, b] * NUM_LEDS
    sock.sendto(''.join(chr(ch) for ch in buf), LPD_SERVER)
    return jsonify({})

@app.route('/pattern/<p>')
def set_pattern(p):
    sock.sendto('PAT'+chr(int(p)), LPD_SERVER)
    return jsonify({})

# run
if __name__ == '__main__':
    app.run('0.0.0.0', port=HTTPD_PORT)
