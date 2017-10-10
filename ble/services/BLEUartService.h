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
    /**
     * Initialize the BLE UART service using the current BLE reference.
     * Optionally adapt the buffer sizes from (default is 20 bytes, which
     * is also the max. MTU of a BLE notification packet.
     * @param _ble the ble reference
     * @param _rxBufferSize the receive buffer size
     * @param _txBufferSize the send buffer size
     */
    explicit BLEUartService(BLE &_ble, uint8_t _rxBufferSize = 20, uint8_t _txBufferSize = 20);

    /**
     * Check if we have received data.
     * @return whether there is data to read
     */
    bool isReadable();

    /**
     * Send data to the connected client.
     * @param buf the byte buffer to send
     * @param length the length of the byte buffer
     * @return how many bytes have actually been written
     */
    int send(const uint8_t *buf, int length);

    /**
     * Read incoming data into a byte buffer.
     * @param buf the buffer to read into
     * @param len the size of the buffer
     * @return how many bytes have actually been read
     */
    int read(uint8_t *buf, int len);

    /**
     * Get a single character from the input buffer. Returns EOF
     * if no data is available.
     * @return the character or EOF (-1)
     */
    int getc();

    /**
     * Put a single character in the outgoing buffer.
     * This may be very inefficient as it tries to send the data
     * immediately.
     * @param c the character to send
     * @return 1 if the data was written, else EOF (-1)
     */
    int putc(char c);


protected:
    /**
     * Get the current size of the receive buffer.
     * @return the size of the receive buffer
     */
    int rxFill();

    /**
     * Get the current size of the send buffer.
     * @return the size of the send buffer
     */
    int txFill();

    /**
     * This copies data from the internal circular buffer into a linear buffer.
     */
    void circularCopy(const uint8_t *circularBuff, uint8_t circularBuffSize, uint8_t *linearBuff, uint16_t tailPosition,
                      uint16_t headPosition);

    /**
     * BLE callback when data has been received from the connected client.
     */
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
