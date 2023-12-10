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
#define ZIRQ2DEV(zirq)                 (&ZDEV2PDEV(zirq)->dev)
#define zirq_err(zirq, fmt, args...)   zocl_err(ZIRQ2DEV(zirq), fmt"\n", ##args)
#define zirq_warn(zirq, fmt, args...)  zocl_warn(ZIRQ2DEV(zirq), fmt"\n", ##args)
#define zirq_info(zirq, fmt, args...)  zocl_info(ZIRQ2DEV(zirq), fmt"\n", ##args)
#define zirq_dbg(zirq, fmt, args...)   zocl_dbg(ZIRQ2DEV(zirq), fmt"\n", ##args)

#define ZOCL_IRQ_DRIVER_NAME "zocl-irq-intc"
#define ZOCL_IRQ_INTC_DRIVER_NAME "ZOCL_IRQ_INTC"

struct zocl_irq_intc_dev {
	/* IRQ INTC platform device list */
	struct platform_device          *pdev;
	struct zocl_intc_handler	*intc_handlr;

	struct list_head                list;
};

static void zirq_intc_config(struct platform_device *pdev, u32 id,
				 bool enabled)
{
	 /* TODO : FILL IT */
}

static void zirq_intc_remove(struct platform_device *pdev, u32 id)
{
	/* TODO : FILL IT */
}

static int zirq_intc_add(struct platform_device *pdev, u32 id,
			     irq_handler_t cb, void *arg)
{
	/* TODO : FILL IT */
	return 0;
}

/*
 * Interfaces exposed to other platform drivers.
 */
static struct zocl_intc_drv_data zocl_irq_intc_drvdata = {
	.add = zirq_intc_add,
	.remove = zirq_intc_remove,
	.config = zirq_intc_config
};

static const struct platform_device_id zocl_irq_intc_id_match[] = {
	{ ZOCL_IRQ_INTC_DRIVER_NAME, (kernel_ulong_t)&zocl_irq_intc_drvdata },
	{ /* end of table */ },
};

static int zocl_irq_intc_probe(struct platform_device *pdev)
{
	struct zocl_drm_dev             *zdev = NULL;
	struct zocl_irq_intc_dev	*zirq_dev = NULL;

	/* Create zocl irq_intc device and initial */
	zirq_dev = devm_kzalloc(&pdev->dev, sizeof(*zirq_dev), GFP_KERNEL);
	if (!zirq_dev)
		return -ENOMEM;

	zirq_dev->pdev = pdev;
	platform_set_drvdata(pdev, zirq_dev);

        zdev = zocl_get_zdev();
        if (!zdev) {
                zirq_info(zirq_dev, "ZOCL Device not yet initialized");
                return -ENODEV;
        }

        /* Add this device to the global irq device list */
        mutex_lock(&zdev->dev_list_lock);
        list_add_tail(&zirq_dev->list, &zdev->zirq_dev_list_head);
        mutex_unlock(&zdev->dev_list_lock);

        /* FIXME : IRQ INTC specific initialization starts from here */

	zirq_info(zirq_dev, "Platform device Probed");
	return 0;
}

static int zocl_irq_intc_remove(struct platform_device *pdev)
{
	struct zocl_irq_intc_dev *zirq_dev = platform_get_drvdata(pdev);

	if (!zirq_dev)
		return -EINVAL;

	zirq_info(zirq_dev, "Platform device Removed");
	return 0;
}

struct platform_driver zocl_irq_intc_driver = {
	.probe  = zocl_irq_intc_probe,
	.remove = zocl_irq_intc_remove,
	.id_table = zocl_irq_intc_id_match,

	.driver = {
		.name = ZOCL_IRQ_DRIVER_NAME,
	},
};
