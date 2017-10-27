/*!
 * @file
 * @brief BLE configuration and initialization.
 *
 * Use this class to set up and initialize the BLE stack the way
 * you want. A sensible default is provided that initializes BLE
 * to just work.
 *
 * @author Matthias L. Jugel
 * @date   2017-10-12
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
#ifndef UBIRCH_MBED_BLE_BLECONFIG_H
#define UBIRCH_MBED_BLE_BLECONFIG_H

#include <BLE.h>
#include <cstdio>

// for debugging purposes, get a message what went wrong
#define PRINTF printf
#define BLE_ASSERT(c, m) {if((c) != BLE_ERROR_NONE) {PRINTF("assert(" m ")"); return c;}}

class BLEConfig {
public:
    const char *deviceName;
    uint16_t advertisingInterval;
    uint16_t advertisingTimeout;

    /**
     * Default configuration parameters for the BLE stack.
     * @param deviceName the device name used in advertising
     * @param advertisingInterval the advertising interval
     * @param advertisingTimeout how long to advertise, 0 means no timeout
     */
    explicit BLEConfig(const char *deviceName = "BLEDEVICE",
                       uint16_t advertisingInterval = 10, uint16_t advertisingTimeout = 0);

    virtual ~BLEConfig() {};

    virtual ble_error_t onInit(BLE& ble);

    virtual void onConnection(const Gap::ConnectionCallbackParams_t *params);;

    virtual void onDisconnection(const Gap::DisconnectionCallbackParams_t *params);
};


#endif //UBIRCH_MBED_BLE_BLECONFIG_H
