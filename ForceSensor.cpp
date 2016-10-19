// -*- C++ -*-
/*!
 * @file  ForceSensor.cpp * @brief Force sensor component for WACOH DynPick   * $Date$ 
 *
 * $Id$ 
 */
#include "ForceSensor.h"

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
  bindParameter("max_scale_Fx", m_max_scale_Fx, "1");
  bindParameter("max_scale_Fy", m_max_scale_Fy, "1");
  bindParameter("max_scale_Fz", m_max_scale_Fz, "1");
  bindParameter("max_scale_Mx", m_max_scale_Mx, "1");
  bindParameter("max_scale_My", m_max_scale_My, "1");
  bindParameter("max_scale_Mz", m_max_scale_Mz, "1");

  // </rtc-template>
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
/*
RTC::ReturnCode_t ForceSensor::onActivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/
/*
RTC::ReturnCode_t ForceSensor::onDeactivated(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/
/*
RTC::ReturnCode_t ForceSensor::onExecute(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/
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



