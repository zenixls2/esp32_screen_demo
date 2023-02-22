FPS test: ESP32 DevkitC V4
==========================

The pin set is defined in platformio.ini<br>
Port is also defined in the file using `/dev/ttyUSB1`.

Any monitor using `ST7789\_v2` controller should work properly with this setup.

Prepare:

```bash
virtualenv venv
source venv/bin/activate
pip install platformio
```

To compile and upload:

```bash
pio run -t upload
```

To monitor

```bash
pio device monitor
```
