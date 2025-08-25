# hw-demo-unit
Arduino code for marketing hardware demo unit

# First Time Startup
1. Install Arduino IDE (download availible [here](https://www.arduino.cc/en/software/) )
2. Open the application, it should open to a fresh sketch with a .ino file ready to code in. If application instead loads with sample code, create a new sketch with File -> New Sketch or Ctrl + N
3. Using the Library Manager (third in the left column or Ctrl + Shift + I), install the following libraries:
    - Adafruit BusIO
    - Adafruit NeoPixel
    - Adafruit PWM Servo Driver Library
4. Install the Board Manager. This can be done multiple ways:
    * After plugging in board to computer for the first time, Arduino IDE should have a popup in the bottom right corner asking if board software should be installed. Click to install.
    * Go to Board Manager (second in left column or Ctrl + Shift + B) and install Arduino AVR Boards.
5. Add `HWDemonstarter.ino` file in this repo to the sketch and save. Users can git clone this repo, but it may be easier to copy and paste the code since Arduino IDE does not interface well with git.

# Loading New Code to the Device
1. Make any necessary updates to the code and save the file.
2. Connect the arduino board to computer with a USB cable.
3. Once the device is recognized, it should appear in the dropdown in the top ribbon with a USB symbol. Select the option for "Arduino Duo (Native USB Port)".
    * If board is not appearing, try going to Tools -> Board and selecting the connected device. Go to Tools -> Port and select the correct port as well.
5. Click the upload button (right arrow round button in the top left of the IDE window) to load code onto the device.
6. After a short delay, the new code will be active on the device. Code previously on the device is lost as the board does not store it in memory.

### Troubleshooting
- If the device freezes while loading new code, try power cycling the device.
- If the device is not showing up as a connection option in Arduino IDE or upload is failing with a message related to no device found, ensure the device is connected correctly and recognized in the computer's device manager.