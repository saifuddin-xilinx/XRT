/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * A GEM style device manager for PCIe based OpenCL accelerators.
 *
 * Copyright (C) 2016-2022 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Authors:
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#include <linux/device.h>
#include "zocl_common.h"

#if KERNEL_VERSION(5, 3, 0) <= LINUX_VERSION_CODE
static int
match_name(struct device *dev, const void *data)
#else
static int
match_name(struct device *dev, void *data)
#endif
{
        const char *name = data;
        /*
         * check if given name is substring inside dev.
         * the dev_name is like: 20300030000.ert_hw
         */
        return strstr(dev_name(dev), name) != NULL;
}

/**
 * Find platform device by name
 *
 * @param       name: device name
 *
 * Returns a platform device. Returns NULL if not found.
 */
struct platform_device *zocl_find_pdev(char *name)
{
	struct device *dev;

	dev = bus_find_device(&platform_bus_type, NULL, (void *)name,
			      match_name);
	if (!dev)
		return NULL;

	return container_of(dev, struct platform_device, dev);
}

inline struct zocl_drm_dev *
zocl_get_zdev(void)
{
	struct platform_device *pdev = zocl_find_pdev("zyxclmm_drm");
	if(!pdev)
		return NULL;

	return platform_get_drvdata(pdev);
}
