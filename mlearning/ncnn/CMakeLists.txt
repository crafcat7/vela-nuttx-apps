# ##############################################################################
# apps/mlearning/ncnn/CMakeLists.txt
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

if(CONFIG_NCNN)

  include(FetchContent)

  # Keep the third-party source separate from the NuttX build integration.  The
  # same revision is used by the CMake FetchContent path and the legacy Make
  # path in Makefile.

  set(NCNN_DIR ${CMAKE_CURRENT_LIST_DIR}/ncnn)
  set(NCNN_VERSION 13b6d5318c73be53bb386fa51e8067615d0eb7c1)

  if(NOT EXISTS ${NCNN_DIR}/src/CMakeLists.txt)
    FetchContent_Declare(
      ncnn_fetch
      URL https://github.com/Tencent/ncnn/archive/${NCNN_VERSION}.zip
      SOURCE_DIR ${NCNN_DIR}
      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/ncnn
      DOWNLOAD_NO_PROGRESS true
      TIMEOUT 30)

    FetchContent_GetProperties(ncnn_fetch)

    if(NOT ncnn_fetch_POPULATED)
      FetchContent_Populate(ncnn_fetch)
    endif()
  endif()

  file(READ ${NCNN_DIR}/CMakeLists.txt NCNN_TOPLEVEL_CMAKE)
  string(FIND "${NCNN_TOPLEVEL_CMAKE}" "NCNN_NUTTX" NCNN_NUTTX_PATCHED)

  if(NCNN_NUTTX_PATCHED EQUAL -1)
    execute_process(
      COMMAND patch -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ncnn-nuttx.patch
      WORKING_DIRECTORY ${NCNN_DIR}
      RESULT_VARIABLE NCNN_PATCH_RESULT)

    if(NOT NCNN_PATCH_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to apply the NCNN openvela patch")
    endif()
  endif()

  file(READ ${NCNN_DIR}/benchmark/benchncnn.cpp NCNN_BENCHMARK_SOURCE)
  string(FIND "${NCNN_BENCHMARK_SOURCE}" "NCNN_NUTTX_BENCHMARK"
              NCNN_BENCHMARK_PATCHED)

  if(NCNN_BENCHMARK_PATCHED EQUAL -1)
    execute_process(
      COMMAND patch -p1 -i ${CMAKE_CURRENT_LIST_DIR}/ncnn-nuttx-benchmark.patch
      WORKING_DIRECTORY ${NCNN_DIR}
      RESULT_VARIABLE NCNN_BENCHMARK_PATCH_RESULT)

    if(NOT NCNN_BENCHMARK_PATCH_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to apply the NCNN benchmark openvela patch")
    endif()
  endif()

  # openvela uses the CPU backend.  Disable GPU support and host-oriented
  # components explicitly so upstream defaults cannot enable them.

  set(NCNN_NUTTX ON)
  set(NCNN_SHARED_LIB OFF CACHE BOOL "" FORCE)
  set(NCNN_OPENMP OFF CACHE BOOL "" FORCE)
  set(NCNN_THREADS OFF CACHE BOOL "" FORCE)
  set(NCNN_VULKAN OFF CACHE BOOL "" FORCE)
  set(NCNN_SIMPLEVK OFF CACHE BOOL "" FORCE)
  set(NCNN_RUNTIME_CPU ON CACHE BOOL "" FORCE)
  set(NCNN_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
  set(NCNN_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(NCNN_BUILD_BENCHMARK \${CONFIG_NCNN_BENCHMARK} CACHE BOOL "" FORCE)
  set(NCNN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(NCNN_INSTALL_SDK OFF CACHE BOOL "" FORCE)
  set(NCNN_PYTHON OFF CACHE BOOL "" FORCE)
  set(NCNN_DISABLE_RTTI ON CACHE BOOL "" FORCE)
  set(NCNN_DISABLE_EXCEPTION ON CACHE BOOL "" FORCE)

  add_subdirectory(${NCNN_DIR} ${CMAKE_CURRENT_BINARY_DIR}/ncnn)
  target_compile_options(ncnn PRIVATE -Wno-unknown-pragmas
                                      -Wno-unused-parameter)
  nuttx_add_external_library(ncnn)

  if(CONFIG_NCNN_EXAMPLE)
    nuttx_add_application(
      NAME
      ncnn_demo
      PRIORITY
      ${CONFIG_NCNN_EXAMPLE_PRIORITY}
      STACKSIZE
      ${CONFIG_NCNN_STACKSIZE}
      MODULE
      ${CONFIG_NCNN_EXAMPLE}
      SRCS
      ncnn_demo_main.cpp)
    target_link_libraries(apps_ncnn_demo PRIVATE ncnn)
  endif()

  if(CONFIG_NCNN_BENCHMARK)
    nuttx_add_application(
      NAME
      ncnn_benchmark
      PRIORITY
      ${CONFIG_NCNN_BENCHMARK_PRIORITY}
      STACKSIZE
      ${CONFIG_NCNN_STACKSIZE}
      MODULE
      ${CONFIG_NCNN_BENCHMARK}
      SRCS
      ${NCNN_DIR}/benchmark/benchncnn.cpp)
    target_link_libraries(apps_ncnn_benchmark PRIVATE ncnn)
    target_include_directories(apps_ncnn_benchmark PRIVATE
                               \${CMAKE_CURRENT_BINARY_DIR}/ncnn/benchmark)
    target_compile_definitions(apps_ncnn_benchmark PRIVATE
                               NCNN_NUTTX_BENCHMARK=1)
    add_dependencies(apps_ncnn_benchmark ncnn-generate-param)
  endif()

endif()
