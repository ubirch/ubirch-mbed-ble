/*!
 * @file
 * @brief Test for the BLE UART Service
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

#include <sdk_common.h>
#include <BLEManager.h>
#include <services/BLEUartService.h>

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "../testhelper.h"

using namespace utest::v1;

#define DEVICE_NAME "C0NNECTME"

class BLEConfigOnConnection : public BLEConfig {

public:
    bool isDisconnected;

    explicit BLEConfigOnConnection(const char *name = DEVICE_NAME) : BLEConfig(name) {
        isDisconnected = true;
    }

    void onConnection(const Gap::ConnectionCallbackParams_t *params) override {
        isDisconnected = false;
    }

    void onDisconnection(const Gap::DisconnectionCallbackParams_t *params) {
        isDisconnected = true;
    }
};

void TestBLEUartServiceDiscoverCharacteristics() {
    char k[48], v[128];

    BLEManager &bleManager = BLEManager::getInstance();
    BLEConfigOnConnection config = BLEConfigOnConnection();

    TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, bleManager.init(&config), "BLE manager init failed");
    BLEUartService *uartService = new BLEUartService(BLE::Instance());

    greentea_send_kv("discover", DEVICE_NAME);

    // check if we receive the correct service UUID (UART TX/ UART RX)
    greentea_parse_kv(k, v, sizeof(k), sizeof(v));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", v, "TX UUID does not match");
    greentea_parse_kv(k, v, sizeof(k), sizeof(v));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", v, "RX UUID does not match");

    // we need to wait until we are fully disconnected or the host test will stall
    while (!config.isDisconnected) Thread::wait(100);
    printf("DISCONNECTED\r\n");

    delete uartService;
}


void TestBLEUartServiceReceiveData() {
    char k[48], v[128], expected[128];

    BLEManager &bleManager = BLEManager::getInstance();
    BLEConfigOnConnection config = BLEConfigOnConnection();

    TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, bleManager.init(&config), "BLE manager init failed");
    BLEUartService *uartService = new BLEUartService(BLE::Instance(), 128, 128);

    // tell the host test to connect and wait for the message to be expected
    greentea_send_kv("writedata", DEVICE_NAME);
    greentea_parse_kv(k, expected, sizeof(k), sizeof(v));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("expect", k, "wrong response key received");

    // wait for data to come in
    while (!uartService->isReadable()) /* wait */;

    // now read all the data and send it back
    int i = 0;
    while (uartService->isReadable()) v[i++] = static_cast<char>(uartService->getc());
    v[i] = '\0';

    // check that the message we expected has been received
    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, v, "wrong message received");

    // we need to wait until we are fully disconnected or the host test will stall
    while (!config.isDisconnected) Thread::wait(100);

    delete uartService;
}

utest::v1::status_t case_teardown_handler(const Case *const source, const size_t passed, const size_t failed,
                                          const failure_t reason) {
    printf("BLEManager::getInstance().deinit()");
    BLEManager::getInstance().deinit();
    return greentea_case_teardown_handler(source, passed, failed, reason);
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    return greentea_case_failure_abort_handler(source, reason);
}

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(20, "BLEUartServiceTests");
    return verbose_test_setup_handler(number_of_cases);
}

int main() {
    bleClockInit();

    Case cases[] = {
    Case("Test ble-uart-discover", TestBLEUartServiceDiscoverCharacteristics,
         case_teardown_handler, greentea_failure_handler),
    Case("Test ble-uart-send", TestBLEUartServiceReceiveData,
         case_teardown_handler, greentea_failure_handler),
    };

    Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);
    return !Harness::run(specification);
}