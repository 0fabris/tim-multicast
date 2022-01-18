import time
import os
import sys

filters = ["m4v", "m4a"]

flimit = 50

path = "./" if len(sys.argv) < 2 else sys.argv[1]

sTime = 5000

while True:

    d = sorted(set(os.listdir(f"{path}")))
    
    fls = dict()
    for f in filters:
        fls[f] = []

    for fi in d:
        ext = fi.split(".")[-1]
        if(ext in fls.keys()):
            fls[ext].append(fi)
    
    
    for ks in fls.keys():
        for fname in list(reversed(fls[ks]))[:-flimit]:
            os.remove(os.path.join(path,fname))
            print("removed "+fname)

    time.sleep(sTime)

