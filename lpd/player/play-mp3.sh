ffmpeg -i "$1" -f s16le - | python play-mp3.py
