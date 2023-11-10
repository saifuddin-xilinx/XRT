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
#include "zocl_xgq_ert.h"

/* Driver Debug Macros */
#define ZXGQERT2DEV(zxgq_ert)                 (&ZDEV2PDEV(zxgq_ert)->dev)
#define zxgq_ert_err(zxgq_ert, fmt, args...)   zocl_err(ZXGQERT2DEV(zxgq_ert), fmt"\n", ##args)
#define zxgq_ert_warn(zxgq_ert, fmt, args...)  zocl_warn(ZXGQERT2DEV(zxgq_ert), fmt"\n", ##args)
#define zxgq_ert_info(zxgq_ert, fmt, args...)  zocl_info(ZXGQERT2DEV(zxgq_ert), fmt"\n", ##args)
#define zxgq_ert_dbg(zxgq_ert, fmt, args...)   zocl_dbg(ZXGQERT2DEV(zxgq_ert), fmt"\n", ##args)

#define ZERT_XGQ_DRIVER_NAME "zert_xgq_embedded_sched_versal"

static const struct of_device_id zocl_xgq_ert_of_match[] = {
	//{ .compatible = "xlnx,embedded_sched", .data = &mpsoc_ert_info},
	//{ .compatible = "xlnx,embedded_sched_versal", .data = &versal_ert_info},
	{ .compatible = "xlnx,embedded_sched"},
	{ .compatible = "xlnx,embedded_sched_versal"},
	{ /* end of table */ },
};

MODULE_DEVICE_TABLE(of, zocl_xgq_ert_of_match);

static int zocl_xgq_ert_probe(struct platform_device *pdev)
{
	struct zocl_xgq_ert_dev     *zxgq_ert_dev = NULL;

	/* Create zocl xgq_ert device and initial */
	zxgq_ert_dev = devm_kzalloc(&pdev->dev, sizeof(*zxgq_ert_dev), GFP_KERNEL);
	if (!zxgq_ert_dev)
		return -ENOMEM;

        zxgq_ert_dev->pdev = pdev;
        platform_set_drvdata(pdev, zxgq_ert_dev);

	zxgq_ert_info(zxgq_ert_dev, "Platform device Probed");
	return 0;

}

static int zocl_xgq_ert_remove(struct platform_device *pdev)
{
	struct zocl_xgq_ert_dev *zxgq_ert_dev = platform_get_drvdata(pdev);

	if (!zxgq_ert_dev)
		return -EINVAL;

	zxgq_ert_info(zxgq_ert_dev, "Platform device Removed");
	return 0;
}

struct platform_driver zocl_xgq_ert_driver = {
	.probe  = zocl_xgq_ert_probe,
	.remove = zocl_xgq_ert_remove,
	.driver = {
		.name = ZERT_XGQ_DRIVER_NAME,
		.of_match_table = zocl_xgq_ert_of_match,
	},
};
