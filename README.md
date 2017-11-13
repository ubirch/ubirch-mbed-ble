# BLE Service Wrapper

A wrapper around the BLE implementation to make implementing services simpler.

```bash
mbed add https://github.com/ubirch/ubirch-mbed-ble
```

## Testing

> The host tests require a host BLE adapter to receive data and discover devices.

## Prerequisites

### Mac-OS

- [Python BLE Wrapper](https://github.com/brettchien/PyBLEWrapper)
    ```
    pip install -U git+https://github.com/brettchien/PyBLEWrapper.git
    ```

### Linux

- [bluepy](https://github.com/IanHarvey/bluepy)
- [BLESuite](https://github.com/nccgroup/BLESuite)


## Installing ubirch-ble-tool 

run ```pip install -U git+https://github.com/ubirch/ubirch-ble-tool.git```

## Running tests
```bash
mbed new .
mbed target NRF52_DK
mbed toolchain GCC_ARM
mbed test -n tests-ble* -vv
```

### Results

Basic Tests
```
+------------------+---------------+--------------------+--------+--------------------+-------------+
| target           | platform_name | test suite         | result | elapsed_time (sec) | copy_method |
+------------------+---------------+--------------------+--------+--------------------+-------------+
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-ble-basic    | OK     | 27.27              | default     |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-ble-services | OK     | 31.59              | default     |
+------------------+---------------+--------------------+--------+--------------------+-------------+
```

UART Service
```
+------------------+---------------+--------------------+------------------------+--------+--------+--------+--------------------+
| target           | platform_name | test suite         | test case              | passed | failed | result | elapsed_time (sec) |
+------------------+---------------+--------------------+------------------------+--------+--------+--------+--------------------+
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-ble-basic    | Test ble-advertise     | 1      | 0      | OK     | 0.53               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-ble-basic    | Test ble-init          | 1      | 0      | OK     | 0.1                |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-ble-basic    | Test ble-on-callbacks  | 1      | 0      | OK     | 0.57               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-ble-basic    | Test ble-singleton     | 1      | 0      | OK     | 0.11               |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-ble-services | Test ble-uart-discover | 1      | 0      | OK     | 0.8                |
| NRF52_DK-GCC_ARM | NRF52_DK      | tests-ble-services | Test ble-uart-send     | 1      | 0      | OK     | 4.63               |
+------------------+---------------+--------------------+------------------------+--------+--------+--------+--------------------+

```
# License

This library is available under the [Apache License](LICENSE)

```
Copyright 2017 ubirch GmbH

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
````


