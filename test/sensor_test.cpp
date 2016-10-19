/*!
 * @file   sensor_test.cpp
 * @brief  test program for force sensor
 * @author takahashi
 * @sa
 * @date   2016/09/09 New
 * @version    1.0
 */
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include "sensor_test.h"

using namespace std;

void sensor_processor::process(void)
{
    unsigned int count = 0;

    cout << "init()" << endl;
    if (0 != init()) {
        return;
    }

    cout << "setup()" << endl;
    if (0 != setup()) {
        return;
    }

    cout << "start processing..." << endl;
    //while (1) {
    for (int i = 0; i < (SAMPLING_RATE_HZ * 20); i++) {
        _sensor_data s;

        memset(&s, 0x00, sizeof(s));

        /* get sensor data */
        // cout << "get_data()" << endl;
        (void)get_data(&s);

        /* print results if needed */
        if (++count > PRINT_PERIOD) {
            // cout << "show_data()" << endl;
            (void)show_data(&s);
            count = 0;
        }

        /* waits for next sampling */
        usleep((1000 * 1000) / SAMPLING_RATE_HZ);
    }

    cout << "finalize()" << endl;
    if (0 != finalize()) {
        return;
    }
}

int main(int argc, char *argv[])
{
    sensor_processor *sp = new SENSOR_DEVICE_CLASS_NAME();

    sp->process();

    delete sp;
}
