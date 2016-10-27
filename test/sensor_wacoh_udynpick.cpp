/*!
 * @file   sensor_wacoh_udynpick.cpp
 * @brief  test program for force sensor
 * @author takahashi
 * @sa
 * @date   2016/09/09 New
 * @version    1.0
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include "sensor_test.h"

/* DEFINITIONS */
#define DEV_NAME    "/dev/ttyUSB0"
#define BAUD_RATE    B921600
#define BUFF_SIZE    (4096)

/* CONFIGURATIONS */
//#define USE_CUSTOM_FILTER (1)
#define ERROR(str) printf("[ERR] %s:%d:%s\n", __FUNCTION__, __LINE__, str)

/* PROTOTYPES */
static void filter(_sensor_data *now);

static void filter(_sensor_data *now)
{
#ifdef USE_CUSTOM_FILTER
    static _sensor_data prev = {0};

    for (int i = 0; i < AXIS_NUM; i++) {
        now->data[i] = ((prev.data[i] * 0.8) + (now->data[i] * 0.2));
    }
    memcpy(&prev, now, sizeof(prev));
#endif /* USE_CUSTOM_FILTER */
}

int sensor_processor_wacoh_udynpick::init(void)
{
    fd = open(DEV_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        ERROR("failed to open");
        return -1;
    }

    struct termios tio;
    memset(&tio, 0, sizeof(tio));
    tio.c_cflag = (BAUD_RATE | CS8 | CLOCAL | CREAD);
    tio.c_cc[VTIME] = 0;
    cfsetispeed(&tio, BAUD_RATE);
    cfsetospeed(&tio, BAUD_RATE);
    tcsetattr(fd,TCSANOW,&tio);

    return 0;
}

int sensor_processor_wacoh_udynpick::setup(void)
{
    /* waits for stabilizing */
    sleep(5);

    /* set offset as initial values */
    if (1 != write(fd, "O", 1)) {
        ERROR("failed to write");
        return -1;
    }
    if (1 != write(fd, "O", 1)) {
        ERROR("failed to write");
        return -1;
    }
    if (1 != write(fd, "O", 1)) {
        ERROR("failed to write");
        return -1;
    }
    sleep(1);

    /* set low pass filter */
    if (2 != write(fd, "1F", 2)) { /* 8F:8-points average */
        ERROR("failed to write");
        return -1;
    }
    sleep(1);

    /* dummy read to set initial offset */
    char str[100];
    int len = 0;

    if (1 != write(fd, "R", 1)) {
        ERROR("failed to write");
        return -1;
    }
    (void)read(fd, &str, 27);

    sleep(1);

    /* checks version information */
    memset(str, 0x00, sizeof(str));
    (void)write(fd, "V", 1);
    len = read(fd, &str, 100);
    printf("---\n");
    for (int i = 0; i < len; i++) {
        printf("%02x ", str[i]);
    }
    printf("---\n");

    /* checks sensor specific parmeters */
    memset(str, 0x00, sizeof(str));
    (void)write(fd, "p", 1);
    len = read(fd, &str, 100);
    printf("---\n");
    for (int i = 0; i < len; i++) {
        printf("%02x ", str[i]);
    }
    printf("---\n");

    return 0;
}

int sensor_processor_wacoh_udynpick::finalize(void)
{
    if (0 != close(fd)) {
        ERROR("failed to close");
        return -1;
    }

    return 0;
}

int sensor_processor_wacoh_udynpick::get_data(_sensor_data *s)
{
    char str[27];

    /* read values */
    if (1 != write(fd, "R", 1)) {
        ERROR("failed to write");
        return -1;
    }

    int len = read(fd, &str, sizeof(str));
    if (len < 0) {
        ERROR("failed to read");
        return -1;
    }
    sscanf(str, "%1d%4hx%4hx%4hx%4hx%4hx%4hx",
            &s->tick,
            &s->data[0],
            &s->data[1],
            &s->data[2],
            &s->data[3],
            &s->data[4],
            &s->data[5]);
    filter(s);

    return 0;
}

int sensor_processor_wacoh_udynpick::show_data(_sensor_data *s)
{
    printf("[%d] ", s->tick);
    for (int i = 0; i < AXIS_NUM; i++) {
        printf("%d,", s->data[i] - 8192);
    }
    printf("\n");

    return 0;
}
