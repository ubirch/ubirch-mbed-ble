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

#include "BLEConfig.h"

BLEConfig::BLEConfig(const char *deviceName, uint16_t advertisingInterval, uint16_t advertisingTimeout) {
    this->deviceName = deviceName;
    this->advertisingInterval = advertisingInterval;
    this->advertisingTimeout = advertisingTimeout;
}

ble_error_t BLEConfig::onInit(BLE &ble) {
    ble_error_t error;

    ble.gap().setEventHandler(this);        //TODO this should be in DeviceBLEConfig ?

//    error = ble.gap().setAddress(BLEProtocol::AddressType::RANDOM_PRIVATE_RESOLVABLE, {0});
//    BLE_ASSERT(error, "address type");

    error = ble.gap().setDeviceName((uint8_t *) this->deviceName);
    BLE_ASSERT(error, "device name");

    error = ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED |
                                                   GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    BLE_ASSERT(error, "adv payload");

    error = ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                                   (uint8_t *) this->deviceName,
    static_cast<uint8_t>(strlen(this->deviceName)));
    BLE_ASSERT(error, "local name");

    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(this->advertisingInterval);
    ble.gap().setAdvertisingTimeout(this->advertisingTimeout);

    return ble.gap().startAdvertising();
}

void BLEConfig::onConnectionComplete(const ble::ConnectionCompleteEvent &event) {}

void BLEConfig::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) {
    // restart advertising if connection is lost
    BLE::Instance().gap().startAdvertising();
}

