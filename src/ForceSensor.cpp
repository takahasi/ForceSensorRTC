// -*- C++ -*-
/*!
 * @file  ForceSensor.cpp
 * @brief Force sensor component for wacoh dynpick
 * @date $Date$
 *
 * $Id$
 */

#include "ForceSensor.h"

#include <rtm/SystemLogger.h>

/* additional includes */
#ifdef WIN32
#else /* WIN32 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#endif /* WIN32 */

/* additional definitions */
#ifdef WIN32
#define BAUD_RATE    921600
#else /* WIN32 */
#define BAUD_RATE    B921600
#endif /* WIN32 */

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
#ifdef WIN32
	"conf.default.port_name", "\\\\.\\COM4",
#else /* WIN32 */
    "conf.default.port_name", "/dev/ttyUSB0",
#endif /* WIN32 */
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
#ifdef WIN32
  bindParameter("port_name", m_port_name, "\\\\.\\COM4");
#else /* WIN32 */
  bindParameter("port_name", m_port_name, "/dev/ttyUSB0");
#endif /* WIN32 */
  bindParameter("points_moving_average", m_points_moving_average, "8");
  // </rtc-template>

#ifdef WIN32
  m_hComm = INVALID_HANDLE_VALUE;
#else /* WIN32 */
  m_fd = -1;
#endif /* WIN32 */
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

    char str[256] = { 0 };

	/* Open COM port */
