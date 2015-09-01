# EasyCap

> EasyCap driver for linux

Most of the documentation for the EasyCap devices are fragmented, incomplete, or old. This is an attempt to bring the software/firmware together for Linux systems.

### Install

### dependencies
`mplayer`

#### dirver lib

x86_64 Linux

`sudo cp ./somagic_firmware.bin /lib/firmware/somagic_firmware.bin`


#### somagic tools
`cd somagic-easycap_1.1`
`make`
`sudo make install`


### Init
`sudo somagic-init`

### Run
`sudo somagic-capture -n | mplayer -vf yadif,screenshot -demuxer rawvideo -rawvideo "ntsc:format=uyvy:fps=30000/1001" -aspect 4:3 -`

Eject and insert your easycap usb device, then run step Init as required on each video stream start.


### Alternative useage 
(Linux x86_64, git clone without compiling)

`cd somagic-easycap_1.1`
#### Init

`sudo ./somagic-init`

### Run
`sudo ./somagic-capture -n | mplayer -vf yadif,screenshot -demuxer rawvideo -rawvideo "ntsc:format=uyvy:fps=30000/1001" -aspect 4:3 -`

