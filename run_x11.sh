#!/bin/bash
killall ibus-daemon 2>/dev/null
xhost +SI:localuser:root
sudo /home/truonghieuvm/Uk362/build/unikey-x11
