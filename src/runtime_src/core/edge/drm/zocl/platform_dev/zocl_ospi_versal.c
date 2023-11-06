/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * A GEM style (optionally CMA backed) device manager for ZynQ based
 * OpenCL accelerators.
 *
 * Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
 *
 * Authors:
 *    Larry Liu <yliu@xilinx.com>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include "zocl_common.h"
#include "zocl_ospi_versal.h"

/* Driver Debug Macros */
#define ZOSPI2DEV(zospi)                 (&ZDEV2PDEV(zospi)->dev)
#define zospi_err(zospi, fmt, args...)   zocl_err(ZOSPI2DEV(zospi), fmt"\n", ##args)
#define zospi_warn(zospi, fmt, args...)  zocl_warn(ZOSPI2DEV(zospi), fmt"\n", ##args)
#define zospi_info(zospi, fmt, args...)  zocl_info(ZOSPI2DEV(zospi), fmt"\n", ##args)
#define zospi_dbg(zospi, fmt, args...)   zocl_dbg(ZOSPI2DEV(zospi), fmt"\n", ##args)

/* OSPI VERSAL driver name */
#define	ZOSPI_VERSAL_DRIVER_NAME "zocl-ospi-versal"

static const struct of_device_id zocl_ospi_versal_of_match[] = {
	{ .compatible = "xlnx,ospi_versal"},
	{ .compatible = "xlnx,mpsoc_ocm"},
	{ /* end of table */ },
};

MODULE_DEVICE_TABLE(of, zocl_ospi_versal_of_match);

static int zocl_ospi_versal_probe(struct platform_device  *pdev)
{
	struct zocl_ospi_versal_dev     *zospi_dev = NULL;

	/* Create zocl ospi device and initial */
	zospi_dev = devm_kzalloc(&pdev->dev, sizeof(*zospi_dev), GFP_KERNEL);
	if (!zospi_dev)
		return -ENOMEM;

	zospi_info(zospi_dev, "Platform device Probed");
	return 0;
}

static int zocl_ospi_versal_remove(struct platform_device *pdev)
{
	struct zocl_ospi_versal_dev *zospi_dev = platform_get_drvdata(pdev);

	if (!zospi_dev)
		return -EINVAL;

	zospi_info(zospi_dev, "Platform device Removed");
	return 0;
}

struct platform_driver zocl_ospi_versal_driver = {
	.driver = {
		.name = ZOSPI_VERSAL_DRIVER_NAME,
		.of_match_table = zocl_ospi_versal_of_match,
	},
	.probe  = zocl_ospi_versal_probe,
	.remove = zocl_ospi_versal_remove,
};
