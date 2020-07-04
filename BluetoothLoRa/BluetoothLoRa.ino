#include <LoRa.h>
#include "BluetoothSerial.h"
//#include <QueueArray.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define SS      18
#define RST     14
#define DI0     26
#define BAND    915E6

byte msg_B     = 0xA1;

BluetoothSerial SerialBT;

//QueueArray <char> queue;


void setupLoRa()
{
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);
  LoRa.setTxPower(20);
 

  //Initialize LoRa
  if (!LoRa.begin(BAND))
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  else
  {
    Serial.println("LoRa On");
  }

  //  LoRa.setSyncWord(0xF3);         // ranges from 0-0xFF, default 0x34
  LoRa.enableCrc();             // Activate crc
}


void couple(byte byteCode, String outgoing)
{
  LoRa.write(byteCode);               // add byteCode
  LoRa.write(outgoing.length());      // add payload length
  LoRa.print(outgoing);               // add payload

  //   Displays Sent Data on OLED and Serial Monitor
  //   Serial.println("Parameter: " + outgoing);
}



void sendPayload(String msg)
{
  LoRa.beginPacket();
  couple(msg_B, msg);
 
  LoRa.endPacket();
}


String readMessages(byte mLength)
{
  String incoming = "";
  int len = 0; 
  char test = 'a';
  
  for (int i = 0; i < mLength; i++)
  {
    incoming += (char)LoRa.read();
    len++;
  }
  //Serial.println(incoming);

  SerialBT.write(test); //Send lora message over BLE
  return incoming;
}



void receive(int packetSize)
{
  if (packetSize != 0)
  {
    byte byteCode, mLength;
    Serial.print("Packet Received");
    // read packet
    int rssi;
    float snr;
    long freqErr;
    int availableBytes;

    rssi = LoRa.packetRssi();
    snr = LoRa.packetSnr();
    freqErr = LoRa.packetFrequencyError();
    availableBytes = LoRa.available();

    while (LoRa.available())
    {
      byteCode = LoRa.read();
      mLength  = LoRa.read();

      if (byteCode == msg_B)
      {
        readMessages(mLength);
        
      }
      
    }
  }
  else
  {
    return;
  }
}


void setup() {
  Serial.begin(115200);
  setupLoRa();
  SerialBT.begin("ESP32-NICK25"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  //queue.setPrinter (Serial);

}

void loop() {
  // put your main code here, to run repeatedly:
  
  receive(LoRa.parsePacket());

  if (SerialBT.available()) {
    String text = SerialBT.readString();
    Serial.write(SerialBT.read());
    //add to queue
//    for (int i = 0; i < text.length (); i++)
//    queue.enqueue(text.charAt(i));
//    queue.enqueue('~');
    sendPayload(text);
    delay(10);
  }
//
//  String msg = "";
//  while(!queue.isEmpty()){
//    char temp = queue.pop();
//    if(temp == '~') {
//      sendPayload(msg);
//      msg = "";
//    } else {
//      msg =+ temp;
//    }
//  }

  

}
