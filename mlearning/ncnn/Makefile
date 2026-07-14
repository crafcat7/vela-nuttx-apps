############################################################################
# apps/mlearning/ncnn/Makefile
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

include $(APPDIR)/Make.defs

NCNN_VERSION := 13b6d5318c73be53bb386fa51e8067615d0eb7c1
NCNN_DIR := ncnn
NCNN_BUILD_DIR := $(NCNN_DIR)/build-nuttx
NCNN_LIBRARY := $(NCNN_BUILD_DIR)/src/libncnn$(LIBEXT)
NCNN_PATCH_STAMP := $(NCNN_DIR)/.nuttx-patched
NCNN_BENCHMARK_PATCH_STAMP := $(NCNN_DIR)/.nuttx-benchmark-patched
NCNN_PROCESSOR := $(subst ",,$(CONFIG_ARCH))
NCNN_FETCH :=
NCNN_CMAKE_BUILD_BENCHMARK := OFF
NCNN_BENCHMARK_PARAM_DATA :=
NCNN_CFLAGS := $(subst ",\",$(CFLAGS))
NCNN_CXXFLAGS := $(subst ",\",$(CXXFLAGS))

ifneq ($(CONFIG_NCNN_BENCHMARK),)
NCNN_CMAKE_BUILD_BENCHMARK := ON
NCNN_BENCHMARK_PARAM_DATA := $(NCNN_BUILD_DIR)/benchmark/benchncnn_param_data.h
endif

# NCNN's standalone CMake build consumes the NuttX libc++ headers directly.
# Ensure their context links exist before it starts, otherwise a parallel
# top-level build can compile NCNN before <algorithm> is available.
ifeq ($(CONFIG_LIBCXX),y)
NCNN_CXX_CONTEXT := $(TOPDIR)/include/libcxx

$(NCNN_CXX_CONTEXT):
	$(Q) $(MAKE) -C $(TOPDIR)/libs/libxx context
endif

ifeq ($(NCNN_PROCESSOR),arm64)
NCNN_PROCESSOR := aarch64
endif

NCNN_CMAKE_ARGS := \
	-DCMAKE_SYSTEM_NAME=Generic \
	-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
	-DCMAKE_SYSTEM_PROCESSOR=$(NCNN_PROCESSOR) \
	-DCMAKE_C_COMPILER=$(CC) \
	-DCMAKE_CXX_COMPILER=$(CXX) \
	-DCMAKE_C_FLAGS="$(NCNN_CFLAGS)" \
	-DCMAKE_CXX_FLAGS="$(NCNN_CXXFLAGS) -Wno-unknown-pragmas -Wno-unused-parameter" \
	-DNCNN_SHARED_LIB=OFF \
	-DNCNN_OPENMP=OFF \
	-DNCNN_THREADS=OFF \
	-DNCNN_VULKAN=OFF \
	-DNCNN_SIMPLEVK=OFF \
	-DNCNN_RUNTIME_CPU=ON \
	-DNCNN_BUILD_TOOLS=OFF \
	-DNCNN_BUILD_EXAMPLES=OFF \
	-DNCNN_BUILD_BENCHMARK=$(NCNN_CMAKE_BUILD_BENCHMARK) \
	-DNCNN_BUILD_TESTS=OFF \
	-DNCNN_INSTALL_SDK=OFF \
	-DNCNN_PYTHON=OFF \
	-DNCNN_DISABLE_RTTI=ON \
	-DNCNN_DISABLE_EXCEPTION=ON

ncnn.zip:
	$(Q) curl -L https://github.com/Tencent/ncnn/archive/$(NCNN_VERSION).zip -o ncnn.zip
	$(Q) unzip -o ncnn.zip
	$(Q) mv ncnn-$(NCNN_VERSION) $(NCNN_DIR)

# Download and unpack the pinned tarball when the third-party checkout is
# absent.  Keeping the wrapper directory in the tree lets a clean source tree
# build without a manual NCNN clone.
ifeq ($(wildcard $(NCNN_DIR)/src/CMakeLists.txt),)
NCNN_FETCH := ncnn.zip
context:: ncnn.zip

distclean::
	$(call DELDIR, $(NCNN_DIR))
	$(call DELFILE, ncnn.zip)
endif

$(NCNN_PATCH_STAMP): $(NCNN_FETCH) ncnn-nuttx.patch
	$(Q) grep -q NCNN_NUTTX $(NCNN_DIR)/CMakeLists.txt || \
		patch -d $(NCNN_DIR) -p1 < ncnn-nuttx.patch
	$(Q) touch $(NCNN_PATCH_STAMP)

$(NCNN_BENCHMARK_PATCH_STAMP): $(NCNN_PATCH_STAMP) \
		ncnn-nuttx-benchmark.patch
	$(Q) grep -q '__has_include("benchncnn_param_data.h")' \
		$(NCNN_DIR)/benchmark/benchncnn.cpp && \
		grep -q '^extern "C"$$' $(NCNN_DIR)/benchmark/benchncnn.cpp || \
		patch -d $(NCNN_DIR) -p1 < ncnn-nuttx-benchmark.patch
	$(Q) touch $(NCNN_BENCHMARK_PATCH_STAMP)

$(NCNN_LIBRARY): $(NCNN_PATCH_STAMP) $(NCNN_BENCHMARK_PATCH_STAMP) \
	$(NCNN_CXX_CONTEXT)
	$(Q) cmake -S $(NCNN_DIR) -B $(NCNN_BUILD_DIR) $(NCNN_CMAKE_ARGS)
	$(Q) cmake --build $(NCNN_BUILD_DIR) --target ncnn

$(NCNN_BENCHMARK_PARAM_DATA): $(NCNN_LIBRARY)
	$(Q) cmake -S $(NCNN_DIR) -B $(NCNN_BUILD_DIR) $(NCNN_CMAKE_ARGS)
	$(Q) cmake --build $(NCNN_BUILD_DIR) --target ncnn-generate-param

context:: $(NCNN_LIBRARY) $(NCNN_BENCHMARK_PARAM_DATA)

CXXEXT = .cpp

ifneq ($(CONFIG_NCNN_EXAMPLE),)
MAINSRC = ncnn_demo_main.cpp
PROGNAME = ncnn_demo
PRIORITY = $(CONFIG_NCNN_EXAMPLE_PRIORITY)
STACKSIZE = $(CONFIG_NCNN_STACKSIZE)
MODULE = $(CONFIG_NCNN_EXAMPLE)
endif

ifneq ($(CONFIG_NCNN_BENCHMARK),)
MAINSRC += ncnn/benchmark/benchncnn.cpp
PROGNAME += ncnn_benchmark
PRIORITY += $(CONFIG_NCNN_BENCHMARK_PRIORITY)
STACKSIZE += $(CONFIG_NCNN_STACKSIZE)
MODULE += $(CONFIG_NCNN_BENCHMARK)
CXXFLAGS += -DNCNN_NUTTX_BENCHMARK=1
CXXFLAGS += -I$(NCNN_BUILD_DIR)/benchmark
endif

include $(APPDIR)/Application.mk

ifneq ($(CONFIG_NCNN_BENCHMARK),)
$(OBJS): $(NCNN_BENCHMARK_PARAM_DATA)
endif
