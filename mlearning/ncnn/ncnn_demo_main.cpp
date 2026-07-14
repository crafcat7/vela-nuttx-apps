/****************************************************************************
 * apps/mlearning/ncnn/ncnn_demo_main.cpp
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>

#include "mat.h"
#include "net.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 *
 * Description:
 *   Run SqueezeNet with a zero-valued 227x227 BGR image.  The supplied model
 *   must use the standard SqueezeNet input and output names: data and prob.
 *
 ****************************************************************************/

extern "C" int main(int argc, FAR char *argv[])
{
  const char *param_path = CONFIG_NCNN_EXAMPLE_PARAM_PATH;
  const char *bin_path = CONFIG_NCNN_EXAMPLE_BIN_PATH;
  ncnn::Net net;
  ncnn::Mat input(227, 227, 3);
  ncnn::Mat output;
  int ret;
  int best_index = -1;
  float best_score = 0.f;

  if (argc == 3)
    {
      param_path = argv[1];
      bin_path = argv[2];
    }
  else if (argc != 1)
    {
      printf("Usage: ncnn_demo [param_path bin_path]\n");
      return 1;
    }

  net.opt.num_threads = 1;

  printf("ncnn_demo: loading param %s\n", param_path);
  fflush(stdout);
  ret = net.load_param(param_path);
  if (ret != 0)
    {
      printf("ncnn_demo: failed to load %s (%d)\n", param_path, ret);
      return 1;
    }

  printf("ncnn_demo: loading model %s\n", bin_path);
  fflush(stdout);
  ret = net.load_model(bin_path);
  if (ret != 0)
    {
      printf("ncnn_demo: failed to load %s (%d)\n", bin_path, ret);
      return 1;
    }

  printf("ncnn_demo: preparing input\n");
  fflush(stdout);
  input.fill(0.f);
  ncnn::Extractor extractor = net.create_extractor();

  printf("ncnn_demo: binding input\n");
  fflush(stdout);
  ret = extractor.input("data", input);
  if (ret != 0)
    {
      printf("ncnn_demo: input failed (%d)\n", ret);
      return 1;
    }

  printf("ncnn_demo: running inference\n");
  fflush(stdout);
  ret = extractor.extract("prob", output);
  if (ret != 0)
    {
      printf("ncnn_demo: inference failed (%d)\n", ret);
      return 1;
    }

  printf("ncnn_demo: selecting result\n");
  fflush(stdout);

  for (int i = 0; i < output.w; i++)
    {
      if (best_index < 0 || output[i] > best_score)
        {
          best_index = i;
          best_score = output[i];
        }
    }

  printf("ncnn_demo: class=%d score=%f\n", best_index, best_score);
  return 0;
}
