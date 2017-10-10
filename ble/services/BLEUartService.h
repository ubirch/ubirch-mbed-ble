/*!
 * @file
 * @brief BLE UART service
 *
 * Part of this service is based on the Micro:bit DAL by Lancaster University (MIT)
 * (https://github.com/lancaster-university/microbit-dal/blob/master/source/bluetooth/MicroBitUARTService.cpp)
 *
 * @author Matthias L. Jugel
 * @date   2017-10-05
 *
 * @copyright &copy; 2017 ubirch GmbH (https://ubirch.com)
 *
 * @section LICENSE
 * ```
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ```
 */
#ifndef UBIRCH_MBED_BLE_BLEUARTSERVICE_H
#define UBIRCH_MBED_BLE_BLEUARTSERVICE_H

#include <mbed.h>
#include <BLE.h>

class BLEUartService {

public:
    BLEUartService(BLE &_ble, uint8_t _rxBufferSize = 20, uint8_t _txBufferSize = 20);

    bool isReadable();

    int send(const uint8_t *buf, int length);

    int read(uint8_t *buf, int len);

    int getc();

    int putc(char c);


protected:
    int rxFill();

    int txFill();

    void circularCopy(const uint8_t *circularBuff, uint8_t circularBuffSize, uint8_t *linearBuff, uint16_t tailPosition,
                      uint16_t headPosition);

    void onDataWritten(const GattWriteCallbackParams *params);

protected:
    BLE &ble;

    uint8_t rxBufferSize;
    uint8_t txBufferSize;

    uint8_t *rxBuffer;
    uint8_t *txBuffer;

    uint8_t rxBufferHead;
    uint8_t txBufferHead;
    uint8_t rxBufferTail;
    uint8_t txBufferTail;



    uint32_t txCharacteristicHandle;

    GattCharacteristic *txCharacteristic;
};


#endif //UBIRCH_MBED_BLE_BLEUARTSERVICE_H
