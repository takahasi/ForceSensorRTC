#!/bin/sh

rtc-template -bcxx \
    --module-name=ForceSensor --module-type='DataFlowComponent' \
    --module-desc='Force sensor component for WACOH DynPick  ' \
    --module-version=1.0 --module-vendor='Takahashi' \
    --module-category=sensor \
    --module-comp-type=DataFlowComponent --module-act-type=SPORADIC \
    --module-max-inst=10 --outport=out:TimedDoubleSeq \
    --config=max_scale_Fx:double:1 \
    --config=max_scale_Fy:double:1 \
    --config=max_scale_Fz:double:1 \
    --config=max_scale_Mx:double:1 \
    --config=max_scale_My:double:1 \
    --config=max_scale_Mz:double:1
