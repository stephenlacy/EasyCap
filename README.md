# EasyCap

> EasyCap driver for linux



### Install

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

Eject and insert easycap usb device and run step Init as required on new stream start.
