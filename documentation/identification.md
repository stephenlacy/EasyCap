#Identification

You can type this command in a terminal, to know if your device is supported
```bash
lsusb
```

The output should be something like this :
```bash
Bus XXX Device XXX: ID 1c88:0007 Somagic, Inc. 
```

After loading le firmware (see [installation](installation.md)) you will get : 
```bash
Bus XXX Device XXX: ID 1c88:003c Somagic, Inc. or Bus XXX Device XXX: ID 1c88:003f Somagic, Inc.
```
