#!/bin/bash

while true; do ffmpeg -f video4linux2 -s 800x600 -framerate 20 -i /dev/video0 -vf hue=s=0 -c:v h264_omx -threads 0 -an -b:v 1234k -f flv "rtmp://dus01.contribute.live-video.net/app/YOUR_KEY_HERE" -update 1 -r 1/10 -y -vf hue=s=0 /run/shm/output.jpg && break; done
