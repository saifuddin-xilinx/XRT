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

#include "zocl_bo.h"

#define zbo_err(fmt, args...)	DRM_ERROR(fmt"\n", ##args)
#define zbo_warn(fmt, args...)  DRM_WARN(fmt"\n", ##args)
#define zbo_info(fmt, args...)  DRM_INFO(fmt"\n", ##args)
#define zbo_dbg(fmt, args...)	DRM_DBG(fmt"\n", ##args)

struct zocl_drm_bo_manager {
	/* Reference to the parent driver data structure */
	struct zocl_drm_dev     *zdev;

	/* DRM MM node for PL-DDR */
	struct drm_mm           *zdrm_bo_manager;
	struct mutex             zdrm_mm_lock;

	/* Zocl driver memory list head */
	struct list_head         zdrm_mm_list_head;

	struct iommu_domain     *zocl_domain;
	phys_addr_t              zocl_host_mem;
	resource_size_t          zocl_host_mem_len;

	/* Register necessery BO Ops here */
	struct zocl_drm_bo_ops	*bo_ops;
};

void zocl_drm_mm_init(struct zocl_drm_dev *zdev, void *bo_handlr)
{
	/* FIXME */
}

void zocl_drm_mm_release(struct zocl_drm_dev *zdev, void *bo_handlr)
{
	/* FIXME */
}

/*
 * BO Interfaces exposed to other part of the drivers.
 */
static struct zocl_drm_bo_ops zocl_bo_ops = {
        .init = zocl_drm_mm_init,
	.release = zocl_drm_mm_release,
};

int zocl_bo_memory_register(struct zocl_drm_dev *zdev, void *bo_handlr)
{
	struct zocl_drm_bo_manager	*zbo_manager = NULL;

	zbo_manager = vzalloc(sizeof(struct zocl_drm_bo_manager));
	if (!zbo_manager)
		return -ENOMEM;

	zbo_manager->bo_ops = &zocl_bo_ops;

	/* Store thr bo manager reference here for future references */
	zbo_manager->zdev = zdev;
	bo_handlr = zbo_manager;

	return 0;
}

int zocl_bo_memory_unregister(void *bo_handlr)
{
	if (!bo_handlr)
		return -EINVAL;

	/* Free the existing bo manager here */
	vfree(bo_handlr);
	return 0;
}
