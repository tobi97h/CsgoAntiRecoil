
import numpy as np
import cv2
import matplotlib.pyplot as plt
import pyautogui
from mss import mss
import os
from PIL import Image

meth ='cv2.TM_CCOEFF_NORMED'
weapons_dir = "weapons1080p";

weapons = {}
for template in os.listdir(weapons_dir):
    print(template)
    weapons[template] = cv2.imread(os.path.join(weapons_dir, template), cv2.IMREAD_GRAYSCALE)

with mss() as sct:

    # 0 monitor is all monitors - 1 is primary
    monitor = sct.monitors[1] 
   
    screenshot =  cv2.cvtColor(np.array(sct.grab(monitor)), cv2.COLOR_RGB2GRAY)
   
    results = []
    # iterate all weapons and show matches
    for weapon in weapons:
        template = weapons[weapon]
        w, h = template.shape[::-1]
       
        method = eval(meth)

        # Apply template Matching
        res = cv2.matchTemplate(screenshot,template,method)
        min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)

        results.append((weapon, max_val));
 

    winner = sorted(results, key=lambda tup: tup[1])[-1];
    print(winner)
