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
#include "zocl_xgq.h"

/* Driver Debug Macros */
#define ZXGQ2DEV(zxgq)                 (&ZDEV2PDEV(zxgq)->dev)
#define zxgq_err(zxgq, fmt, args...)   zocl_err(ZXGQ2DEV(zxgq), fmt"\n", ##args)
#define zxgq_warn(zxgq, fmt, args...)  zocl_warn(ZXGQ2DEV(zxgq), fmt"\n", ##args)
#define zxgq_info(zxgq, fmt, args...)  zocl_info(ZXGQ2DEV(zxgq), fmt"\n", ##args)
#define zxgq_dbg(zxgq, fmt, args...)   zocl_dbg(ZXGQ2DEV(zxgq), fmt"\n", ##args)

/* XGQ driver name. */
#define ZCU_XGQ_DRIVER_NAME "zocl-xgq"

struct zocl_xgq_dev {
	/* XGQ platform device list */
	struct platform_device          *pdev;
	struct zocl_drm_dev             *zdev_parent;
	struct list_head                list;
};

static int zocl_xgq_attach(struct platform_device *pdev, u32 xgq_id, void *arg)
{
	/* FIXME : Fill this function */
	return 0;
}

static int zocl_xgq_detach(struct platform_device *pdev, u32 xgq_id)
{
	/* FIXME : Fill this function */
	return 0;
}

static struct zocl_xgq_drv_data zocl_xgq_drvdata = {
	.attach = zocl_xgq_attach,
	.detach = zocl_xgq_detach,
};

static int zocl_xgq_probe(struct platform_device *pdev)
{
	struct zocl_drm_dev             *zdev = NULL;
	struct zocl_xgq_dev             *zxgq_dev = NULL;

	/* Create zocl device and initial */
	zxgq_dev = devm_kzalloc(&pdev->dev, sizeof(*zxgq_dev), GFP_KERNEL);
	if (!zxgq_dev)
		return -ENOMEM;

	zxgq_dev->pdev = pdev;
	platform_set_drvdata(pdev, zxgq_dev);

	zdev = zocl_get_zdev();
	if (!zdev) {
		zxgq_info(zxgq_dev, "ZOCL Device not yet initialized");
		return -ENODEV;
	}

	/* Add this device to the global xgq device list */
	mutex_lock(&zdev->dev_list_lock);
	list_add_tail(&zxgq_dev->list, &zdev->zxgq_dev_list_head);
	mutex_unlock(&zdev->dev_list_lock);

	/* FIXME : XGQ specific initialization starts from here */
	zxgq_info(zxgq_dev, "Platform device Probed");
	return 0;
}

static int zocl_xgq_remove(struct platform_device *pdev)
{
	struct zocl_xgq_dev *zxgq_dev = platform_get_drvdata(pdev);

	if (!zxgq_dev)
		return -EINVAL;

	zxgq_info(zxgq_dev, "Platform device Removed");
	return 0;
}

static const struct of_device_id zocl_xgq_of_match[] = {
	{ .compatible = "xlnx,cmd-queue-1.0", .data = &zocl_xgq_drvdata},
	{ /* end of table */ },
};
MODULE_DEVICE_TABLE(of, zocl_xgq_of_match);

struct platform_driver zocl_xgq_driver = {
	.probe  = zocl_xgq_probe,
	.remove = zocl_xgq_remove,

	.driver = {
		.name = ZCU_XGQ_DRIVER_NAME,
		.of_match_table = zocl_xgq_of_match,
	},
};
