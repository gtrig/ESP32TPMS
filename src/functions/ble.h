#ifndef BLE_H
#define BLE_H
#include <Arduino.h>
#include "BLEDevice.h"
#include "functions/tpms.h"

/* BLE functions */
static void notifyCallback(
    BLERemoteCharacteristic* pBLERemoteCharacteristic,
    uint8_t* pData,
    size_t length,
    bool isNotify);

String retmanData(String txt, int shift);
byte retByte(String Data, int start);
long returnData(String Data, int start);
int returnBatt(String Data);
int returnAlarm(String Data);

void bleInit();
void bleScan();

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice);
};



#endif