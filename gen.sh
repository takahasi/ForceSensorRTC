#!/bin/sh

rtc-template -bcxx \
    --module-name=ForceSensor --module-type='DataFlowComponent' \
    --module-desc='Force sensor component for WACOH DynPick  ' \
    --module-version=1.0 --module-vendor='Takahashi' \
    --module-category=sensor \
    --module-comp-type=DataFlowComponent --module-act-type=SPORADIC \
    --module-max-inst=10 --outport=out:TimedFloatSeq \
    --config=points_moving_average:char:0 \
    --config=max_scale_Fx:float:1 \
    --config=max_scale_Fy:float:1 \
    --config=max_scale_Fz:float:1 \
    --config=max_scale_Mx:float:1 \
    --config=max_scale_My:float:1 \
    --config=max_scale_Mz:float:1
