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

#include <UARTService.h>
#include "BLEUartService.h"

// some static stuff, because the underlying lib does not handle object pointers here
static uint8_t txBufferHead = 0;
static uint8_t txBufferTail = 0;

static GattCharacteristic *rxCharacteristic = NULL;
static volatile bool txEmpty = false;

void onConfirmationReceived(uint16_t handle) {
    if (handle == rxCharacteristic->getValueAttribute().getHandle()) {
        txBufferTail = txBufferHead;
        txEmpty = true;
    }
}

BLEUartService::BLEUartService(BLE &_ble, uint8_t _rxBufferSize, uint8_t _txBufferSize)
: ble(_ble),
  rxBufferSize(static_cast<uint8_t>(_rxBufferSize + 1)),
  txBufferSize(static_cast<uint8_t>(_txBufferSize + 1)),
  rxBuffer(new uint8_t[rxBufferSize]),
  txBuffer(new uint8_t[txBufferSize]),
  rxBufferHead(0), txBufferHead(0), rxBufferTail(0), txBufferTail(0) {
    txCharacteristic = new GattCharacteristic(UARTServiceTXCharacteristicUUID,
                                              rxBuffer, 1, static_cast<uint16_t>(rxBufferSize),
                                              GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                                              GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
    rxCharacteristic = new GattCharacteristic(UARTServiceRXCharacteristicUUID,
                                              txBuffer, 1, static_cast<uint16_t>(txBufferSize),
                                              GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                              GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);

    GattCharacteristic *charTable[] = {txCharacteristic, rxCharacteristic};
    GattService uartService(UARTServiceUUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
    ble.addService(uartService);

    this->txCharacteristicHandle = txCharacteristic->getValueAttribute().getHandle();

    ble.gattServer().onDataWritten(this, &BLEUartService::onDataWritten);
    ble.gattServer().onConfirmationReceived(onConfirmationReceived);
}

bool BLEUartService::isReadable() {
    return (rxBufferTail != rxBufferHead);
}

int BLEUartService::send(const uint8_t *buf, int length) {
    if (length < 1) return EOF;

//    bool updatesEnabled = false;

//    ble.gattServer().areUpdatesEnabled(*rxCharacteristic, &updatesEnabled);

    if (!ble.getGapState().connected/* && !updatesEnabled*/)
        return EOF;

    int bytesWritten = 0;

    while (bytesWritten < length && ble.getGapState().connected/* && updatesEnabled*/) {

        for (int bufferIterator = bytesWritten; bufferIterator < length; bufferIterator++) {
            uint8_t nextHead = static_cast<uint8_t>((txBufferHead + 1) % txBufferSize);

            if (nextHead != txBufferTail) {
                txBuffer[txBufferHead] = buf[bufferIterator];
                txBufferHead = nextHead;
                bytesWritten++;
            }
        }

        uint8_t size = static_cast<uint8_t>(txFill());
        uint8_t *temp = new uint8_t[size];
        memset(temp, 0, size);

        circularCopy(txBuffer, txBufferSize, temp, txBufferTail, txBufferHead);

        ble_error_t error = ble.gattServer().write(rxCharacteristic->getValueAttribute().getHandle(), temp, size);
        if(error == BLE_ERROR_NONE) txBufferTail = txBufferHead;

//        ble.gattServer().areUpdatesEnabled(*rxCharacteristic, &updatesEnabled);
    }

    return bytesWritten;
}

int BLEUartService::read(uint8_t *buf, int len) {
    int i = 0;
    int c;

    while (i < len && (c = getc()) >= 0) {
        buf[i] = static_cast<uint8_t>(c);
        i++;
    }

    return i;
}

int BLEUartService::getc() {
    if (!isReadable()) return EOF;

    char c = rxBuffer[rxBufferTail];
    rxBufferTail = static_cast<uint8_t>((rxBufferTail + 1) % rxBufferSize);
    return c;
}

int BLEUartService::putc(char c) {
    return (send((uint8_t *) &c, 1) == 1) ? 1 : EOF;
}

int BLEUartService::rxFill() {
    if (rxBufferTail > rxBufferHead)
        return (rxBufferSize - rxBufferTail) + rxBufferHead;
    return rxBufferHead - rxBufferTail;
}

int BLEUartService::txFill() {
    if (txBufferTail > txBufferHead)
        return (txBufferSize - txBufferTail) + txBufferHead;
    return txBufferHead - txBufferTail;
}

void BLEUartService::circularCopy(const uint8_t *circularBuff, uint8_t circularBuffSize, uint8_t *linearBuff,
                                  uint16_t tailPosition, uint16_t headPosition) {
    int toBuffIndex = 0;

    while (tailPosition != headPosition) {
        linearBuff[toBuffIndex++] = circularBuff[tailPosition];
        tailPosition = static_cast<uint16_t>((tailPosition + 1) % circularBuffSize);
    }
}

void BLEUartService::onDataWritten(const GattWriteCallbackParams *params) {
    if (params->handle == this->txCharacteristicHandle) {
        for (int byteIterator = 0; byteIterator < params->len; byteIterator++) {
            uint8_t bufferHead = static_cast<uint8_t>((rxBufferHead + 1) % rxBufferSize);
            if (bufferHead != rxBufferTail) {
                char c = params->data[byteIterator];
                rxBuffer[rxBufferHead] = static_cast<uint8_t>(c);
                rxBufferHead = bufferHead;
            }
        }
    }
}

