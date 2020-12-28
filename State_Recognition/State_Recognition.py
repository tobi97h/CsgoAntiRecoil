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


def screen_lower_right(monitor):
    screenshot = cv2.cvtColor(np.array(sct.grab(monitor)), cv2.COLOR_RGB2GRAY)

    # halbieren, dritteln, vertikal und horizontal xD
    screenshot = np.hsplit(screenshot, 5)[4]
    screenshot = np.vsplit(screenshot, 3)[2]

    thresh, bw = cv2.threshold(screenshot, 250,255,cv2.THRESH_BINARY)
    return bw

class Recognizer:

    def __init__(self):
        self.threshold = 0.3

        # connect to recoil_control
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect(("localhost", 5000))

        # algos
        self.sift = cv2.SIFT_create()
        self.bf = cv2.BFMatcher()

        self.set_weapons()

       
    def set_weapons(self):
        weapons_dir = "weapons1080p"

        self.weapons = {}
        for template in os.listdir(weapons_dir):
            print(template)
            px_color = cv2.imread(os.path.join(weapons_dir, template), cv2.IMREAD_GRAYSCALE)
            thresh, bw = cv2.threshold(px_color, 250,255,cv2.THRESH_BINARY);
            kp, des = self.sift.detectAndCompute(bw,None)
            wc = Weapon(template, kp, des, bw)
            self.weapons[template] = wc

    def sort_send_res(self, results):
        res_sorted = sorted(results, key=lambda tup: tup[1])

        winner = res_sorted[-1]

        if winner[1] > 5:
            bytes = (winner[0].name + '\0').encode()
            self.s.sendall(len(bytes).to_bytes(2, byteorder='little'))
            self.s.sendall(bytes)
        else:
            bytes = ('nothing matched\0').encode()
            self.s.sendall(len(bytes).to_bytes(2, byteorder='little'))
            self.s.sendall(bytes)

    def loop(self, monitor):
        while True:
            bw = screen_lower_right(monitor)

            kp, des = self.sift.detectAndCompute(bw,None)

            results = []
            # iterate all weapons and show matches
            for weapon in self.weapons:
                wc = self.weapons[weapon]
                # first comes the big picture, the soruce, then what you want to find in it
                matches = self.bf.knnMatch(des,wc.des,k=2)
                # Apply ratio test
                good = []
                for m,n in matches:
                    if m.distance < self.threshold * n.distance:
                        good.append(m)


                results.append((wc, len(good)));
 
            self.sort_send_res(results)

            time.sleep(1)




reco = Recognizer()

with mss() as sct:
    # 0 monitor is all monitors - 1 is primary
    monitor = sct.monitors[1] 
    reco.loop(monitor)

