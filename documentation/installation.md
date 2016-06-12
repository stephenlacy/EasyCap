Installation
============

Copy the firmware
-----------------
```bash
sudo cp ./somagic_firmware.bin /lib/firmware/somagic_firmware.bin
```

Install dependencies
------------
* make
* gcc
* libusb-1.0-0-dev
* libgcrypt11-dev

Example for debian/ubuntu
```bash
sudo apt-get install make gcc libusb-1.0-0-dev libgcrypt11-dev
```


Compile from source
---------------------
```bash
cd somagic-easycap_1.1

make
sudo make install

make beta
sudo make install-beta
```

Results
-------
Now, you have installed : 
* somagic-init
* somagic-capture
* somagic-audio-capture
* somagic-both

Please see [how to use](usage.md) the tools now
