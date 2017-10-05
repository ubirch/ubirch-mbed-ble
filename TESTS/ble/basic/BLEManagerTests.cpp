/*!
 * @file
 * @brief TODO: ${FILE}
 *
 * ...
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
#include "mbed.h"

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

using namespace utest::v1;

void TestBLEManagerSingleton() {
    BLEManager &i1 = BLEManager::getInstance();
    BLEManager &i2 = BLEManager::getInstance();
    printf("singleton::BLEManager[%p] == BLEManager[%p]\r\n", &i1, &i2);

    TEST_ASSERT_EQUAL_INT32_MESSAGE(&i1, &i2, "singleton instances are not the same");
}

void TestBLEManagerInit() {
    BLEManager &bleManager = BLEManager::getInstance();
    printf("init::BLEManager[%p]\r\n", &bleManager);

    TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, bleManager.init(), "BLE manager initialization failed");
    TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, bleManager.deinit(), "BLE deinit failed");
}

void TestBLEManagerAdvertising() {
    char k[48], v[128];
    BLEConfig config("0123456789ABCDEF");

    BLEManager &bleManager = BLEManager::getInstance();
    printf("advertising::BLEManager[%p]\r\n", &bleManager);

    TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, bleManager.init(&config), "BLE manager initialization failed");

    greentea_send_kv("discover", config.deviceName);
    greentea_parse_kv(k, v, sizeof(k), sizeof(v));
    TEST_ASSERT_EQUAL_STRING_MESSAGE(config.deviceName, v, "BLE device discovery failed");
    TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, bleManager.deinit(), "BLE deinit failed");
}


void TestBLEManagerOnConnection() {
    char k[48], v[128];

    class BLEConfigOnConnection : public BLEConfig {
    public:
        explicit BLEConfigOnConnection(const char *name) : BLEConfig(name) {};

        void onConnection(const Gap::ConnectionCallbackParams_t *params) {
            greentea_send_kv("connected", "OK");
        }
    };
    BLEConfigOnConnection config = BLEConfigOnConnection("C0NNECTME");

    BLEManager &bleManager = BLEManager::getInstance();
    printf("onConnection::BLEManager[%p]\r\n", &bleManager);

    TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, bleManager.init(&config), "BLE manager initialization failed");

    greentea_send_kv("connect", config.deviceName);
    greentea_parse_kv(k, v, sizeof(k), sizeof(v));

    TEST_ASSERT_EQUAL_STRING_MESSAGE("connected", k, "wrong key received");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("OK", v, "wrong device connected");
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

Case cases[] = {
Case("Test stack-ble-singleton", TestBLEManagerSingleton, greentea_failure_handler),
Case("Test stack-ble-init", TestBLEManagerInit, greentea_failure_handler),
Case("Test stack-ble-advertise", TestBLEManagerAdvertising, greentea_failure_handler),
Case("Test stack-ble-on-connection", TestBLEManagerOnConnection, greentea_failure_handler),
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(60, "BLEManagerTests");
    return verbose_test_setup_handler(number_of_cases);
}

void greentea_test_teardown(const size_t passed, const size_t failed, const failure_t failure)
{
    printf("BLEManager::shutdown()");
    BLEManager::getInstance().deinit();
    greentea_test_teardown_handler(passed, failed, failure);
}

int main() {

    // initialize external clock for our tests
    DigitalOut externalClockPin(P0_27, 0);

    externalClockPin = 0;
    wait_ms(100);
    externalClockPin = 1;
    wait_ms(100);

    /* Mark the HF clock as not started */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    /* Try to start the HF clock */
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    //Make sure HFCLK is on
    //TODO use a while loop and reset module if it fails to init HFCLK
    for (int i = 0; i < 5; i++) {
        if (NRF_CLOCK->EVENTS_HFCLKSTARTED) {
            break;
        }
        wait(1);
    }

    Specification specification(greentea_test_setup, cases, greentea_test_teardown);
    return !Harness::run(specification);
}