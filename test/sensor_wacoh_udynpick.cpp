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

static float sense[6] = {0, 0, 0, 0, 0, 0};

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
    char str[100] = {0};

    /* waits for stabilizing */
    sleep(5);

    /* checks version information */
    (void)read(fd, &str, sizeof(str)); /* dummy read */
    memset(str, 0x00, sizeof(str));
    (void)write(fd, "V", 1);

    fd_set fds;
    struct timeval tv;

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    if (select((fd + 1), &fds, NULL, NULL, &tv) < 0) {
        /* select errors */
        printf("select fd error!\n");
        return -1;
    }
    if (!FD_ISSET(fd, &fds)) {
        /* select timeout or caught unknown signals */
    }

    (void)read(fd, &str, sizeof(str));
    printf("\nVersion information:\n%s", str);

    /* checks sensor specific parmeters */
    while (1) {
        memset(str, 0x00, sizeof(str));
        (void)write(fd, "p", 1);

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        if (select((fd + 1), &fds, NULL, NULL, &tv) < 0) {
            /* select errors */
            printf("select fd error!\n");
            return -1;
        }
        if (!FD_ISSET(fd, &fds)) {
            /* select timeout or caught unknown signals */
            continue;
        }
        (void)read(fd, &str, sizeof(str));
        sscanf(str, "%f,%f,%f,%f,%f,%f",
                &sense[0],
                &sense[1],
                &sense[2],
                &sense[3],
                &sense[4],
                &sense[5]);
        if ((sense[0] == 0) || (sense[1] == 0) || (sense[2] == 0)
            || (sense[3] == 0) || (sense[4] == 0) || (sense[5] == 0)) {
            continue;
        }
        printf("\nSensing paramters:\n");
        printf("%4.3f, %4.3f, %4.3f, %4.3f, %4.3f, %4.3f\n",
                sense[0],
                sense[1],
                sense[2],
                sense[3],
                sense[4],
                sense[5]);
        break;
    }

    /* set low pass filter */
    if (2 != write(fd, "8F", 2)) { /* 8F:8-points average */
        ERROR("failed to write");
        return -1;
    }

    /* checks filter paramter */
    (void)read(fd, &str, sizeof(str)); /* dummy read */
    if (2 != write(fd, "0F", 2)) {
        ERROR("failed to write");
        return -1;
    }

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    if (select((fd + 1), &fds, NULL, NULL, &tv) < 0) {
        /* select errors */
        printf("select fd error!\n");
        return -1;
    }
    if (!FD_ISSET(fd, &fds)) {
        /* select timeout or caught unknown signals */
    }

    memset(str, 0x00, sizeof(str));
    (void)read(fd, &str, sizeof(str));
    printf("\nCurrent filter:\n%s", str);

    /* set offset as initial values */
    for (int i = 0; i < 3; i++) {
        if (1 != write(fd, "O", 1)) {
            ERROR("failed to write");
            return -1;
        }
    }

    /* dummy read to set initial offset */
    if (1 != write(fd, "R", 1)) {
        ERROR("failed to write");
        return -1;
    }
    (void)read(fd, &str, sizeof(str)); /* dummy read */

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

    /* dummy read */
    (void)read(fd, &str, sizeof(str));

    /* read values 1 time */
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
    printf("[%d]\n", s->tick);
    printf(" - Fx : %f [N]\n", (s->data[0] - 8192) / sense[0]);
    printf(" - Fy : %f [N]\n", (s->data[1] - 8192) / sense[1]);
    printf(" - Fz : %f [N]\n", (s->data[2] - 8192) / sense[2]);
    printf(" - Mx : %f [Nm]\n", (s->data[3] - 8192) / sense[3]);
    printf(" - My : %f [Nm]\n", (s->data[4] - 8192) / sense[4]);
    printf(" - Mz : %f [Nm]\n", (s->data[5] - 8192) / sense[5]);

    return 0;
}
