 
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define CLIENT_ADDRESS 2
#define SERVER_ADDRESS 1

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Frequency
#define RF95_FREQ 915.0


// Blinky on receipt
#define LED 13

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);


void setup() 
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  //while (!Serial) {
  //  delay(1);
  //}
  delay(100);

  Serial.println("Feather LoRa RX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!manager.init())
    Serial.println("init failed");

  if (!driver.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
    
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  driver.setTxPower(5, false);
  
  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
//  driver.setCADTimeout(10000);

  driver.setModemConfig(2);  /// Bw31_25Cr48Sf512 < Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range

}

char radiopacket[RH_RF95_MAX_MESSAGE_LEN] =   "";   //packet that will be transmitted

char data[RH_RF95_MAX_MESSAGE_LEN] = "1000";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{

  String bufferOut = "";
  delay(1000); // Wait 1 second between transmits, could also 'sleep' here!
  Serial.println("Hearing for any well transmitting..."); // Send a message to rf95_server
  
  if (manager.available())
  {
    
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      digitalWrite(LED, HIGH);
      Serial.print("got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(driver.lastRssi(), DEC);

      bufferOut += "1000 ";
      bufferOut += driver.lastRssi();

      bufferOut.toCharArray(radiopacket, RH_RF95_MAX_MESSAGE_LEN);

      // Send a reply back to the originator client
      if (!manager.sendtoWait(radiopacket, sizeof(radiopacket), from))
        Serial.println("send reply failed");
      digitalWrite(LED, LOW);
    }
  }
}


