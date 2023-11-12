/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * Copyright (C) 2016-2020 Xilinx, Inc. All rights reserved.
 *
 * Author(s):
 *        Min Ma <min.ma@xilinx.com>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include "zocl_common.h"
#include "zocl_ert.h"

/* Driver Debug Macros */
#define ZERT2DEV(zert)                 (&ZDEV2PDEV(zert)->dev)
#define zert_err(zert, fmt, args...)   zocl_err(ZERT2DEV(zert), fmt"\n", ##args)
#define zert_warn(zert, fmt, args...)  zocl_warn(ZERT2DEV(zert), fmt"\n", ##args)
#define zert_info(zert, fmt, args...)  zocl_info(ZERT2DEV(zert), fmt"\n", ##args)
#define zert_dbg(zert, fmt, args...)   zocl_dbg(ZERT2DEV(zert), fmt"\n", ##args)

#define ZERT_DRIVER_NAME "zert_embedded_sched_versal"

static int zocl_ert_probe(struct platform_device *pdev)
{
	struct zocl_drm_dev		*zdev = NULL;
	struct zocl_ert_dev		*zert_dev = NULL;

	/* Create zocl ert device and initialize */
	zert_dev = devm_kzalloc(&pdev->dev, sizeof(*zert_dev), GFP_KERNEL);
	if (!zert_dev)
		return -ENOMEM;

	zert_dev->pdev = pdev;
	platform_set_drvdata(pdev, zert_dev);

	zdev = zocl_get_zdev();
	if (!zdev) {
		zert_info(zert_dev, "ZOCL Device not yet initialized");
		return -ENODEV;
	}

	zert_dev->zdev_parent = zdev;
	zdev->zert_dev = zert_dev;

	/* FIXME : ERT Related initialization starts from here */

	zert_info(zert_dev, "Platform device Probed");
	return 0;
}

static int zocl_ert_remove(struct platform_device *pdev)
{
	struct zocl_ert_dev *zert_dev = platform_get_drvdata(pdev);

	if (!zert_dev)
		return -EINVAL;

	zert_info(zert_dev, "Platform device Removed");
	return 0;
}

static const struct of_device_id zocl_ert_of_match[] = {
	//{ .compatible = "xlnx,embedded_sched", .data = &mpsoc_ert_info},
	//{ .compatible = "xlnx,embedded_sched_versal", .data = &versal_ert_info},
	{ .compatible = "xlnx,embedded_sched"},
	{ .compatible = "xlnx,embedded_sched_versal"},
	{ /* end of table */ },
};
MODULE_DEVICE_TABLE(of, zocl_ert_of_match);

struct platform_driver zocl_ert_driver = {
	.probe  = zocl_ert_probe,
	.remove = zocl_ert_remove,
	.driver = {
		.name = ZERT_DRIVER_NAME,
		.of_match_table = zocl_ert_of_match,
	},
};
