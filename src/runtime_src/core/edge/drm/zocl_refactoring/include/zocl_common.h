/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Authors:
 *	ch.vamshi.krishna@amd.com
 *	saifuddin.kaijar@amd.com
 *
 */

#ifndef _ZOCL_COMMON_H
#define _ZOCL_COMMON_H

#include <linux/platform_device.h>
#include "zocl_drv.h"
#include "zocl_bo.h"
#include "zocl_xclbin.h"
#include "zocl_platfrm_cu.h"

#define zDEV_HANDLR(zdev_handlr)			\
	((struct zocl_dev *)(zdev_handlr))
#define aDRV_MEMHNDLR(zdev_handlr)			\
	((void *)(zDEV_HANDLR(zdev_handlr))->zocl_mm)
#define aDRV_XCLBINHNDLR(zdev_handlr)			\
	((void *)(zDEV_HANDLR(zdev_handlr))->zocl_xclbin)

#define zDEV_LOCK(zdev_handlr)			\
	mutex_lock(&((struct zocl_dev *)(zdev_handlr))->dev_lock)
#define zDEV_UNLOCK(zdev_handlr)			\
	mutex_unlock(&((struct zocl_dev *)(zdev_handlr))->dev_lock)

#define ZDEV2PDEV(zdev)					\
	((struct platform_device *)zdev->pdev)
#define aDRV_DRMHNDLR(zdrv_handlr)			\
	((struct drm_device *)(zDEV_HANDLR(zdrv_handlr)->ddev))
#define aDRM_DRVHNDLR(zdrm_handlr)			\
	((struct zocl_dev *)(zdrm_handlr)->dev_private)
#define aDRM_MEMHNDLR(zdrm_handlr)			\
	((void *)(aDRV_MEMHNDLR(xDRM_DRVHNDLR(zdrm_handlr))))

extern struct platform_driver zocl_platfrm_cu_driver;
inline struct zocl_drm_dev *zocl_get_zdev(void);

#endif /* endif of _ZOCL_COMMON_H */
