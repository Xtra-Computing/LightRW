#!/bin/bash

make app=$1 TARGET=hw_emu all -j
ret=$?

if [ ${ret} -eq 0 ];
  then
    echo "[PASS] make success"
else
    echo ${ret}
    echo "[FAILD] make failed"
    exit -1
fi


make app=$1 hostemu  $2  &


sleep 240

# xdpyinfo | grep name
#env_display=$(xdpyinfo | grep name | awk '{print $4}')
#export DISPLAY=${env_display}

export DISPLAY=:10


window_id=$(xwininfo -root -tree  | grep Sourcing\ Tcl | awk '{print $1}')
echo ${window_id}
xdotool windowactivate ${window_id}
xdotool windowfocus ${window_id}
xdotool key Alt+b

window_id=$(xwininfo -root -tree  | grep Vivado\ 2020 | awk '{print $1}')
echo ${window_id}
xdotool windowactivate ${window_id}
xdotool windowfocus ${window_id}
xdotool key "F5"
