# BLE Service Wrapper

A wrapper around the BLE implementation to make implementing services simpler.

```bash
mbed add https://github.com/ubirch/ubirch-mbed-ble
```

## Testing

> The host tests require a host BLE adapter to receive data and discover devices.

### Prerequisites

- [Python BLE Wrapper](https://github.com/brettchien/PyBLEWrapper)
    ```
    pip install -U git+https://github.com/brettchien/PyBLEWrapper.git
    ```

```bash
mbed new .
mbed target NRF52_DK
mbed toolchain GCC_ARM
mbed test -n tests-ble* -vv
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


