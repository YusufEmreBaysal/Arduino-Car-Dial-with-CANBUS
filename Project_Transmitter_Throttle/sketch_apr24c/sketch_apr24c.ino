// Transmitter Throttle (Potentiometer)

// define necessary libraries
#include <mcp2515.h>
#include <SPI.h>
#define SPI_CS_PIN 10

// define mcp 2515 object
MCP2515 mcp2515(SPI_CS_PIN);

// define variables
struct can_frame canMsg1;
int potVal;
int mappedPotVal;

void setup()
{
  // canbus message
  canMsg1.can_id  = 0x05B;
  canMsg1.can_dlc = 1;
  canMsg1.data[0] = 0x00;

  while (!Serial);
  Serial.begin(9600);

  // canbus settings
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS);
  mcp2515.setNormalMode();

  pinMode(A7, INPUT);
}

void loop()
{
  potVal = analogRead(A7);    // read potentiometer income value
  mappedPotVal = map(potVal, 0, 1030, 0, 255); // map incoming 10 bit value to 8 bit value
  if (mappedPotVal > 255) {
    mappedPotVal = 255;
  }

  canMsg1.data[0] = mappedPotVal;
  mcp2515.sendMessage(&canMsg1);  // send data

  delay(10);
}
