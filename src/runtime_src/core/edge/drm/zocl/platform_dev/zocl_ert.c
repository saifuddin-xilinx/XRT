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
#define ZRPU2DEV(zert)                 (&ZDEV2PDEV(zert)->dev)
#define zert_err(zert, fmt, args...)   zocl_err(ZRPU2DEV(zert), fmt"\n", ##args)
#define zert_warn(zert, fmt, args...)  zocl_warn(ZRPU2DEV(zert), fmt"\n", ##args)
#define zert_info(zert, fmt, args...)  zocl_info(ZRPU2DEV(zert), fmt"\n", ##args)
#define zert_dbg(zert, fmt, args...)   zocl_dbg(ZRPU2DEV(zert), fmt"\n", ##args)

#define ZERT_DRIVER_NAME "zert_embedded_sched_versal"

static const struct zocl_ert_info mpsoc_ert_info = {
	/* TODO : Fill it */
	//.ops   = &mpsoc_ops,
};

static const struct zocl_ert_info versal_ert_info = {
	/* TODO : Fill it */
	//.ops   = &versal_ops,
};

static const struct of_device_id zocl_ert_of_match[] = {
	{ .compatible = "xlnx,embedded_sched", .data = &mpsoc_ert_info},
	{ .compatible = "xlnx,embedded_sched_versal", .data = &versal_ert_info},
	{ /* end of table */ },
};

MODULE_DEVICE_TABLE(of, zocl_ert_of_match);

static int zocl_ert_probe(struct platform_device *pdev)
{
	struct zocl_ert_dev		*zert_dev = NULL;
	const struct of_device_id	*id = NULL;

	id = of_match_node(zocl_ert_of_match, pdev->dev.of_node);
	if (!id)
		return -EINVAL;

	zert_dev = devm_kzalloc(&pdev->dev, sizeof(*zert_dev), GFP_KERNEL);
	if (!zert_dev)
		return -ENOMEM;

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

struct platform_driver zocl_ert_driver = {
	.driver = {
		.name = ZERT_DRIVER_NAME,
		.of_match_table = zocl_ert_of_match,
	},
	.probe  = zocl_ert_probe,
	.remove = zocl_ert_remove,
};
