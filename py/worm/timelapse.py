#!/usr/bin/env python3

import cv2
import sys
import os
import time
import shutil
import numpy
import threading

srcFilename = "/run/shm/output.jpg"
targetPath  = "/home/pi/sequence/"
videoPath   = "/home/pi/sequence/"
videoFile   = "output"
imgCount    = 6*60*24
serverFile  = "/home/wormuser/worm.mp4"
username    = "user"
serverURL   = "192.168.0.2"

def scp(source, target, username, server, port=22):
  os.system('scp -P %i "%s" "%s@%s:%s"' % (port, source, username, server, target))

def genVideo(filenames):
  print("\nGenerating video")
  rawVideoWriter = cv2.VideoWriter(videoPath + videoFile + ".mp4", cv2.VideoWriter_fourcc(*"mp4v"), 20, (800, 600))
  for filename in filenames:
    img = cv2.imread(filename)
    rawVideoWriter.write(img)
    print("-", end="")
    sys.stdout.flush()
  rawVideoWriter.release()
  scp(videoPath + videoFile + ".mp4", serverFile, username, serverURL)

print("Source", srcFilename)
print("Target", targetPath)

if not os.path.isdir(targetPath):
  print("Creating output directory")
  os.mkdir(targetPath)

while True:
  print("Capturing Frames",end="")
  sys.stdout.flush()
  counter = 0
  filenames = []

  while counter < imgCount:
    if os.path.isfile(srcFilename):
      time.sleep(2)
      targetFilename = targetPath + "image_" + str(counter) + ".jpg"
      try:
        shutil.move(srcFilename, targetFilename)
        counter += 1
        filenames += [targetFilename]
        print(".",end="")
      except:
        print("x",end="")
      sys.stdout.flush()
    else:
      time.sleep(1)

  t = threading.Thread(target=genVideo, args=(filenames,))
  t.start()
