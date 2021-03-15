// By: Odalys Benitez and Leon Santen
// Purpose: Receive voltage from server and change lights

#include <SPI.h>
#include <RH_RF95.h>

// Singleton instance of the radio driver
RH_RF95 rf95;


void setup() 
{


  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

}

void loop()
{
  //sendMessageToServer();
  waitForMessageFromServer(); 
  
}

//-----------------------------------------------------------------------------------------------
// Functions for setup code
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// Functions for flight code
//-----------------------------------------------------------------------------------------------

void sendMessageToServer(){
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  uint8_t data[] = "bitch aint a good word";
  rf95.send(data, sizeof(data));
  rf95.waitPacketSent();
}

void waitForMessageFromServer(){
// Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  Serial.println(sizeof(buf));

  if (rf95.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf95_server running?");
  }
  delay(400);
}
