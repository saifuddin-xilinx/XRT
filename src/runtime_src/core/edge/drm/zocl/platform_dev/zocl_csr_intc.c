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
#include <linux/interrupt.h>

#include "zocl_common.h"
#include "zocl_intc.h"

/* Driver Debug Macros */
#define ZCSR2DEV(zcsr)                 (&ZDEV2PDEV(zcsr)->dev)
#define zcsr_err(zcsr, fmt, args...)   zocl_err(ZCSR2DEV(zcsr), fmt"\n", ##args)
#define zcsr_warn(zcsr, fmt, args...)  zocl_warn(ZCSR2DEV(zcsr), fmt"\n", ##args)
#define zcsr_info(zcsr, fmt, args...)  zocl_info(ZCSR2DEV(zcsr), fmt"\n", ##args)
#define zcsr_dbg(zcsr, fmt, args...)   zocl_dbg(ZCSR2DEV(zcsr), fmt"\n", ##args)

#define ZOCL_CSR_DRIVER_NAME "zocl-csr-intc"
#define ZOCL_CSR_INTC_DRIVER_NAME "ZOCL_CSR_INTC"

struct zocl_csr_intc_dev {
	/* IRQ INTC platform device list */
	struct platform_device          *pdev;
	struct zocl_intc_handler        *intc_handlr;

	struct list_head                list;

};

static void zcsr_intc_config(struct platform_device *pdev, u32 id,
                                 bool enabled)
{
         /* TODO : FILL IT */
}

static void zcsr_intc_remove(struct platform_device *pdev, u32 id)
{
        /* TODO : FILL IT */
}

static int zcsr_intc_add(struct platform_device *pdev, u32 id,
                             irq_handler_t cb, void *arg)
{
        /* TODO : FILL IT */
        return 0;
}

/*
 * Interfaces exposed to other subdev drivers.
 */
static struct zocl_intc_drv_data zocl_csr_intc_drvdata = {
        .add = zcsr_intc_add,
        .remove = zcsr_intc_remove,
        .config = zcsr_intc_config
};

static const struct platform_device_id zocl_csr_intc_id_match[] = {
	{ ZOCL_CSR_INTC_DRIVER_NAME, (kernel_ulong_t)&zocl_csr_intc_drvdata },
	{ /* end of table */ },
};

static int zocl_csr_intc_probe(struct platform_device *pdev)
{
        struct zocl_drm_dev             *zdev = NULL;
	struct zocl_csr_intc_dev	*zcsr_dev = NULL;

	/* Create zocl csr_intc device and initial */
	zcsr_dev = devm_kzalloc(&pdev->dev, sizeof(*zcsr_dev), GFP_KERNEL);
	if (!zcsr_dev)
		return -ENOMEM;

	zcsr_dev->pdev = pdev;
	platform_set_drvdata(pdev, zcsr_dev);

        zdev = zocl_get_zdev();
        if (!zdev) {
                zcsr_info(zcsr_dev, "ZOCL Device not yet initialized");
                return -ENODEV;
        }

        /* Add this device to the global xgq device list */
        mutex_lock(&zdev->dev_list_lock);
        list_add_tail(&zcsr_dev->list, &zdev->zcsr_dev_list_head);
        mutex_unlock(&zdev->dev_list_lock);

        /* FIXME : CSR INTC specific initialization starts from here */

	zcsr_info(zcsr_dev, "Platform device Probed");
	return 0;
}

static int zocl_csr_intc_remove(struct platform_device *pdev)
{
	struct zocl_csr_intc_dev *zcsr_dev = platform_get_drvdata(pdev);

	if (!zcsr_dev)
		return -EINVAL;

	zcsr_info(zcsr_dev, "Platform device Removed");
	return 0;
}

struct platform_driver zocl_csr_intc_driver = {
	.probe  = zocl_csr_intc_probe,
	.remove = zocl_csr_intc_remove,
	.id_table = zocl_csr_intc_id_match,

	.driver = {
		.name = ZOCL_CSR_DRIVER_NAME,
	},
};
