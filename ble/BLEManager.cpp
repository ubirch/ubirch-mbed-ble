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


static Thread *bleEventThread;
static EventQueue *bleEventQueue;

// BLE events processing
static void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext *context) {
    if (bleEventQueue) bleEventQueue->call(Callback<void()>(&context->ble, &BLE::processEvents));
}

BLEManager &BLEManager::getInstance() {
    static BLEManager *instance;
    if (!instance) {
        bleEventQueue = new EventQueue(4 * EVENTS_EVENT_SIZE);
        bleEventThread = new Thread(osPriorityNormal);
        bleEventThread->start(callback(bleEventQueue, &EventQueue::dispatch_forever));
        instance = new BLEManager();
    }
    return *instance;
}

void BLEManager::_init(BLE::InitializationCompleteCallbackContext *params) {
    BLE &ble = params->ble;
    this->error = params->error;

    if (!this->config || params->ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        this->error = BLE_ERROR_INVALID_STATE;
        return;
    }

    this->error = this->config->onInit(ble);
    this->isInitialized = (error == BLE_ERROR_NONE);
}

ble_error_t BLEManager::init(BLEConfig *config) {
    if (isInitialized) return BLE_ERROR_ALREADY_INITIALIZED;

    this->config = config;

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(this, &BLEManager::_init);

    while (!isInitialized && error == BLE_ERROR_NONE) /* wait for initialization done or error state */;

    return error;
}

ble_error_t BLEManager::init(const char *deviceName, const uint16_t advInterval, const uint16_t advTimeout) {
    return init(new BLEConfig(deviceName, advInterval, advTimeout));
}

ble_error_t BLEManager::deinit() {
    if (isInitialized) {
        isInitialized = false;
        return BLE::Instance().shutdown();
    }
    return BLE_ERROR_NONE;
}

