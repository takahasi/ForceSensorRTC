Readme
============

* RTComponent: ForceSensor specificatioin
* OpenRTM-aist-1.0.0
* Date: $Date$

  This file is generated by rtc-template with the following argments.

`
   rtc-template -bcxx --module-name=ForceSensor 
   --module-desc='Force sensor component for wacoh dynpick' --module-version=1.0.0 --module-vendor=takahasi 
   --module-category=Sensor --module-comp-type=STATIC 
   --module-act-type=PERIODIC --module-max-inst=1 
   --config=port_name:string:/dev/ttyUSB0 --config=points_moving_average:short:8 
   --outport=out:RTC::TimedFloatSeq 
`

Build Status
============

#### TraviCI [![Build Status](https://travis-ci.org/takahasi/ForceSensorRTC.svg?branch=master)](https://travis-ci.org/takahasi/ForceSensorRTC)

#### Appveyor [![Build status](https://ci.appveyor.com/api/projects/status/t5xxur9vckvanjbn?svg=true)](https://ci.appveyor.com/project/takahasi/forcesensorrtc)


Basic Information
======================================================================

* Module Name: ForceSensor
* Description: Force sensor component for wacoh dynpick
* Version:     1.0.0
* Vendor:      takahasi
* Category:    Sensor
* Kind:        DataFlowComponent
* Comp. Type:  STATIC
* Act. Type:   PERIODIC
* MAX Inst.:   1
* Lang:        C++
* Lang Type:   


Activity definition
======================================================================

[on_initialize]    implemented

[on_finalize]

[on_startup]

[on_shutdown]

[on_activated]     implemented

[on_deactivated]   implemented

[on_execute]       implemented

[on_aborting]

[on_error]

[on_reset]

[on_state_update]

[on_rate_changed]

InPorts definition
======================================================================


OutPorts definition
======================================================================

	Name:        out
	PortNumber:  0
	Description: 
	PortType: 
	DataType:    RTC::TimedFloatSeq
	MaxOut: 
	[Data Elements]
		Name:
		Type:            
		Number:          
		Semantics:       
		Unit:            
		Frequency:       
		Operation Cycle: 
		RangeLow:
		RangeHigh:
		DefaultValue:


Service Port definition
======================================================================


Configuration definition
======================================================================

	Configuration:
		Name:             port_name
		Description:     
		Type:            string
		DefaultValue:     /dev/ttyUSB0
		Unit:            
		Range:           
		Constraint:      

		Name:             points_moving_average
		Description:     
		Type:            short
		DefaultValue:     8
		Unit:            
		Range:           
		Constraint:      


License
=======
This software is developed at the National Institute of Advanced
Industrial Science and Technology. Approval number H23PRO-????. This
software is licensed under the Lesser General Public License. See
COPYING.LESSER.

Note
====
This area is reserved for future OpenRTM.
