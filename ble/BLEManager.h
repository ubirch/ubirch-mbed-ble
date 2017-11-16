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
#include <BLEConfig.h>

class BLEManager {
public:
    /**
     * Get a singleton of this manager.
     * @return a single instance reference.
     */
    static BLEManager &getInstance();

    /**
     * The actual initialization of the BLE instance.
     *
     * @param deviceName the name of  the BLE device for advertisement
     * @returns BLE_ERROR_NONE if the initialization was successful
     * @returns BLE_ERROR_ALDREADY_INITIALIZED if this instance is configured
     * @returns BLE_ERROR_* for any other BLE related errors
     */
    ble_error_t init(BLEConfig *config);

    /**
     * Initialize the BLE instance and configure services using the config.
     * @param deviceName the device name to use
     * @param advInterval the advertising interval to use
     * @param advTimeout how long to advertise until low power mode
     * @returns BLE_ERROR_NONE if the initialization was successful
     * @returns BLE_ERROR_ALDREADY_INITIALIZED if this instance is configured
     * @returns BLE_ERROR_* for any other BLE related errors
     */
    ble_error_t init(const char *deviceName, uint16_t advInterval = 10, uint16_t advTimeout = 0);

    /**
     * De-initialize the BLE instance, remove services and shutdown BLE.
     * @returns BLE_ERROR_NONE if the shutdown was successful
     * @returns BLE_ERROR_* for any other BLE related errors
     */
    ble_error_t deinit();

    /**
     * Check if this instance has already been initialized.
     * @returns the status of the initialization
     */
    bool isInitialized();

    /**
     * Check if this instance is currently connected.
     * @returns the status of the connectivity
     */
    bool isConnected();


protected:
    BLEManager() {
        config = NULL;
        initialized = false;
        error = BLE_ERROR_NONE;
    };

    ~BLEManager() {
        BLE::Instance().shutdown();
    };

    void _init(BLE::InitializationCompleteCallbackContext *params);

private:
    BLEConfig *config;
    bool initialized;
    ble_error_t error;
};


#endif //UBIRCH_MBED_BLE_BLEMANAGER_H
