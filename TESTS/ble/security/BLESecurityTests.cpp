/*!
 * @file
 * @brief Test for the BLE security.
 *
 * These tests require user interaction!
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
#include <SecurityManager.h>
#include <BLEManager.h>
#include "mbed.h"

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "../testhelper.h"

using namespace utest::v1;

static const uint8_t testServiceBaseUUID[UUID::LENGTH_OF_LONG_UUID] = {
        0xc3, 0x0f, 0x00, 0x00, 0x11, 0x6b, 0x4b, 0xf2,
        0x8e, 0xb3, 0xb9, 0x7c, 0xd5, 0x31, 0xa1, 0x13
};
static const uint16_t testServiceShortUUID = 0x7571;
static const uint16_t testServiceCharShortUUID = 0x7572;

const uint8_t testServiceUUID[UUID::LENGTH_OF_LONG_UUID] = {
        0xc3, 0x0f, (uint8_t) (testServiceShortUUID >> 8), (uint8_t) (testServiceShortUUID & 0xFF),
        0x11, 0x6b, 0x4b, 0xf2, 0x8e, 0xb3, 0xb9, 0x7c, 0xd5, 0x31, 0xa1, 0x13
};
const uint8_t testServiceCharUUID[UUID::LENGTH_OF_LONG_UUID] = {
        0xc3, 0x0f, (uint8_t) (testServiceCharShortUUID >> 8), (uint8_t) (testServiceCharShortUUID & 0xFF),
        0x11, 0x6b, 0x4b, 0xf2,
        0x8e, 0xb3, 0xb9, 0x7c, 0xd5, 0x31, 0xa1, 0x13
};

// a very simple service that only notifies a char
class SimpleSecureService {
public:
    unsigned char value;
    unsigned char published;

    explicit SimpleSecureService(BLE &_ble) :
            value('0'), published('?'), ble(_ble),
            secureCharacteristic(testServiceCharUUID, &published, 1, 1,
                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
        // ensure we have encryption and MITM protection
        secureCharacteristic.requireSecurity(SecurityManager::SECURITY_MODE_ENCRYPTION_WITH_MITM);

        GattCharacteristic *charTable[] = {&secureCharacteristic};
        GattService service(testServiceUUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.addService(service);
    }

    void setValue(unsigned char v) {
        value = v;
    }

    void publishValue() {
        published = value;
        ble.gattServer().write(secureCharacteristic.getValueAttribute().getHandle(), &published, 1);
    }

protected:
    BLE &ble;
    GattCharacteristic secureCharacteristic;
};

static SimpleSecureService *service = NULL;
static Gap::Handle_t handle = 0;

void securitySetupCompletedCallback(Gap::Handle_t handle, SecurityManager::SecurityCompletionStatus_t status) {
    greentea_send_kv("secured", status);
}

void linkSecuredCallback(Gap::Handle_t handle, SecurityManager::SecurityMode_t securityMode) {
    (void) handle;

    greentea_send_kv("secured", securityMode);
    service->publishValue();
}

void passkeyDisplay(Gap::Handle_t handle, const SecurityManager::Passkey_t passkey) {
    (void) handle;

    char p[7];
    memcpy(p, passkey, 7);
    p[6] = 0;
    printf("PASSKEY: '%s'\r\n", passkey);
    greentea_send_kv("passkey", p);

}

class BLEConfigSecured : public BLEConfig {
public:
    bool isConnected = true;

    explicit BLEConfigSecured(const char *name) : BLEConfig(name) {}

    ble_error_t onInit(BLE &ble) {
        ble_error_t error;
        SecurityManager::Passkey_t passkey = {'0', '1', '0', '1', '0', '1'};
        error = ble.securityManager().init(true, true, SecurityManager::IO_CAPS_DISPLAY_ONLY, passkey);
        TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, error, "can't init security manager");

        ble.securityManager().onSecuritySetupCompleted(securitySetupCompletedCallback);
        ble.securityManager().onLinkSecured(linkSecuredCallback);
        ble.securityManager().onPasskeyDisplay(passkeyDisplay);

        service = new SimpleSecureService(BLE::Instance());
        uint16_t uuids[2] = {testServiceShortUUID, testServiceCharShortUUID};
        error = ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS,
                                                       (uint8_t *) uuids, sizeof(uuids));
        TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, error, "failed to set uuid list");

        return BLEConfig::onInit(ble);
    }

    void onConnection(const Gap::ConnectionCallbackParams_t *params) {
        handle = params->handle;
        isConnected = true;
    }

    void onDisconnection(const Gap::DisconnectionCallbackParams_t *params) {
        isConnected = false;
        greentea_send_kv("disconnected", "OK");
    }
};

void TestBLEManagerSecurity() {
    char k[48], v[128];

    BLEConfigSecured config = BLEConfigSecured("C0NNECTME");

    BLEManager &bleManager = BLEManager::getInstance();
    printf("security::BLEManager[%p]\r\n", &bleManager);

    TEST_ASSERT_EQUAL_INT_MESSAGE(BLE_ERROR_NONE, bleManager.init(&config), "BLE manager initialization failed");

    // tell host to connect
    greentea_send_kv("connect_secure", config.deviceName);

    greentea_parse_kv(k, v, sizeof(k), sizeof(v));
    TEST_ASSERT_EQUAL_STRING("expect", k);
    service->setValue(static_cast<unsigned char>(v[0]));

    while (config.isConnected) {
        greentea_parse_kv(k, v, sizeof(k), sizeof(v));
        if(!strcmp("finished", k)) break;
        if(!strcmp("received", k)) {
            TEST_ASSERT_EQUAL_INT_MESSAGE(service->value, v[0], "wrong value received");
        } else if(!strcmp("secured", k)) {
            if(v[0] == '0') continue;
            TEST_ASSERT_EQUAL_STRING_MESSAGE("3", v, "link not encrypted with MITM protection");
        }
    }

    // we need to wait until we are fully disconnected or the host test will stall
    while (config.isConnected) Thread::wait(100);

    greentea_parse_kv(k, v, sizeof(k), sizeof(v));
    TEST_ASSERT_EQUAL_STRING("disconnected", k);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("OK", v, "wrong device disconnected");

    delete service;
}

utest::v1::status_t case_teardown_handler(const Case *const source, const size_t passed, const size_t failed,
                                          const failure_t reason) {
    printf("BLEManager::getInstance().deinit()\r\n");
    BLE::Instance().purgeAllBondingState();
    BLEManager::getInstance().deinit();
    return greentea_case_teardown_handler(source, passed, failed, reason);
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(300, "BLEManagerTests");
    return verbose_test_setup_handler(number_of_cases);
}

int main() {
    bleClockInit();

    Case cases[] = {
            Case("Test ble-security", TestBLEManagerSecurity,
                 case_teardown_handler, greentea_failure_handler),
    };

    Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);
    return !Harness::run(specification);
}