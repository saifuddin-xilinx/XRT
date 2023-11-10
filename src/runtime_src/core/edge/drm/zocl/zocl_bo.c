/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * A GEM style (optionally CMA backed) device manager for ZynQ based
 * OpenCL accelerators.
 *
 * Copyright (C) 2016-2022 Xilinx, Inc. All rights reserved.
 *
 * Authors:
 *    Sonal Santan <sonal.santan@xilinx.com>
 *    Umang Parekh <umang.parekh@xilinx.com>
 *    Jan Stephan  <j.stephan@hzdr.de>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#include <linux/pagemap.h>
#include <linux/of_address.h>
#include <linux/iommu.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0))
        #include <linux/iosys-map.h>
        #define ZOCL_MAP_TYPE iosys_map
        #define ZOCL_MAP_IS_NULL iosys_map_is_null
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0))
        #include <linux/dma-buf-map.h>
        #define ZOCL_MAP_TYPE dma_buf_map
        #define ZOCL_MAP_IS_NULL dma_buf_map_is_null
#endif

