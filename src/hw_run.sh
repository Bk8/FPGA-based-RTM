#!/bin/bash

make app-hw && LD_PRELOAD="libmaxeleros.so" ./PassThroughRun
