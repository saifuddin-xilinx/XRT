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
#include "zocl_scu.h"

/* Driver Debug Macros */
#define ZSCU2DEV(zscu)                  (&ZDEV2PDEV(zscu)->dev)
#define zscu_err(zscu, fmt, args...)   zocl_err(ZSCU2DEV(zscu), fmt"\n", ##args)
#define zscu_warn(zscu, fmt, args...)  zocl_warn(ZSCU2DEV(zscu), fmt"\n", ##args)
#define zscu_info(zscu, fmt, args...)  zocl_info(ZSCU2DEV(zscu), fmt"\n", ##args)
#define zscu_dbg(zscu, fmt, args...)   zocl_dbg(ZSCU2DEV(zscu), fmt"\n", ##args)

/* ZOCL SCU driver name. */
#define ZOCL_SCU_DRIVER_NAME "zocl-scu"

#define ZOCL_SCU_NAME "ZOCL_SCU"

static int zocl_scu_probe(struct platform_device *pdev)
{
	struct zocl_scu_dev             *zscu_dev = NULL;

	/* Create zocl device and initial */
	zscu_dev = devm_kzalloc(&pdev->dev, sizeof(*zscu_dev), GFP_KERNEL);
	if (!zscu_dev)
		return -ENOMEM;

	zscu_dev->pdev = pdev;
	platform_set_drvdata(pdev, zscu_dev);

	zscu_info(zscu_dev, "Platform device Probed");
	return 0;
}

static int zocl_scu_remove(struct platform_device *pdev)
{
	struct zocl_scu_dev *zscu_dev = platform_get_drvdata(pdev);

	if (!zscu_dev)
		return -EINVAL;

	zscu_info(zscu_dev, "Platform device Removed");
	return 0;
}

static const struct platform_device_id zocl_scu_id_match[] = {
	{ ZOCL_SCU_NAME, 0 },
	{ /* end of table */ },
};

struct platform_driver zocl_scu_driver = {
	.probe  = zocl_scu_probe,
	.remove = zocl_scu_remove,
	.id_table = zocl_scu_id_match,

	.driver = {
		.name = ZOCL_SCU_DRIVER_NAME,
	},
};
