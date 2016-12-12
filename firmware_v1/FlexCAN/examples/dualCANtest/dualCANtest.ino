// -------------------------------------------------------------
// CANtest for Teensy 3.6 dual CAN bus
// by Pawelsky (based on CANtest by teachop)
//
// This test transmits all data coming from CAN0 to CAN1 and vice versa (at 1Mbps)
//

#include <FlexCAN.h>

#ifndef __MK66FX1M0__
  #error "Teensy 3.6 with dual CAN bus is required to run this example"
#endif

FlexCAN CANbus0(1000000, 0);
FlexCAN CANbus1(1000000, 1);

static CAN_message_t msg;
static uint8_t hex[17] = "0123456789abcdef";


// -------------------------------------------------------------
static void hexDump(uint8_t dumpLen, uint8_t *bytePtr)
{
  uint8_t working;
  while( dumpLen-- ) {
    working = *bytePtr++;
    Serial.write( hex[ working>>4 ] );
    Serial.write( hex[ working&15 ] );
  }
  Serial.write('\r');
  Serial.write('\n');
}


// -------------------------------------------------------------
void setup(void)
{
  CANbus0.begin();
  CANbus1.begin();

  delay(1000);
  Serial.println(F("Hello Teensy 3.6 dual CAN Test."));
}


// -------------------------------------------------------------
void loop(void)
{
  if(CANbus0.available()) 
  {
    CANbus0.read(msg);
//    Serial.print("CAN bus 0: "); hexDump(8, msg.buf);
    CANbus1.write(msg);
  }

  if(CANbus1.available()) 
  {
    CANbus1.read(msg);
//    Serial.print("CAN bus 1: "); hexDump(8, msg.buf);
    CANbus0.write(msg);
  }
}