# hw-demo-unit
Arduino code for marketing hardware demo unit

# First Time Startup
1. Install Arduino IDE (download availible [here](https://www.arduino.cc/en/software/) )
2. Open the application, it should open to a fresh sketch with a .ino file ready to code in. If application instead loads with sample code, create a new sketch with File -> New Sketch or Ctrl + N
3. Using the Library Manager (third in the left column or Ctrl + Shift + I), install the following libraries:
    - Adafruit BusIO
    - Adafruit NeoPixel
    - Adafruit PWM Servo Driver Library
4. Add HWDemonstarter .ino file in this repo to the sketch and save. Users can git clone this repo, but it may be easier to copy and paste the code since Arduino IDE does not interface well with git.

# Loading New Code to the Device
1. Make any necessary updates to the code and save the file
2. Connect the arduino board to computer with a USB cable
3. In the Arduino IDE, go to Tools -> Board and select the connected device. Go to Tools -> Port and select the correct port for the device.
5. Click the upload button (right arrow round button in the top left of the IDE window) to load code onto the device
6. After a short delay, the new code will be active on the device. Code previously on the device is lost as the board does not store it in memory.