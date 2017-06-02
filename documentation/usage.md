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

###### Convert while capturing (NTSC)

``` bash
sudo killall -9 somagic-both
sudo somagic-init
rm .audio .video video.avi
sudo somagic-both -n 2>.audio | avconv \
-f rawvideo -pix_fmt uyvy422 -r 30 -s:v 720x480 -i pipe:0 \
-vcodec mpeg4 -vtag xvid -qscale:v 7 \
-vf yadif -s:v 720x540 \
-f avi .video

# type ctrl+c to finish capturing and converting
# merge raw audio to video

avconv \
-f avi -i .video \
-f s16le -sample_rate 24000 -ac 2 -i .audio -strict experimental \
-vcodec mpeg4 -vtag xvid -qscale:v 7 \
-vf yadif -s:v 720x540 \
-f avi video.avi

```




