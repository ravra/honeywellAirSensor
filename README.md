# honeywellAirSensor
Arduino sketch to read data from the Honeywell 32322550 air sensor and display it on a multi-function shield.

Besides 5V and Ground, the only connection from the Arduino to the Honeywell sensor is the sensor's TX (pin 6) is connected to the Arduino's pin 5 which is configured as an RX software serial input. A YouTube video showing this connection is here: https://www.youtube.com/watch?v=vJRyTTnJo08

Further comments are in the sketch.
