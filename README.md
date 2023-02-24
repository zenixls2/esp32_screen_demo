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

### HC\_SR04 setup (the ultrasonic sensor)

- VCC: 5v or 3.3v both OK
- TRIG: GPIO22
- ECHO: GPIO21

And change the flag `-DHC_SR04=1` in the platformio.ini to switch it on or off.<br>
If we switch it on, from the screen we'll see the text of distance it detects in centimeter.
