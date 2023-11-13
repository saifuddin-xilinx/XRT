/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * A GEM style (optionally CMA backed) device manager for ZynQ based
 * OpenCL accelerators.
 *
 * Copyright (C) 2016-2022 Xilinx, Inc. All rights reserved.
 *
 * Authors:
 *    Sonal Santan <sonal.santan@xilinx.com>
 *    Umang Parekh <umang.parekh@xilinx.com>
 *    Jan Stephan  <j.stephan@hzdr.de>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#include <linux/pagemap.h>
#include <linux/of_address.h>
#include <linux/iommu.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0))
        #include <linux/iosys-map.h>
        #define ZOCL_MAP_TYPE iosys_map
        #define ZOCL_MAP_IS_NULL iosys_map_is_null
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0))
        #include <linux/dma-buf-map.h>
        #define ZOCL_MAP_TYPE dma_buf_map
        #define ZOCL_MAP_IS_NULL dma_buf_map_is_null
#endif

#include "zocl_sched.h"

#define zsched_err(fmt, args...)	DRM_ERROR(fmt"\n", ##args)
#define zsched_warn(fmt, args...)  DRM_WARN(fmt"\n", ##args)
#define zsched_info(fmt, args...)  DRM_INFO(fmt"\n", ##args)
#define zsched_dbg(fmt, args...)	DRM_DBG(fmt"\n", ##args)

struct zocl_drm_sched_manager {
	/* Reference to the parent driver data structure */
	struct zocl_drm_dev	*zdev;

	/* Register necessery BO Ops here */
	struct zocl_sched_ops	*sched_ops;
};

void zocl_drm_sched_init(struct zocl_drm_dev *zdev, void *sched_handlr)
{
	/* FIXME */
}

void zocl_drm_sched_release(struct zocl_drm_dev *zdev, void *sched_handlr)
{
	/* FIXME */
}

/*
 * BO Interfaces exposed to other part of the drivers.
 */
static struct zocl_sched_ops zocl_drm_sched_ops = {
        .init = zocl_drm_sched_init,
	.release = zocl_drm_sched_release,
};

int zocl_drm_sched_register(struct zocl_drm_dev *zdev, void *sched_handlr)
{
	struct zocl_drm_sched_manager	*zsched_manager = NULL;

	zsched_manager = vzalloc(sizeof(struct zocl_drm_sched_manager));
	if (!zsched_manager)
		return -ENOMEM;

	zsched_manager->sched_ops = &zocl_drm_sched_ops;

	/* Store the sched manager reference here for future references */
	zsched_manager->zdev = zdev;
	sched_handlr = zsched_manager;

	return 0;
}

int zocl_drm_sched_unregister(void *sched_handlr)
{
	if (!sched_handlr)
		return -EINVAL;

	/* Free the existing sched manager here */
	vfree(sched_handlr);
	return 0;
}