#ifdef WIN32
    m_hComm = CreateFile((LPCTSTR)m_port_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (m_hComm == INVALID_HANDLE_VALUE) {
        RTC_ERROR(("failed to open"));
        return RTC::RTC_ERROR;
    }

    DCB config;
    config.DCBlength = sizeof(DCB);
    config.BaudRate = BAUD_RATE;
    config.ByteSize = 8;
    config.Parity = NOPARITY;
    config.StopBits = ONESTOPBIT;
    config.fBinary = TRUE;
    config.fOutxCtsFlow = FALSE;
    config.fOutxDsrFlow = FALSE;
    config.fDtrControl = DTR_CONTROL_DISABLE;
    config.fRtsControl = RTS_CONTROL_DISABLE;
    config.fOutX = FALSE;
    config.fInX = FALSE;
    config.fTXContinueOnXoff = TRUE;
    config.XonLim = 512;
    config.XoffLim = 512;
    config.XonChar = 0x11;
    config.XoffChar = 0x13;
    config.fNull = TRUE;
    config.fAbortOnError = TRUE;
    config.fErrorChar = FALSE;
    config.ErrorChar = 0x00;
    config.EofChar = 0x03;
    config.EvtChar = 0x02;

    BOOL ret = SetCommState(m_hComm, &config);
    if (FALSE == ret) {
        RTC_ERROR(("failed to SetCommState()"));
        return RTC::RTC_ERROR;
    }

    COMMTIMEOUTS timeout;

    timeout.ReadIntervalTimeout = 100;
    timeout.ReadTotalTimeoutMultiplier = 0;
    timeout.ReadTotalTimeoutConstant = 100;
    timeout.WriteTotalTimeoutMultiplier = 0;
    timeout.WriteTotalTimeoutConstant = 100;

    ret = SetCommTimeouts(m_hComm, &timeout);
    if (FALSE == ret)
    {
        RTC_ERROR(("failed to SetCommTimeoouts()"));
        return RTC::RTC_ERROR;
    }

    DWORD errors;
    COMSTAT comStat;
    ClearCommError(m_hComm, &errors, &comStat);

#else /* WIN32 */

    if (-1 == m_fd) {
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

#endif /* WIN32 */


    /* checks version information */
#ifdef WIN32
    DWORD len;
    ReadFile(m_hComm, str, sizeof(str), &len, NULL); /* dummy read */
    (void)memset(str, 0x00, sizeof(str));
    WriteFile(m_hComm, "V", 1, &len, NULL);
    ReadFile(m_hComm, str, sizeof(str), &len, NULL);
#else /* WIN32 */
    ssize_t len = read(m_fd, str, sizeof(str)); /* dummy read */
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

#endif /* WIN32 */

    RTC_INFO(("Version information:"));
    RTC_INFO_STR((str));


    /* checks sensor specific parmeters */

	int retry = 0;
    while (retry++ < 3) {
        (void)memset(str, 0x00, sizeof(str));

#ifdef WIN32
        WriteFile(m_hComm, "p", 1, &len, NULL);
        ReadFile(m_hComm, str, sizeof(str), &len, NULL);
#else /* WIN32 */
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
#endif /* WIN32 */
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
    /* retry out */
    if (retry > 3) {
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
#ifdef WIN32
    if (FALSE == WriteFile(m_hComm, average_command, 2, &len, NULL))
#else /* WIN32 */
    if (2 != write(m_fd, average_command, 2))
#endif /* WIN32 */
    {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }


    /* set offset as initial values */
    for (int i = 0; i < 3; i++) {
#ifdef WIN32
        if (FALSE == WriteFile(m_hComm, "O", 1, &len, NULL))
#else /* WIN32 */
        if (1 != write(m_fd, "O", 1))
#endif /* WIN32 */
        {
            RTC_ERROR(("failed to write"));
            return RTC::RTC_ERROR;
        }
    }
#ifdef WIN32
    if (FALSE == WriteFile(m_hComm, "R", 1, &len, NULL))
#else /* WIN32 */
    if (1 != write(m_fd, "R", 1))
#endif /* WIN32 */
    {
        RTC_ERROR(("failed to write"));
         return RTC::RTC_ERROR;
    }
#ifdef WIN32
    if (FALSE == ReadFile(m_hComm, str, sizeof(str), &len, NULL))
#else /* WIN32 */
    if (0 > read(m_fd, str, sizeof(str)))
#endif /* WIN32 */
    {
        RTC_WARN(("failed to read"));
    }

    RTC_TRACE(("Exit onActivated()"));

    return RTC::RTC_OK;
}


RTC::ReturnCode_t ForceSensor::onDeactivated(RTC::UniqueId ec_id)
{
    RTC_TRACE(("Enter onDeactivated()"));

#ifdef WIN32
    if (m_hComm == INVALID_HANDLE_VALUE)
#else /* WIN32 */
    if (m_fd == -1) {
#endif /* WIN32 */
    {
        RTC_INFO(("already closed"));
        return RTC::RTC_OK;
    }

#ifdef WIN32
    CloseHandle(m_hComm);
    m_hComm = INVALID_HANDLE_VALUE;
#else /* WIN32 */
    if (0 != close(m_fd)) {
        RTC_ERROR(("failed to close"));
        return RTC::RTC_ERROR;
    }

    m_fd = -1;
#endif /* WIN32 */

    RTC_TRACE(("Exit onDeactivated()"));

    return RTC::RTC_OK;
}


RTC::ReturnCode_t ForceSensor::onExecute(RTC::UniqueId ec_id)
{
	RTC_TRACE(("Enter onExecute()"));

	char str[27] = { 0 };
	int tick = 0;

    /* dummy read */
#ifdef WIN32
    DWORD len;
    ReadFile(m_hComm, str, sizeof(str), &len, NULL);

    if (FALSE == ReadFile(m_hComm, str, sizeof(str), &len, NULL))
#else /* WIN32 */
    ssize_t len = read(m_fd, str, sizeof(str));
#endif /* WIN32 */

    (void)memset(str, 0x00, sizeof(str));

    /* request next data */
#ifdef WIN32
    if (FALSE == WriteFile(m_hComm, "R", 1, &len, NULL))
#else /* WIN32 */
    if (1 != write(m_fd, "R", 1))
#endif /* WIN32 */
    {
        RTC_ERROR(("failed to write"));
        return RTC::RTC_ERROR;
    }

#ifdef WIN32
#else /* WIN32 */
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
#endif /* WIN32 */

    /* read values */
#ifdef WIN32
    if (FALSE == ReadFile(m_hComm, str, sizeof(str), &len, NULL))
#else /* WIN32 */
    if (0 > read(m_fd, str, sizeof(str)))
#endif /* WIN32 */
    {
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


