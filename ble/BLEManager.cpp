/*!
 * @file
 * @brief BLEManager - handle BLE initialization and services.
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
#include <rtos.h>
#include <UARTService.h>
#include "BLEManager.h"

#define ASSERT(c, m) if(!(c)) {printf("assert(" m ")"); return;}

static Thread bleEventThread(osPriorityNormal);
static EventQueue bleEventQueue(4 * EVENTS_EVENT_SIZE);

// BLE events processing
static void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext *context) {
    bleEventQueue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}


BLEManager &BLEManager::getInstance() {
    static BLEManager *instance;
    if (!instance) {
        bleEventThread.start(callback(&bleEventQueue, &EventQueue::dispatch_forever));
        instance = new BLEManager();
    }
    return *instance;
}

void BLEManager::_init(BLE::InitializationCompleteCallbackContext *params) {
    BLE &ble = params->ble;
    error = params->error;

    if (params->ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        error = BLE_ERROR_INVALID_STATE;
    }
    if (error != BLE_ERROR_NONE) return;

    ble.gap().onConnection(config, &BLEConfig::onConnection);
    ble.gap().onDisconnection(config, &BLEConfig::onDisconnection);
    error = ble.gap().setAddress(BLEProtocol::AddressType::RANDOM_PRIVATE_RESOLVABLE, {0});
    ASSERT(error == BLE_ERROR_NONE, "address type");

    error = ble.gap().setDeviceName((uint8_t *) config->deviceName);
    ASSERT(error == BLE_ERROR_NONE, "device name");

    error = ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED |
                                                   GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ASSERT(error == BLE_ERROR_NONE, "adv payload");

    error = ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                                   (uint8_t *) config->deviceName,
                                                   static_cast<uint8_t>(strlen(config->deviceName)));
    ASSERT(error == BLE_ERROR_NONE, "local name");
    
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(config->advertisingInterval);
    ble.gap().setAdvertisingTimeout(config->advertisingTimeout);
    error = ble.gap().startAdvertising();
    ASSERT(error == BLE_ERROR_NONE, "start adv");
    
    isInitialized = true;
}

ble_error_t BLEManager::init(BLEConfig *config) {
    if(isInitialized) BLE_ERROR_INVALID_STATE;

    this->config = config;

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(this, &BLEManager::_init);

    while (!isInitialized && error == BLE_ERROR_NONE) /* wait for initialization done or error state */;

    return error;
}

ble_error_t BLEManager::init(const char *deviceName, const uint16_t advInterval, const uint16_t advTimeout) {
    if(!isInitialized) return init(new BLEConfig(deviceName, advInterval, advTimeout));
    return BLE_ERROR_INVALID_STATE;
}

ble_error_t BLEManager::deinit() {
    if(isInitialized) {
        isInitialized = false;
        return BLE::Instance().shutdown();
    }
    return BLE_ERROR_NONE;
}



