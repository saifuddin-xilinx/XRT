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

#ifndef _ZOCL_DRV_H_
#define _ZOCL_DRV_H_

#include <drm/drm.h>
#include <drm/drm_device.h>
#include <drm/drm_drv.h>
#include <linux/version.h>
#include <linux/list.h>

/* Platform device driver declaration */
extern struct platform_driver zocl_xgq_driver;
extern struct platform_driver zocl_ospi_versal_driver;
extern struct platform_driver zocl_ert_driver;
extern struct platform_driver zocl_csr_intc_driver;
extern struct platform_driver zocl_irq_intc_driver;
extern struct platform_driver zocl_rpu_channel_driver;
extern struct platform_driver zocl_platfrm_cu_driver;
extern struct platform_driver zocl_platfrm_scu_driver;

/*
 * zocl drm dev specific data info, if there are different configs across
 * different compitible device, add their specific data here.
 */
struct zdev_data {
	char fpga_driver_name[64];
	char fpga_driver_new_name[64];
};

struct zocl_drm_dev {
        /* platform device */
	struct platform_device		*pdev;

	/* DRM device handler */
	struct drm_device		*drm_zdev;

	/* Platform device handlers */
	struct zocl_ospi_versal_dev	*zospi_dev;
	struct zocl_rpu_channel_dev	*zrpu_dev;
	struct zocl_ert_dev		*zert_dev;
	struct list_head		zxgq_dev_list_head;
	struct list_head		zirq_dev_list_head;
	struct list_head		zcsr_dev_list_head;
	struct mutex			dev_list_lock;
	//struct zocl_xgq_dev		*zxgq_dev;
	//struct zocl_irq_intc_dev	*zirq_dev;
	//struct zocl_csr_intc_dev	*zcsr_dev;

	/* Sub module data structure handler */
	void				*zdev_bo;
	void				*zdev_sched;

	//struct zocl_drm_bo_manager	*zdev_bo;
	//struct zocl_scheduler		*zdev_sched;
	struct zocl_ctx_manager		*zdev_ctx;
	struct zocl_xclbin		*zdev_xclbin;
	struct zocl_aie			*zdev_aie;

	/* Other utility data structure */
};

#endif
