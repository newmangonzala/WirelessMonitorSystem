
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define CLIENT_ADDRESS 2
#define SERVER_ADDRESS 1

// for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

//Frequency
#define RF95_FREQ 915.0

int analogPin = A0;     // potentiometer wiper (middle terminal)

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);


void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  while (!Serial) {
    delay(1);
    Serial.println("Serial not connected");
  }
  delay(100);

  Serial.println("Feather LoRa TX Test!");

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
  driver.setTxPower(23, false);

  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
//  driver.setCADTimeout(10000);
}

char radiopacket[20] =   "";   //packet that will be transmitted
uint32_t syncTime;            //This is the synchronization time from hub

// Dont put this on the stack:
char buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  int syncT = 0;
  Serial.println("Sleeping..");
  delay(syncT); // Wait 1 second between transmits, could also 'sleep' here!
  Serial.println("Sending to Hub");
  
  char bufferVoltage[7] = "";    //buffer to hold voltageIN
  int val = 0;                   // variable to store the value read
  float vout = 0;                // voltage out
  String bufferOut = "";         //string to append all values
  float bLife = 0;               //battery life
  
  
  val = analogRead(analogPin);          // read the input pin
  vout = 0.00292968 * val;              //convert back to voltage
  dtostrf(vout, 4, 3, bufferVoltage);   //decimal to char

  bufferOut += vout;                  
  bufferOut += " ";
  bufferOut += bLife;

  bufferOut.toCharArray(radiopacket, 20);
    
  // Send a message to manager_server
  if (manager.sendtoWait(radiopacket, sizeof(radiopacket), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);

      syncT = atoi(buf);
      Serial.println(syncT);
    }
    else
    {
      Serial.println("No reply, is the hub running?");
    }
  }
  else
    Serial.println("sendtoWait failed");
  delay(500);
}


