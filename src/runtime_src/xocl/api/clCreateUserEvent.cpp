/**
 * Copyright (C) 2016-2020 Xilinx, Inc
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

// Copyright 2017-2020 Xilinx, Inc. All rights reserved.

#include "xocl/config.h"
#include "xocl/core/event.h"
#include "detail/context.h"
#include "plugin/xdp/profile_v2.h"
#include <CL/opencl.h>

namespace xocl {

static void
validOrError(cl_context context,
             cl_int*    errcode_ret)
{
  if (!config::api_checks())
    return;

  // CL_INVALID_CONTEXT if context is not a valid context.
  detail::context::validOrError(context);

  // CL_OUT_OF_RESOURCES if there is a failure to allocate resources
  // required by the OpenCL implementation on the device.

  // CL_OUT_OF_HOST_MEMORY if there is a failure to allocate resources
  // required by the OpenCL implementation on the host.
}

static cl_event
clCreateUserEvent(cl_context context,
                  cl_int*    errcode_ret)
{
  validOrError(context,errcode_ret);

  // Soft event
  auto uevent = xocl::create_soft_event(context,CL_COMMAND_USER);

  uevent->queue();

  xocl::assign(errcode_ret,CL_SUCCESS);
  return uevent.release();
}

} // xocl

cl_event
clCreateUserEvent(cl_context     context ,
                  cl_int *       errcode_ret )
{
  try {
    PROFILE_LOG_FUNCTION_CALL;
    LOP_LOG_FUNCTION_CALL;
    return xocl::clCreateUserEvent(context,errcode_ret);
  }
  catch (const xrt_xocl::error& ex) {
    xocl::send_exception_message(ex.what());
    xocl::assign(errcode_ret,ex.get_code());
  }
  catch (const std::exception& ex) {
    xocl::send_exception_message(ex.what());
    xocl::assign(errcode_ret,CL_OUT_OF_HOST_MEMORY);
  }
  return nullptr;
}
