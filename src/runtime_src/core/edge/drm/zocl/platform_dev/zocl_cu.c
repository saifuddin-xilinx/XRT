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
#include "zocl_cu.h"

/* Driver Debug Macros */
#define ZCU2DEV(zcu)                 (&ZDEV2PDEV(zcu)->dev)
#define zcu_err(zcu, fmt, args...)   zocl_err(ZCU2DEV(zcu), fmt"\n", ##args)
#define zcu_warn(zcu, fmt, args...)  zocl_warn(ZCU2DEV(zcu), fmt"\n", ##args)
#define zcu_info(zcu, fmt, args...)  zocl_info(ZCU2DEV(zcu), fmt"\n", ##args)
#define zcu_dbg(zcu, fmt, args...)   zocl_dbg(ZCU2DEV(zcu), fmt"\n", ##args)

/* ZOCL CU driver name. */
#define ZOCL_CU_DRIVER_NAME "zocl-cu"

#define ZOCL_CU_NAME "ZOCL_CU"

static int zocl_cu_probe(struct platform_device *pdev)
{
	struct zocl_cu_dev             *zcu_dev = NULL;

	/* Create zocl device and initial */
	zcu_dev = devm_kzalloc(&pdev->dev, sizeof(*zcu_dev), GFP_KERNEL);
	if (!zcu_dev)
		return -ENOMEM;

	zcu_dev->pdev = pdev;
	platform_set_drvdata(pdev, zcu_dev);

	zcu_info(zcu_dev, "Platform device Probed");
	return 0;
}

static int zocl_cu_remove(struct platform_device *pdev)
{
	struct zocl_cu_dev *zcu_dev = platform_get_drvdata(pdev);

	if (!zcu_dev)
		return -EINVAL;

	zcu_info(zcu_dev, "Platform device Removed");
	return 0;
}

static const struct platform_device_id zocl_cu_id_match[] = {
	{ ZOCL_CU_NAME, 0 },
	{ /* end of table */ },
};

struct platform_driver zocl_cu_driver = {
	.probe  = zocl_cu_probe,
	.remove = zocl_cu_remove,
	.id_table = zocl_cu_id_match,

	.driver = {
		.name = ZOCL_CU_DRIVER_NAME,
	},
};
