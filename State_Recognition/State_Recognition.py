import numpy as np
import cv2
import matplotlib.pyplot as plt
from mss import mss
import os
import time

class Weapon:
    def __init__(self,name, kp, des, img):
        self.name = name
        self.kp = kp
        self.des = des
        self.img = img


meth ='cv2.TM_CCOEFF_NORMED'
weapons_dir = "weapons1080p";
threshold = 0.3
sleep = 0.01 # 1ms

sift = cv2.SIFT_create()
bf = cv2.BFMatcher()

FLANN_INDEX_KDTREE = 1
index_params = dict(algorithm = FLANN_INDEX_KDTREE, trees = 5)
search_params = dict(checks=50)   # or pass empty dictionary
flann = cv2.FlannBasedMatcher(index_params,search_params)

weapons = {}
for template in os.listdir(weapons_dir):
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
        thresh, bw = cv2.threshold(screenshot, 250,255,cv2.THRESH_BINARY);

        kp, des = sift.detectAndCompute(bw,None)

        results = []
        # iterate all weapons and show matches
        for weapon in weapons:
            wc = weapons[weapon]
            # first comes the big picture, the soruce, then what you want to find in it
            matches = flann.knnMatch(des,wc.des,k=2)
            # Apply ratio test
            good = []
            for m,n in matches:
                if m.distance < threshold * n.distance:
                    good.append(m)


            results.append((wc, len(good), good));
 

        res_sorted = sorted(results, key=lambda tup: tup[1])

        winner = res_sorted[-1]

        if winner[1] > 5:
            print(winner[0].name + " " + str(winner[1]))
        else:
            print("nothing matched")


        time.sleep(sleep)
        print()
