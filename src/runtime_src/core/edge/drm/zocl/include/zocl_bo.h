/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * Copyright (C) 2016-2022 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Author(s):
 *        Min Ma <min.ma@xilinx.com>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#ifndef _ZOCL_BO_H_
#define _ZOCL_BO_H_

#include "zocl_drv.h"

struct zocl_drm_bo_ops {
        /**
         * @init: Initialize Memory management here. This will be done once while
	 * first XCLBIN is downloading.
         */
        void (*init)(struct zocl_drm_dev *zdev, void *bo_handlr);

	/**
         * @release: Release existing memory management. This will be done if we
	 * need to reinitialize the cuurent memory manager or driver is unloading.
         */
	void (*release)(struct zocl_drm_dev *zdev, void *bo_handlr);
};

int zocl_bo_memory_register(struct zocl_drm_dev *zdev, void *bo_handlr);
int zocl_bo_memory_unregister(void *bo_handlr);

#endif
