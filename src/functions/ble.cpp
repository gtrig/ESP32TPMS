#include <Arduino.h>
#include "BLEDevice.h"
#include "functions/tpms.h"

BLEScan *pBLEScan;
BLEClient *pClient;

// Variables
static BLEAddress *pServerAddress;
// TPMS BLE SENSORS known addresses
String knownAddresses[] = {"81:ea:ca:22:d5:e7", "82:ea:ca:33:02:d7"};
String FrontAddress = "81:ea:ca:22:d5:e7";
String BackAddress = "82:ea:ca:33:02:d7";

/* BLE functions */
static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  // Serial.print("Notify callback for characteristic ");
  // Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  // Serial.print(" of data length ");
  // Serial.println(length);
}

String retmanData(String txt, int shift)
{
  // Return only manufacturer data string
  int start = txt.indexOf("data: ") + 6 + shift;
  return txt.substring(start, start + (36 - shift));
}

byte retByte(String Data, int start)
{
  // Return a single byte from string
  int sp = (start) * 2;
  char *ptr;
  return strtoul(Data.substring(sp, sp + 2).c_str(), &ptr, 16);
}

long returnData(String Data, int start)
{
  // Return a long value with little endian conversion
  return retByte(Data, start) | retByte(Data, start + 1) << 8 | retByte(Data, start + 2) << 16 | retByte(Data, start + 3) << 24;
}

int returnBatt(String Data)
{
  // Return battery percentage
  return retByte(Data, 16);
}

int returnAlarm(String Data)
{
  // Return battery percentage
  return retByte(Data, 17);
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice Device)
  {
    // Serial.print("BLE Advertised Device found: ");
    // Serial.println(Device.toString().c_str());
    pServerAddress = new BLEAddress(Device.getAddress());
    bool known = false;
    bool Master = false;
    String ManufData = Device.toString().c_str();
    for (int i = 0; i < (sizeof(knownAddresses) / sizeof(knownAddresses[0])); i++)
    {
      if (strcmp(pServerAddress->toString().c_str(), knownAddresses[i].c_str()) == 0)
        known = true;
    }
    if (known)
    {
      String instring = retmanData(ManufData, 0);
      int Alarm = returnAlarm(instring);
      int Battery = returnBatt(instring);
      // pressure and temperature need to have only 1 decimal
      float Pressure = (float)returnData(instring, 8) / 6894.76;
      float Temperature = (float)returnData(instring, 12) / 100.0;

      if (strcmp(pServerAddress->toString().c_str(), FrontAddress.c_str()) == 0)
      {
        updateFrontTyreValues(Battery, Pressure, Temperature);
      }
      else if (strcmp(pServerAddress->toString().c_str(), BackAddress.c_str()) == 0)
      {
        updateBackTyreValues(Battery, Pressure, Temperature);
      }

      Device.getScan()->stop();
      delay(100);
    }
  }
};

void bleInit()
{
  Serial.print("Init BLE. ");
  BLEDevice::init("");
  pClient = BLEDevice::createClient();
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  Serial.println("Done");
}

void bleScan()
{
  Serial.println("Start BLE scan");
  pBLEScan->start(5);
  Serial.println("Scan done");
}