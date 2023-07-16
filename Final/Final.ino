#include <Arduino.h>  //standard Arduino library
#include <BleGamepad.h> //BLEGamepad library
#include <Wire.h>   //for the i2c communication
#include <LSM303.h> //accelerometer library
#include <math.h>   //library for angle calculation

#define numOfButtons 64
#define numOfHatSwitches 4
#define BUTTON_PIN_THROTTLE 5
#define BUTTON_PIN_BREAK 18

LSM303 compass; //LSM303 object
BleGamepad bleGamepad;  //BLEgamepad object
BleGamepadConfiguration bleGamepadConfig; //BLEGamepad config object

char report[80];
float theta, thetaDegres;  //variables to store angle
int adjustedDegreeValue;
int buttonStateThrottle, buttonStateBreak;  //variables for buttons

void setup()
{
    //Bluetooth config
    Serial.begin(115200);
    Serial.println("Starting BLE work!");
    bleGamepadConfig.setAutoReport(false);
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
    bleGamepadConfig.setButtonCount(numOfButtons);
    bleGamepadConfig.setHatSwitchCount(numOfHatSwitches);
    bleGamepadConfig.setVid(0xe502);
    bleGamepadConfig.setPid(0xabcd);
    // Some non-Windows operating systems and web based gamepad testers don't like min axis set below 0, so 0 is set by default
    //bleGamepadConfig.setAxesMin(0x8001); // -32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
	bleGamepadConfig.setAxesMin(0x0000); // 0 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
  bleGamepadConfig.setAxesMax(0x7FFF); // 32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal 
  bleGamepad.begin(&bleGamepadConfig); // Simulation controls, special buttons and hats 2/3/4 are disabled by default

  if(Wire.begin()){
    Serial.println("I2C communcation initialization sucessful.");
  }
  else {
    Serial.println("I2C communcation initialization did not succeed. Check connecions.");
  }
  
  if (compass.init()){
    Serial.println("Accelerometer initialization sucessful.");
  }
  else{
    Serial.println("Accelerometer initialization did not succeed. Check connecions.");
  }

  compass.enableDefault();
  //angles initial values
  theta = 0;
  thetaDegres = 0;
  adjustedDegreeValue = 0;
  buttonStateThrottle = 0;
  buttonStateBreak = 0;
  //configure pins as input
  pinMode(BUTTON_PIN_THROTTLE, INPUT);
  pinMode(BUTTON_PIN_BREAK, INPUT);
}

void loop()
{
    if (bleGamepad.isConnected())
    {   
        compass.readAcc();  //read only accelleration x,y,z
        thetaDegres = atan2(compass.a.x, compass.a.z)*180/M_PI; //theta angle in degrees, frrom -PI to +PI
        theta = atan2(compass.a.x, compass.a.z);  //theta angle in radians
        adjustedDegreeValue = map(thetaDegres, -180, 180, 32737, 0);  //map degres on values used in BLE Gamepad

        //Left Thumb value update
        bleGamepad.setLeftThumb(adjustedDegreeValue, 0);
        //bleGamepad.setAxes(); // Reset all axes to zero
        bleGamepad.sendReport();


        //Throtle and Break
        buttonStateThrottle = digitalRead(BUTTON_PIN_THROTTLE);
        buttonStateBreak = digitalRead(BUTTON_PIN_BREAK);

        if (buttonStateThrottle == 1) {
          bleGamepad.press(BUTTON_6);
        }
        else {
          bleGamepad.release(BUTTON_6);
        }
        if (buttonStateBreak == 1) {
          bleGamepad.press(BUTTON_5);
        }
        else {
          bleGamepad.release(BUTTON_5);
        }
        snprintf(report, sizeof(report), "Throttle =  %d  ,  Break = %d", buttonStateThrottle, buttonStateBreak); //for debug purpose
        Serial.println(report);

        bleGamepad.sendReport();


    }
}