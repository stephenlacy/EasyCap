Usage
=====

Command description
-------------------


Please [install](installation.md) the tools before using them.

* **somagic-init**.
Load the firmware and will try to connect to the easycap device
* **somagic-capture**.
Record video, output the raw stream in stdout
* **somagic-audio-capture**.
Record audio, output the raw stream in stdout
* **somagic-both**.
Record video and audio, output the video in stdout and the audio in stderr

The format of the raw video and audio depends on the options your choose.
Your can find more information using man page (or in the examples below)
```bash
man somagic-capture
```


Live visualization of the video
-------------------------------

Dependencies
* mplayer

###### PAL format
```bash
# kill untracked zombie processes (could be usefull if the script is killed)
sudo killall -9 somagic-capture

# init the somagic driver
sudo somagic-init

sudo somagic-capture | mplayer -vf yadif,screenshot -demuxer rawvideo -rawvideo "pal:format=uyvy:fps=25" -aspect 4:3 -
```

###### NTSC format
```bash
# kill untracked zombie processes (could be usefull if the script is killed)
sudo killall -9 somagic-capture

# init the somagic driver
sudo somagic-init

# recording
sudo somagic-capture -n | mplayer -vf yadif,screenshot -demuxer rawvideo -rawvideo "ntsc:format=uyvy:fps=30000/1001" -aspect 4:3 -
```

###### SECAM format
```bash
# kill untracked zombie processes (could be usefull if the script is killed)
sudo killall -9 somagic-capture

# init the somagic driver
sudo somagic-init

# recording
sudo somagic-capture --secam | mplayer -vf yadif,screenshot -demuxer rawvideo -rawvideo "pal:format=uyvy:fps=25" -aspect 4:3 -
```

Record video and audio.
---------------------------------------

This section is a cookbook. Feel free to add yours solutions!

Dependencies:
* libav-tools (avconv)
* moreutils   (buffer)

###### Secam format

``` bash
sudo killall -9 somagic-both
sudo killall -9 somagic-capture

# init the somagic driver
sudo somagic-init

# recording
rm -f  .video .audio .video_buffer .audio_buffer
mkfifo .video .audio .video_buffer .audio_buffer
sudo somagic-both --secam 1>.video 2>.audio & pid=$!

# buffer the data acquired (prevent frame lost)
buffer < .video > .video_buffer &
buffer < .audio > .audio_buffer &

sleep 1

avconv \
-f rawvideo -pix_fmt uyvy422 -r 25 -s:v 720x576 -i .video_buffer \
-f s16le -sample_rate 24000 -ac 2 -i .audio_buffer -strict experimental \
-vcodec mpeg4 -vtag xvid -qscale:v 7 \
-vf yadif -s:v 720x540 \
video.avi

# now, type ctrl-c to stop the encoding
# you can then kill somagic-both
```

###### PAL RTSP streaming using RTSP simpe server

Deploy docker image with defaults according to instructios from https://github.com/aler9/rtsp-simple-server
Use the following setup script.

```
sudo killall -9 somagic-both
sudo killall -9 somagic-capture

# init the somagic driver
sudo somagic-init

# recording
rm -f  .video .audio
mkfifo .video .audio 

sudo somagic-both --pal 1>.video 2>.audio & pid=$!

sleep 3

ffmpeg \
-re -f rawvideo -pix_fmt uyvy422 -r 25 -s:v 720x576 -i .video \
-vf scale=1280:720 -rtsp_transport tcp -f rtsp rtsp://localhost:8554/vid &

ffmpeg \
-re -f s16le -sample_rate 24000 -ac 2 -i .audio \
-vf scale=1280:720 -rtsp_transport tcp -f rtsp rtsp://localhost:8554/aud &

sleep 3

ffmpeg -loglevel verbose -i rtsp://localhost:8554/vid \
-i rtsp://localhost:8554/aud \
-acodec copy -vcodec copy \
-map 0:0 -map 1:0 -rtsp_transport tcp -f rtsp rtsp://localhost:8554/live

killall ffmpeg
#
# now, type ctrl-c to stop the encoding
# you c then kill somagic-both
sudo killall -9 somagic-both
sudo killall -9 somagic-capture
```
