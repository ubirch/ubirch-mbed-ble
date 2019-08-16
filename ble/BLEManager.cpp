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
#include <edebug.h>
#include "BLEManager.h"
#include "mbed.h"


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

bool BLEManager::isInitialized() {
    return initialized;
}

bool BLEManager::isConnected() {
    return BLE::Instance().gap().getState().connected;
}

void BLEManager::on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
    BLE &ble = params->ble;
    this->error = params->error;

    EDEBUG_PRINTF("(BLE::InitializationCompleteCallback\r\n");

    if (this->error) {
        EDEBUG_PRINTF("Error during the initialisation\r\n");
        return;
    }

    if (!this->config || params->ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        this->error = BLE_ERROR_INVALID_STATE;
        EDEBUG_PRINTF("BLE_ERROR_INVALID_STATE\r\n");
        return;
    }

    this->error = this->config->onInit(ble);

    if (error == BLE_ERROR_NONE) {
        EDEBUG_PRINTF("BLE initialized.\r\n");
        this->initialized = true;
    }
}

ble_error_t BLEManager::init(BLEConfig *config) {
    if (initialized) return BLE_ERROR_ALREADY_INITIALIZED;

    this->config = config;

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    EDEBUG_PRINTF("calling BLE init\r\n");
    ble_error_t ble_error = ble.init(this, &BLEManager::on_init_complete);
    if (ble_error) {
        EDEBUG_PRINTF("Error returned by BLE::init");
        return ble_error;
    }

    EDEBUG_PRINTF("waiting for BLE::InitializationCompleteCallback\r\n");
    while (!initialized && error == BLE_ERROR_NONE) /* wait for initialization done or error state */;

    return error;
}

// never used
//ble_error_t BLEManager::init(const char *deviceName, const uint16_t advInterval, const uint16_t advTimeout) {
//    return init(new BLEConfig(deviceName, advInterval, advTimeout));
//}

ble_error_t BLEManager::deinit() {
    if (initialized) {
        initialized = false;
        return BLE::Instance().shutdown();
    }
    return BLE_ERROR_NONE;
}

