import re
lines = file('sketch.ino', 'r').readlines()

pattern = r'\(rgb_color\)\{(.+?)\}';
for l in lines:
    l = l.rstrip()
    print re.sub(pattern, r'CRGB(\1)', l)
