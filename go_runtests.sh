#! /bin/sh
TESTS='tests-ble*'
mbed new .
mbed target NRF52_DK
mbed toolchain GCC_ARM
mbed test --compile -n "$TESTS"
mbedgt -n "$TESTS" --plain --report-junit=testresult.xml --report-memory-metrics-csv=testmem.csv