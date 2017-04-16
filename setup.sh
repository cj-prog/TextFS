#!/bin/sh

# sfml-window-dependencies
apt-get install libx11-dev -y
apt-get install libxcb1-dev -y
apt-get install libx11-xcb-dev -y
apt-get install libxcb-randr0-dev -y
apt-get install libxcb-image0-dev -y
apt-get install libgl1-mesa-dev -y
apt-get install libudev-dev -y

# sfml-graphics-dependencies
apt-get install libfreetype6-dev -y
apt-get install libjpeg-dev -y

# sfml-audio-dependencies
apt-get install libopenal-dev -y
apt-get install libflac-dev -y
apt-get install libvorbis-dev -y 

# sfml-main-libraries
apt-get install libsfml-dev -y
apt-get install libcsfml-dev -y
apt-get install libsfml-doc -y
apt-get install libcsfml-doc -y

make
