import numpy as np
import cv2
import matplotlib.pyplot as plt
from mss import mss
import os
import time
import socket

class Weapon:
    def __init__(self,name, kp, des, img):
        self.name = name.split('.')[0]
        self.kp = kp
        self.des = des
        self.img = img


meth ='cv2.TM_CCOEFF_NORMED'
weapons_dir = "weapons1080p";
threshold = 0.3
sleep = 1 # ms

connected = False
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.connect(("localhost", 5000))
    connected = True
except:
  print("An exception occurred") 

sift = cv2.SIFT_create()
bf = cv2.BFMatcher()

weapons = {}
for template in os.listdir():
    print(template)
    px_color = cv2.imread(os.path.join(weapons_dir, template), cv2.IMREAD_GRAYSCALE)
    thresh, bw = cv2.threshold(px_color, 250,255,cv2.THRESH_BINARY);
    kp, des = sift.detectAndCompute(bw,None)
    wc = Weapon(template, kp, des, bw)
    weapons[template] = wc


with mss() as sct:

    # 0 monitor is all monitors - 1 is primary
    monitor = sct.monitors[1] 
   
    while True:
        screenshot = cv2.cvtColor(np.array(sct.grab(monitor)), cv2.COLOR_RGB2GRAY)
        screenshot = np.hsplit(screenshot, 5)[4]
        screenshot = np.vsplit(screenshot, 3)[2]

        thresh, bw = cv2.threshold(screenshot, 250,255,cv2.THRESH_BINARY);

        kp, des = sift.detectAndCompute(bw,None)

        results = []
        # iterate all weapons and show matches
        for weapon in weapons:
            wc = weapons[weapon]
            # first comes the big picture, the soruce, then what you want to find in it
            matches = bf.knnMatch(des,wc.des,k=2)
            # Apply ratio test
            good = []
            for m,n in matches:
                if m.distance < threshold * n.distance:
                    good.append(m)


            results.append((wc, len(good)));
 

        res_sorted = sorted(results, key=lambda tup: tup[1])

        winner = res_sorted[-1]

        if winner[1] > 5:
           
            bytes = (winner[0].name + '\0').encode()
            if connected:
                s.sendall(len(bytes).to_bytes(2, byteorder='little'))
                s.sendall(bytes)
            print(winner[0].name + " " + str(winner[1]))
        else:
            bytes = ('nothing matched\0').encode()
            if connected:
                s.sendall(len(bytes).to_bytes(2, byteorder='little'))
                s.sendall(bytes)
            print("nothing matched")


        time.sleep(sleep)
        print()

s.close()