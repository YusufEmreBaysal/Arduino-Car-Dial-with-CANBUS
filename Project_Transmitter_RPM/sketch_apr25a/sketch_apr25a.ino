// Transmitter 2 RPM

// define necessary libraries
#include <mcp2515.h>
#include <SPI.h>
#include <TimerOne.h>

// define necesary variables
#define SPI_CS_PIN 10
#define encoderPin 2

// define mcp 2515 object
MCP2515 mcp2515(SPI_CS_PIN);
struct can_frame canMsg1;

// RPM calculation variables
unsigned int rpm; // rpm variable
volatile byte pulses; // signal counter
unsigned long timeold;

unsigned int pulsesperturn = 1;

// define necesary variables
int normalizedRPM = 0;

int RPMarray[15] = {};
int averageRPM;

// pulse counter
void counter()
{
  //sayımı arttır
  pulses++;
}


void setup()
{
  // canbus message
  canMsg1.can_id  = 0x054;
  canMsg1.can_dlc = 1;
  canMsg1.data[0] = 0x00;

  while (!Serial);
  Serial.begin(96000);

  // canbus settings
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS);
  mcp2515.setNormalMode();

  // rpm sensor settings
  attachInterrupt(0, counter, FALLING);
  // start
  pulses = 0;
  rpm = 0;
  timeold = 0;
}

void loop()
{
  // calculate the rpm
  if (millis() - timeold >= 100) {
    detachInterrupt(0);
    rpm = ((60 * 100 / pulsesperturn ) / (millis() - timeold) * pulses);
    timeold = millis();
    pulses = 0;
    //Serial.print("RPM = ");
    //Serial.println(rpm, DEC);
    //restart
    attachInterrupt(0, counter, FALLING);
  }

  // map rpm value to 8 bit 
  normalizedRPM = map(rpm, 0, 6000, 0, 255);
  if ( normalizedRPM > 255) {
    normalizedRPM = 255;
  }

  canMsg1.data[0] = normalizedRPM;
  mcp2515.sendMessage(&canMsg1);  // send data

  delay(10);
}
