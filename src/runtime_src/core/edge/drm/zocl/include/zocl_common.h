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

#ifndef _ZOCL_COMMON_H_
#define _ZOCL_COMMON_H_

#include <linux/platform_device.h>
#include <asm/io.h>
#include "zocl_drv.h"

#define ZDEV2PDEV(zdev)			((zdev)->pdev)

#define zocl_err(dev, fmt, args...)     dev_err(dev, "%s: "fmt, __func__, ##args)
#define zocl_warn(dev, fmt, args...)    dev_warn(dev, "%s: "fmt, __func__, ##args)
#define zocl_info(dev, fmt, args...)    dev_info(dev, "%s: "fmt, __func__, ##args)
#define zocl_dbg(dev, fmt, args...)     dev_dbg(dev, "%s: "fmt, __func__, ##args)

struct platform_device *zocl_find_pdev(char *name);

static inline void reg_write(void __iomem *base, u64 off, u32 val)
{
	iowrite32(val, base + off);
}

static inline u32 reg_read(void __iomem *base, u64 off)
{
	return ioread32(base + off);
}

/* Get the platform device node */
static inline struct zocl_drm_dev *zocl_get_zdev(void)
{
	struct platform_device *pdev = zocl_find_pdev("zyxclmm_drm");
	if(!pdev)
		return NULL;

	return platform_get_drvdata(pdev);
}

#endif
