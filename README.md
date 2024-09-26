# XookyNabox
By Rafael Vega <contacto@rafaelvega.co>
Xooky Nabox is a Jack client for running Pure Data patches on embedded devices.

## Requirements
- jack

## Compiling
```
cd /path/to/xooky_nabox/vendor/libpd/src
make  #This step may throu some errors about failing to create jni packages, just ignore.
sudo make install
cd /path/to/xooky_nabox/src
make
```

## Using
This will start the jack client:
`xooky_nabox path/to/puredata_patch.pd &`
Now you can use jack_lsp and jack_connect or QJackCTL or JackPilot or whatever you prefer to hook up the xooky_nabox ports to your sound deviceo

> Bl.
