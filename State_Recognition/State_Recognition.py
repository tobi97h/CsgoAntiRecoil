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
threshold = 0.5

sift = cv2.SIFT_create()
bf = cv2.BFMatcher()

weapons = {}
for template in os.listdir(weapons_dir):
    print(template)
    px_color = cv2.imread(os.path.join(weapons_dir, template), cv2.IMREAD_GRAYSCALE)
    kp, des = sift.detectAndCompute(px_color,None)
    wc = Weapon(template, kp, des, px_color)
    weapons[template] = wc


with mss() as sct:

    # 0 monitor is all monitors - 1 is primary
    monitor = sct.monitors[1] 
   

    while True:
        screenshot = cv2.cvtColor(np.array(sct.grab(monitor)), cv2.COLOR_RGB2GRAY)

        kp, des = sift.detectAndCompute(screenshot,None)

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


            results.append((wc, len(good), good));
 

        res_sorted = sorted(results, key=lambda tup: tup[1])

        for res in res_sorted:
            print(res[0].name + " " + str(res[1]))
        winner = res_sorted[-1]
    


        # => dont seem to need pixel color selection, because key points are different in selected and non selected weapons
        goods = winner[2]
        winner_wc = winner[0]

        for good in goods:
        
            kp = winner_wc.kp[good.trainIdx]
    
            px_color = winner_wc.img[round(kp.pt[1]), round(kp.pt[0])]

            print(px_color)

        time.sleep(5)
        print()
