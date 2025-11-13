#!/bin/sh

pwd
cd ../jni
echo NDK_HOME: $NDK_HOME

export PATH=$NDK_HOME:$NDK_ARM_LIB_DIR:$PATH
./$1 && echo DONE.
