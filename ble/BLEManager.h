/*!
 * @file
 * @brief BLEManager - handle BLE initialization and services
 *
 * @author Matthias L. Jugel
 * @date   2017-10-02
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
#ifndef UBIRCH_MBED_BLE_BLEMANAGER_H
#define UBIRCH_MBED_BLE_BLEMANAGER_H

#include <BLE.h>

class BLEManager {

public:
    /**
     * Get a singleton of this manager.
     * @return a single instance reference.
     */
    static BLEManager &getInstance();

    ~BLEManager() {
        BLE::Instance().shutdown();
    };

    /**
     * The actual initialization of the BLE instance.
     *
     * @param deviceName the name of  the BLE device for advertisement
     * @return if the initialization was successful
     */
    ble_error_t init(const char *deviceName = "BLEDEVICE",
                     uint16_t advertisingInterval = 10,
                     uint16_t advertisingTimeout = 0);

    /**
     * Handle the connection of a BLE device.
     */
    void onConnection(const Gap::ConnectionCallbackParams_t *params) {
        // no special handling
    }

    /**
     * Handle the disconnection of a BLE device. 
     */
    void onDisconnection(const Gap::DisconnectionCallbackParams_t *params) {
        // restart advertising if connection is lost
        BLE::Instance().gap().startAdvertising();
    }

    char *getDeviceName() { return deviceName; }

protected:
    BLEManager() {
        deviceName = NULL;
        advertisingInterval = 10;
        advertisingTimeout = 0;
        error = BLE_ERROR_NONE;
        isInitialized = true;
    };

    void _init(BLE::InitializationCompleteCallbackContext *params);

private:
    char *deviceName;
    uint16_t advertisingInterval;
    uint16_t advertisingTimeout;

    bool isInitialized;
    ble_error_t error;
};


#endif //UBIRCH_MBED_BLE_BLEMANAGER_H
