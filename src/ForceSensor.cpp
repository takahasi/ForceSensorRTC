// -*- C++ -*-
/*!
 * @file  ForceSensor.cpp
 * @brief Force sensor component for wacoh dynpick
 * @date $Date$
 *
 * $Id$
 */

#include "ForceSensor.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <time.h>

#include <rtm/SystemLogger.h>

#define BAUD_RATE    B921600

// Module specification
// <rtc-template block="module_spec">
static const char* forcesensor_spec[] =
  {
    "implementation_id", "ForceSensor",
    "type_name",         "ForceSensor",
    "description",       "Force sensor component for wacoh dynpick",
    "version",           "1.0.0",
    "vendor",            "takahasi",
    "category",          "Sensor",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.port_name", "/dev/ttyUSB0",
    "conf.default.points_moving_average", "8",

    // Widget
    "conf.__widget__.port_name", "text",
    "conf.__widget__.points_moving_average", "text",
    // Constraints

    "conf.__type__.port_name", "string",
    "conf.__type__.points_moving_average", "short",

    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
ForceSensor::ForceSensor(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_outOut("out", m_out)

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
ForceSensor::~ForceSensor()
{
}



RTC::ReturnCode_t ForceSensor::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  
  // Set OutPort buffer
  addOutPort("out", m_outOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("port_name", m_port_name, "/dev/ttyUSB0");
  bindParameter("points_moving_average", m_points_moving_average, "8");
  // </rtc-template>

  m_fd = -1;
  m_out.data.length(6);
  (void)memset(sense, 0, sizeof(sense));

  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t ForceSensor::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ForceSensor::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ForceSensor::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t ForceSensor::onActivated(RTC::UniqueId ec_id)
{
    RTC_TRACE(("Enter onActivated()"));

    ssize_t len = 0;
    char str[256] = {0};

    if (m_fd == -1) {
        m_fd = open(m_port_name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (m_fd < 0) {
            RTC_ERROR(("failed to open"));
            return RTC::RTC_ERROR;
        }
    }

    struct termios tio;
    (void)memset(&tio, 0, sizeof(tio));
    tio.c_cflag = (BAUD_RATE | CS8 | CLOCAL | CREAD);
    tio.c_cc[VTIME] = 0;
    cfsetispeed(&tio, BAUD_RATE);
    cfsetospeed(&tio, BAUD_RATE);
    tcsetattr(m_fd, TCSANOW, &tio);

    /* checks version information */
    len = read(m_fd, str, sizeof(str)); /* dummy read */
    (void)memset(str, 0x00, sizeof(str));
    len = write(m_fd, "V", 1);

    fd_set fds;
    struct timeval tv;

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(m_fd, &fds);
    if (select((m_fd + 1), &fds, NULL, NULL, &tv) < 0) {
        /* select errors */
        RTC_ERROR(("select fd error!"));
        return RTC::RTC_ERROR;
    }
    if (!FD_ISSET(m_fd, &fds)) {
        /* select timeout or caught unknown signals */
        RTC_WARN(("select fd timeout!"));
    }

    len = read(m_fd, str, sizeof(str));
    RTC_INFO(("Version information:"));
    RTC_INFO_STR((str));

    /* checks sensor specific parmeters */
    int retry = 0;
    while (retry++ < 3) {
        (void)memset(str, 0x00, sizeof(str));
        len = write(m_fd, "p", 1);

        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(m_fd, &fds);
        if (select((m_fd + 1), &fds, NULL, NULL, &tv) < 0) {
            /* select errors */
            RTC_ERROR(("select fd error!"));
            return RTC::RTC_ERROR;
        }
        if (!FD_ISSET(m_fd, &fds)) {
            /* select timeout or caught unknown signals */
            continue;
        }
        len = read(m_fd, str, sizeof(str));
        sscanf(str, "%f,%f,%f,%f,%f,%f",
                &sense[0], &sense[1], &sense[2],
                &sense[3], &sense[4], &sense[5]);

        if ((sense[0] == 0) || (sense[1] == 0) ||
            (sense[2] == 0) || (sense[3] == 0) ||
            (sense[4] == 0) || (sense[5] == 0)) {
            /* illegal value */
            continue;
        }

        RTC_INFO(("Sensing paramters: %f,%f,%f,%f,%f,%f",
                sense[0], sense[1], sense[2],
                sense[3], sense[4], sense[5]));
        break;
    }
    if (retry > 3) {
        /* retry out */
        RTC_ERROR(("retry out error!"));
        return RTC::RTC_ERROR;
    }

    /* set low pass filter */
    char average_command[] = "1F";
    switch (m_points_moving_average) {
        case '2':
            average_command[0] = '2';
            break;
        case '4':
            average_command[0] = '4';
            break;
        case '8':
            average_command[0] = '8';
            break;
        case '1':
            /* fall through */
        default:
            break;
    }
    /* #F:#-points average */
    if (2 != write(m_fd, average_command, 2)) {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }


    /* set offset as initial values */
    for (int i = 0; i < 3; i++) {
        if (1 != write(m_fd, "O", 1)) {
            RTC_ERROR(("failed to write"));
            return RTC::RTC_ERROR;
        }
    }
    if (1 != write(m_fd, "R", 1)) {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }
    len = read(m_fd, str, sizeof(str));
    if (len < 0) {
        RTC_WARN(("failed to read"));
    }

    RTC_TRACE(("Exit onActivated()"));

    return RTC::RTC_OK;
}


RTC::ReturnCode_t ForceSensor::onDeactivated(RTC::UniqueId ec_id)
{
    RTC_TRACE(("Enter onDeactivated()"));

    if (m_fd == -1) {
        RTC_INFO(("already closed"));
        return RTC::RTC_OK;
    }

    if (0 != close(m_fd)) {
        RTC_ERROR(("failed to close"));
        return RTC::RTC_ERROR;
    }

    m_fd = -1;

    RTC_TRACE(("Exit onDeactivated()"));

    return RTC::RTC_OK;
}


RTC::ReturnCode_t ForceSensor::onExecute(RTC::UniqueId ec_id)
{
    ssize_t len = 0;
    char str[27] = {0};
    int tick = 0;

    RTC_TRACE(("Enter onExecute()"));

    /* dummy read */
    len = read(m_fd, str, sizeof(str));
    (void)memset(str, 0x00, sizeof(str));

    /* request next data */
    if (1 != write(m_fd, "R", 1)) {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }

    fd_set fds;
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    FD_ZERO(&fds);
    FD_SET(m_fd, &fds);
    if (select((m_fd + 1), &fds, NULL, NULL, &tv) < 0) {
        /* select errors */
        RTC_ERROR(("select fd error!"));
        return RTC::RTC_ERROR;
    }
    if (!FD_ISSET(m_fd, &fds)) {
        /* select timeout or caught unknown signals */
        return RTC::RTC_OK;
    }

    /* read values */
    len = read(m_fd, str, sizeof(str));
    if (len < 0) {
        RTC_ERROR(("failed to read"));
        return RTC::RTC_ERROR;
    }

    unsigned short data[6] = {0};

    sscanf(str, "%1d%4hx%4hx%4hx%4hx%4hx%4hx",
            &tick, &data[0], &data[1], &data[2],
            &data[3], &data[4], &data[5]);

    for (int i = 0; i < 6; i++) {
        m_out.data[i] = (float)((data[i] - 8192) / sense[i]);
    }

    m_outOut.write();

    RTC_INFO(("data: [%d] %f,%f,%f,%f,%f,%f",
                tick, m_out.data[0], m_out.data[1], m_out.data[2],
                m_out.data[3], m_out.data[4], m_out.data[5]));

    RTC_TRACE(("Exit onExecute()"));

    return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t ForceSensor::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ForceSensor::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ForceSensor::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ForceSensor::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ForceSensor::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void ForceSensorInit(RTC::Manager* manager)
  {
    coil::Properties profile(forcesensor_spec);
    manager->registerFactory(profile,
                             RTC::Create<ForceSensor>,
                             RTC::Delete<ForceSensor>);
  }
  
};


