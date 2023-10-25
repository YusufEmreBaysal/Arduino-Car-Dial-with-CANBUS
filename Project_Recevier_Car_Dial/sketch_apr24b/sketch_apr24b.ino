// Receiver Car Dial (Motor, LCD Display and Servo Motor)

// include necessary libraries
#include <SPI.h>
#include <mcp2515.h>

#include <Servo.h>
#include <TimerOne.h>

// LCD Libaries
#include<Wire.h>
#include<LiquidCrystal_I2C.h>

// define necessary variables
#define SPI_CS_PIN 49
#define servo

#define topButton 11
#define bottomButton 12

bool errorState = false;

int potValReceived = 0;   // incoming potentimeter value
int rpmValReceived = 0;   // incoming rpm sensor value

int Rpwm_pin = 6;    //pin of controlling speed---- ENB of motor driver board
int pinRB = 4;       //pin of controlling diversion----IN3 of motor driver board
int pinRF = 5;       //pin of controlling diversion----IN4 of motor driver board

unsigned char motorSpeed = 100;

// tru rom value variables
int RPMarray[25] = {};
int averageRPM = 0;
int correctedRPM = 0;

struct can_frame canMsg;

// create objects
MCP2515 mcp2515(SPI_CS_PIN);
Servo myservo;
LiquidCrystal_I2C lcd( 0x27, 16, 2);

void setup()
{
  Serial.begin(9600);

  // canbus module settings
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS);
  mcp2515.setNormalMode();

  // define connected servo pin
  myservo.attach(7);

  pinMode(pinRB, OUTPUT);     // pin 7--IN3 of motor driver board
  pinMode(pinRF, OUTPUT);     // pin 8--IN4 of motor driver board
  pinMode(Rpwm_pin, OUTPUT);  // pin 10 (PWM) --ENB of motor driver board
  pinMode(topButton, INPUT);  // button pin
  pinMode(bottomButton, INPUT); // button 2 pin

  // lcd settings
  lcd.init();
  lcd.backlight();
}

void loop()
{

  // button controls for error
  if (digitalRead(topButton) == HIGH ) {
    errorState = false;
    potValReceived = 0;
    rpmValReceived = 0;
    lcd.clear();
    delay(50);
  }
  if (digitalRead(bottomButton) == HIGH) {
    errorState = true;
    potValReceived = 0;
    rpmValReceived = 0;
    lcd.clear();
    delay(50);
  }

  // Potantiometer Income Value
  canMsg.can_id == 0x00;  // set adress to zero

  // control if the message is received
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {

    // control incoming message adress
    if (canMsg.can_id == 0x05B) {

      // display the incoming adress values to Serial Monitor
      //      Serial.print("CAN ID: ");
      //      Serial.print(canMsg.can_id, HEX); // print ID
      //      Serial.print(" ");
      //      Serial.print("CAN DLC: ");
      //      Serial.print(canMsg.can_dlc, HEX); // print DLC
      //      Serial.println(" ");
      //      Serial.println(canMsg.data[0]);
      //      for (int i = 0; i < canMsg.can_dlc; i++)  { // print the data
      //        Serial.print(canMsg.data[i], HEX);
      //        Serial.print(" ");
      //      }
      //      Serial.println();

      // Code Segment
      potValReceived = canMsg.data[0]; // received data

      // control the dc motor via motor driver
      if (errorState == false) {
        analogWrite(Rpwm_pin, potValReceived);
        digitalWrite(pinRB, HIGH);
        digitalWrite(pinRF, LOW);
      }

      // display the incoming canbus value and shut the motor for error detection
      else if (errorState == true) {
        if (millis() % 5 == 0) {
          analogWrite(Rpwm_pin, 0);
          digitalWrite(pinRB, LOW);
          digitalWrite(pinRF, LOW);

          lcd.setCursor(0, 0);
          lcd.print("Pot In:          ");
          lcd.setCursor(0, 0);
          lcd.print("Pot In: ");
          lcd.print(potValReceived);
          lcd.print(" ");
          lcd.print(potValReceived, HEX);
        }
      }
    }

    // RPM Sensor Income
    // control the incoming message adress
    if (canMsg.can_id == 0x054) {
      
      //      Serial.print("CAN ID: ");
      //      Serial.print(canMsg.can_id, HEX); // print ID
      //      Serial.print(" ");
      //      Serial.print("CAN DLC: ");
      //      Serial.print(canMsg.can_dlc, HEX); // print DLC
      //      Serial.print(" ");
      //
      //      for (int i = 0; i < canMsg.can_dlc; i++)  { // print the data
      //        Serial.print(canMsg.data[i], HEX);
      //        Serial.print(" ");
      //      }
      //      Serial.println();

      // Code Segment
      rpmValReceived = canMsg.data[0]; // received data

      // take the average of last 25 values of rpm to let the system work more stable
      for (int j = 0; j < 24; j++) {
        RPMarray[j] = RPMarray[j + 1];
      }
      RPMarray[24] = rpmValReceived;

      for (int j = 0; j < 24; j++) {
        averageRPM = averageRPM + RPMarray[j];
      }
      averageRPM = averageRPM / 25;
      
      myservo.write(map(averageRPM, 0, 255, 180, 0));   // adjust the servo motor position
      correctedRPM = map(averageRPM, 0, 255, 0, 6000);  // decode the rpm

      // display the values to LCD display
      if (errorState == false) {
        if (millis() % 10 == 0) {
          lcd.setCursor(0, 0);
          lcd.print("RPM:       ");
          lcd.setCursor(0, 0);
          lcd.print("RPM: ");
          lcd.print(correctedRPM);

          lcd.setCursor(0, 1);
          lcd.print("Speed: ");
          lcd.print(correctedRPM * 0.213 * 0.06);
          lcd.print("km/h  ");
        }
      }
    }
  }
}
