# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

PROJECT_DIR := ../../..
SRC_DIR := $(PROJECT_DIR)/src

LOCAL_MODULE    := ngrtl


# Environment variables
# ---------------------

# Common
LOCAL_CFLAGS :=                 \
 	-DANDROID               \
	-D__ANDROID__           \
	-DANDROID_NDK           \
	-DOS_ANDROID            \
	-D__NEW__               \
	-pipe			\
	-D_FILE_OFFSET_BITS=64	\
	-DNG_REQUIRE_IS_EXCEPTION \

# default is -fno-rtti flag  
LOCAL_CPPFLAGS +=		\
	-frtti

# using mutex
LOCAL_CPPFLAGS +=		\
	-std=gnu++0x

# Source files
# ------------
LOCAL_SRC_FILES := \
	$(SRC_DIR)/require.cpp \
	$(SRC_DIR)/math.cpp \
	$(SRC_DIR)/mutex.cpp \
	$(SRC_DIR)/mainstub.cpp \
	$(SRC_DIR)/format.cpp \
	$(SRC_DIR)/stream/bytestream.cpp \
	$(SRC_DIR)/stream/filestreambuf.cpp \
	$(SRC_DIR)/stream/memstreambuf.cpp \
	$(SRC_DIR)/stream/textstream.cpp \
	$(SRC_DIR)/log/log.cpp \
	$(SRC_DIR)/json/deserialize.cpp \
	$(SRC_DIR)/json/json.cpp \
	$(SRC_DIR)/json/JSON_parser.c \
	$(SRC_DIR)/json/serialize.cpp \
	$(SRC_DIR)/geo/geoboundingbox.cpp \
	$(SRC_DIR)/geo/lonlat.cpp \
	$(SRC_DIR)/geo/quadid.cpp \
	$(SRC_DIR)/geo/quadbitmap.cpp \
	$(SRC_DIR)/progress.cpp \
	$(SRC_DIR)/compress/vectorcodec.cpp \
	$(SRC_DIR)/kml/simplekmldump.cpp \
	$(SRC_DIR)/sysutil.cpp \
	$(SRC_DIR)/morton.cpp \

	
# Include paths
# -------------

LOCAL_C_INCLUDES :=	\
	$(PROJECT_DIR)/..	\
	$(PROJECT_DIR)/../../include/core
		
LOCAL_LDLIBS := \
	-ldl	\
	-llog	\
	-lz

include $(BUILD_STATIC_LIBRARY)
