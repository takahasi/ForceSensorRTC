// -*- C++ -*-
/*!
 * @file  ForceSensor.cpp * @brief Force sensor component for WACOH DynPick   * $Date$ 
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

#define DEV_NAME    "/dev/ttyUSB0"
#define BAUD_RATE    B921600
#define BUFF_SIZE    (4096)


// Module specification
// <rtc-template block="module_spec">
static const char* forcesensor_spec[] =
  {
    "implementation_id", "ForceSensor",
    "type_name",         "ForceSensor",
    "description",       "Force sensor component for WACOH DynPick  ",
    "version",           "1.0",
    "vendor",            "Takahashi",
    "category",          "sensor",
    "activity_type",     "SPORADIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.points_moving_average", "0",
    "conf.default.max_scale_Fx", "1",
    "conf.default.max_scale_Fy", "1",
    "conf.default.max_scale_Fz", "1",
    "conf.default.max_scale_Mx", "1",
    "conf.default.max_scale_My", "1",
    "conf.default.max_scale_Mz", "1",
    ""
  };
// </rtc-template>

ForceSensor::ForceSensor(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_outOut("out", m_out)

    // </rtc-template>
{
}

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
  bindParameter("points_moving_average", m_points_moving_average, "0");
  bindParameter("max_scale_Fx", m_max_scale_Fx, "1");
  bindParameter("max_scale_Fy", m_max_scale_Fy, "1");
  bindParameter("max_scale_Fz", m_max_scale_Fz, "1");
  bindParameter("max_scale_Mx", m_max_scale_Mx, "1");
  bindParameter("max_scale_My", m_max_scale_My, "1");
  bindParameter("max_scale_Mz", m_max_scale_Mz, "1");

  // </rtc-template>

  m_fd = -1;
  m_out.data.length(6);

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

    if (m_fd == -1) {
        m_fd = open(DEV_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (m_fd < 0) {
            RTC_ERROR(("failed to open"));
            return RTC::RTC_ERROR;
        }
    }

    struct termios tio;
    memset(&tio, 0, sizeof(tio));
    tio.c_cflag = (BAUD_RATE | CS8 | CLOCAL | CREAD);
    tio.c_cc[VTIME] = 0;
    cfsetispeed(&tio, BAUD_RATE);
    cfsetospeed(&tio, BAUD_RATE);
    tcsetattr(m_fd, TCSANOW, &tio);

    /* waits for stabilizing */
    //sleep(5);

    /* set offset as initial values */
    if (1 != write(m_fd, "O", 1)) {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }
    if (1 != write(m_fd, "O", 1)) {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }
    if (1 != write(m_fd, "O", 1)) {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }
    sleep(1);

    /* set low pass filter */
    if (2 != write(m_fd, "1F", 2)) { /* 8F:8-points average */
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }
    sleep(1);

    /* dummy read to set initial offset */
    if (1 != write(m_fd, "R", 1)) {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
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
    char str[27] = {0};
    int tick = 0;
    unsigned short data[6] = {0};

    RTC_TRACE(("Enter onExecute()"));

    /* read values */
    if (1 != write(m_fd, "R", 1)) {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }

    int len = read(m_fd, &str, sizeof(str));
    if (len < 0) {
        RTC_ERROR(("failed to read"));
        return RTC::RTC_ERROR;
    }

    sscanf(str, "%1d%4hx%4hx%4hx%4hx%4hx%4hx",
            &tick, &data[0], &data[1], &data[2],
            &data[3], &data[4], &data[5]);

    m_out.data[0] = (float)((data[0] - 8192) / 32.130); // Fx
    m_out.data[1] = (float)((data[1] - 8192) / 32.375); // Fy
    m_out.data[2] = (float)((data[2] - 8192) / 32.520); // Fz
    m_out.data[3] = (float)((data[3] - 8192) / 1641.750); // Mx
    m_out.data[4] = (float)((data[4] - 8192) / 1623.750); // My
    m_out.data[5] = (float)((data[5] - 8192) / 1642.500); // Mz

    m_outOut.write();

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



