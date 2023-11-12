/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * Copyright (C) 2016-2022 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Author(s):
 *        Min Ma <min.ma@xilinx.com>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#ifndef _ZOCL_IRQ_INTC_H_
#define _ZOCL_IRQ_INTC_H_

struct zocl_ert_intc_drv_data {
	int (*add)(struct platform_device *pdev, u32 id, irq_handler_t handler, void *arg);
	void (*remove)(struct platform_device *pdev, u32 id);
	void (*config)(struct platform_device *pdev, u32 id, bool enabled);
};

struct zocl_irq_intc_dev {
	/* IRQ platform device list */
	struct platform_device          *pdev;
	struct zocl_drm_dev             *zdev_parent;
	struct list_head                list;

};

#endif
