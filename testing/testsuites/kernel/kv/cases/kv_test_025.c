/****************************************************************************
 * apps/testing/testsuites/kernel/kv/cases/kv_test_025.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 *The ASF licenses this file to you under the Apache License, Version 2.0
 *(the "License"); you may not use this file except in compliance with
 *the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 *WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 *implied.  See the License for the specific language governing
 *permissions and limitations under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>
#include <stdlib.h>
#include <inttypes.h>
#include <syslog.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <time.h>
#include <cmocka.h>
#include <time.h>
#include "KvTest.h"
#include "kvdb.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: test_nuttx_kv25
 ****************************************************************************/

void test_nuttx_kv25(FAR void **state)
{
  int ret;
  char key[50] =
  {
    0
  };

  char data[50] =
  {
    0
  };

  char value[50] =
  {
    0
  };

  for (int i = 1; i <= 300; i++)
    {
      /* test key */

      sprintf(key, "persist.kv25_%d", i);

      /* test data */

      sprintf(value, "test_value_kv25_%d_", i);

      ret = property_set_buffer(key, value, 50);
      assert_int_equal(ret, 0);
      ret = property_commit();
      if (ret != 0)
        {
          syslog(
              LOG_ERR,
              "Failed to commit after inserting data. Key:%s Value:%s",
              key, value);
          assert_int_equal(ret, 0);
        }

      ret = property_get_buffer(key, data, 50);
      assert_int_equal(ret, 50);
      assert_int_equal(strcmp(value, data), 0);

      ret = property_delete(key);
      assert_int_equal(ret, 0);
      ret = property_commit();
      if (ret != 0)
        {
          syslog(LOG_ERR,
                 "Failed to commit after deleting data. Key:%s Value:%s",
                 key, value);
          assert_int_equal(ret, 0);
        }
    }
}
