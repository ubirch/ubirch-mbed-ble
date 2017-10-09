#ifndef __TESTHELPER_H__
#define __TESTHELPER_H__


inline void bleClockInit() {
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
}

#endif