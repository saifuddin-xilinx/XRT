/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * Copyright (C) 2021 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Author(s):
 *        Lizhi Hou <lizhih@xilinx.com>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include "zocl_common.h"
#include "zocl_rpu_channel.h"

/* Driver Debug Macros */
#define ZRPU2DEV(zrpu)                 (&ZDEV2PDEV(zrpu)->dev)
#define zrpu_err(zrpu, fmt, args...)   zocl_err(ZRPU2DEV(zrpu), fmt"\n", ##args)
#define zrpu_warn(zrpu, fmt, args...)  zocl_warn(ZRPU2DEV(zrpu), fmt"\n", ##args)
#define zrpu_info(zrpu, fmt, args...)  zocl_info(ZRPU2DEV(zrpu), fmt"\n", ##args)
#define zrpu_dbg(zrpu, fmt, args...)   zocl_dbg(ZRPU2DEV(zrpu), fmt"\n", ##args)

#define ZRPU_DRIVER_NAME "zocl-rpu-channel"

static const struct of_device_id zocl_rpu_channel_of_match[] = {
	{ .compatible = "xlnx,rpu-channel", },
	{ /* end of table */ },
};

static int zocl_rpu_channel_probe(struct platform_device *pdev)
{
	struct zocl_rpu_channel_dev	*zrpu_dev = NULL;

	/* Create zocl rpu channel device and initial */
	zrpu_dev = devm_kzalloc(&pdev->dev, sizeof(*zrpu_dev), GFP_KERNEL);
	if (!zrpu_dev)
		return -ENOMEM;

	zrpu_dev->pdev = pdev;
	platform_set_drvdata(pdev, zrpu_dev);

	zrpu_info(zrpu_dev, "Platform device Probed");
	return 0;
};

static int zocl_rpu_channel_remove(struct platform_device *pdev)
{
	struct zocl_rpu_channel_dev *zrpu_dev = platform_get_drvdata(pdev);

	if (!zrpu_dev)
		return -EINVAL;

	zrpu_info(zrpu_dev, "Platform device Removed");
	return 0;
};

struct platform_driver zocl_rpu_channel_driver = {
	.driver = {
		.name = ZRPU_DRIVER_NAME,
		.of_match_table = zocl_rpu_channel_of_match,
	},
	.probe = zocl_rpu_channel_probe,
	.remove = zocl_rpu_channel_remove,
};
