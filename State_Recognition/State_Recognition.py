import numpy as np
import cv2
import matplotlib.pyplot as plt
from mss import mss
import os
import time
import socket
import win32api, win32con

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
    screenshot = np.vsplit(screenshot, 2)[1]

    thresh, bw = cv2.threshold(screenshot, 250,255,cv2.THRESH_BINARY)

    return bw

class Recognizer:

    def __init__(self):
        # feature match ditstance count 1-0
        self.threshold = 0.4

        # min feature matches to count as weapon found
        self.min_matched = 10

        # last matched weapon to compare
        self.last_weapon = None

        # connect to recoil_control
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while True:
            try:
                self.s.connect(("localhost", 5000))
                print("connected")
                break;
            except:
                print("couldn't connect, trying again")

        # algos
        self.sift = cv2.SIFT_create()
        self.bf = cv2.BFMatcher()

        self.set_weapons()

       
    def set_weapons(self):
        weapons_dir = "weapons1080p"

        print("loaded weapons")
        self.weapons = {}
        for template in os.listdir(weapons_dir):
            print(template)
            px_color = cv2.imread(os.path.join(weapons_dir, template), cv2.IMREAD_GRAYSCALE)
            thresh, bw = cv2.threshold(px_color, 250,255,cv2.THRESH_BINARY);
            kp, des = self.sift.detectAndCompute(bw,None)
            wc = Weapon(template, kp, des, bw)
            self.weapons[template] = wc
        print()


    def sort_send_res(self, results):
        res_sorted = sorted(results, key=lambda tup: tup[1])

        winner = res_sorted[-1]

        if winner[1] > self.min_matched:
            # dont resent matched weapon
            if self.last_weapon is not winner[0]:
                # we have a winner with more than min features
                bytes = (winner[0].name + '\0').encode()
                self.s.sendall(len(bytes).to_bytes(2, byteorder='little'))
                self.s.sendall(bytes)
                self.last_weapon = winner[0]

        else:
            # only send nothing matched if not flashing weapon
            bytes = ('nothing matched\0').encode()
            self.s.sendall(len(bytes).to_bytes(2, byteorder='little'))
            self.s.sendall(bytes)
            self.last_weapon = None



    def determine(self, monitor):
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
              
        

reco = Recognizer()

keys = [win32con.VK_ESCAPE, win32con.VK_RETURN, win32con.VK_LBUTTON,
        # Q,E,B,1,2,3,4,5,6
        0x51, 0x45, 0x42, 0x47, 0x31, 0x32, 0x33, 0x34, 0x35]

with mss() as sct:
    # 0 monitor is all monitors - 1 is primary
    monitor = sct.monitors[1]
    while True:  

        # if any of the specified keys are pressed
        for key in keys:
            if win32api.GetAsyncKeyState(key) & 0x8000:
                # if any relevant key was hit, call determine a bunch of times
                for i in range(0,5):
                    reco.determine(monitor)
                    time.sleep(0.01)
                break

        # reset the timestamp for the matching flashing weapons
        time.sleep(0.001)


