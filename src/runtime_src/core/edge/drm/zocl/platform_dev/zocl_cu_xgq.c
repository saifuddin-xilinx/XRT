/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * Copyright (C) 2021-2023 Xilinx, Inc. All rights reserved.
 *
 * Author(s):
 *        Max Zhen <maxz@xilinx.com>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include "zocl_common.h"
#include "zocl_cu_xgq.h"

/* Driver Debug Macros */
#define ZXGQ2DEV(zxgq)                 (&ZDEV2PDEV(zxgq)->dev)
#define zxgq_err(zxgq, fmt, args...)   zocl_err(ZXGQ2DEV(zxgq), fmt"\n", ##args)
#define zxgq_warn(zxgq, fmt, args...)  zocl_warn(ZXGQ2DEV(zxgq), fmt"\n", ##args)
#define zxgq_info(zxgq, fmt, args...)  zocl_info(ZXGQ2DEV(zxgq), fmt"\n", ##args)
#define zxgq_dbg(zxgq, fmt, args...)   zocl_dbg(ZXGQ2DEV(zxgq), fmt"\n", ##args)

/* CU XGQ driver name. */
#define ZCU_XGQ_DRIVER_NAME "zocl-cu-xgq"

static int zocl_cu_xgq_probe(struct platform_device *pdev)
{
	struct zocl_cu_xgq_dev             *zxgq_dev = NULL;

	/* Create zocl device and initial */
	zxgq_dev = devm_kzalloc(&pdev->dev, sizeof(*zxgq_dev), GFP_KERNEL);
	if (!zxgq_dev)
		return -ENOMEM;

	zxgq_info(zxgq_dev, "Platform device Probed");
	return 0;
}

static int zocl_cu_xgq_remove(struct platform_device *pdev)
{
	struct zocl_cu_xgq_dev *zxgq_dev = platform_get_drvdata(pdev);

	if (!zxgq_dev)
		return -EINVAL;

	zxgq_info(zxgq_dev, "Platform device Removed");
	return 0;
}

static const struct of_device_id zocl_cu_xgq_of_match[] = {
	{ .compatible = "xlnx,cmd-queue-1.0"},
	{ /* end of table */ },
};
MODULE_DEVICE_TABLE(of, zocl_cu_xgq_of_match);

struct platform_driver zocl_cu_xgq_driver = {
	.probe  = zocl_cu_xgq_probe,
	.remove = zocl_cu_xgq_remove,

	.driver = {
		.name = ZCU_XGQ_DRIVER_NAME,
		.of_match_table = zocl_cu_xgq_of_match,
	},
};
