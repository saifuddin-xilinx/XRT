/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Authors:
 *      ch.vamshi.krishna@amd.com
 *      saifuddin.kaijar@amd.com
 *      Himanshu.Choudhary@amd.com
 *      yidong.zhang@amd.com
 */

#include <linux/pagemap.h>
#include <linux/of_address.h>
#include <linux/iommu.h>
#include <linux/version.h>
#include <drm/drm_print.h>
#include <drm/drm_mm.h>
#include <drm/drm_gem.h>
#include <drm/drm_cache.h>
#include <drm/drm_gem_dma_helper.h>

#include "zocl_bo.h"

#define xmm_err(fmt, args...)	DRM_ERROR(fmt"\n", ##args)
#define xmm_warn(fmt, args...)  DRM_WARN(fmt"\n", ##args)
#define xmm_info(fmt, args...)  DRM_INFO(fmt"\n", ##args)
#define xmm_dbg(fmt, args...)	DRM_DEBUG(fmt"\n", ##args)

#define to_zocl_bo(gobj) \
	((struct zocl_drm_bo *)container_of((gobj), struct zocl_drm_bo, gem_base))

struct zocl_drm_bo {
	union {
		/* For CMA allocator */
		struct drm_gem_dma_object       dma_base;

		/* For Range allocator */
		struct {
			struct drm_gem_object		gem_base;

			/* userptr only */
			struct sg_table		       *bo_sgt;
			struct page		      **bo_pages;
			u32				bo_nr_pages;
			void			       *bo_vmap;
			/* end userptr only */
		};
	};

	void				*priv_handlr;

	/* Common BO specific fileds */
	struct drm_mm_node              *bo_mm_node;
	unsigned int                     mem_index;
	uint32_t                         bo_flags;
	/* SAIF TODO : Need to identify the additional fileds */
};

static int zocl_drm_mm_release(struct zdrm_mem_manager *mm_handlr)
{
	xmm_info("I am inside %s", __func__);
	/* FIXME */
	return 0;
}

static int zocl_drm_mm_init(struct zdrm_mem_manager *mm_handlr,
			       struct mem_topology *m_topo)
{
	xmm_info("I am inside %s", __func__);
	/* FIXME */
	return 0;
}

static int zocl_drm_create_device_bo(struct zdrm_mem_manager *mm_handlr,
		struct drm_file *filp, uint64_t bo_size, uint32_t bo_flags)
{
	xmm_info("I am inside %s", __func__);
	/* FIXME */
	return 0;
}

static int zocl_drm_create_userptr_bo(struct zdrm_mem_manager *mm_handlr,
					uint64_t bo_size)
{
	xmm_info("I am inside %s", __func__);
	/* FIXME */
	return 0;
}

static int zocl_drm_get_info_bo(struct zdrm_mem_manager *mm_handlr,
				  uint32_t bo_handlr)
{
	xmm_info("I am inside %s", __func__);
	/* FIXME */
	return 0;
}

static int zocl_drm_sync_bo(struct zdrm_mem_manager *mm_handlr,
			      uint32_t bo_handlr)
{
	xmm_info("I am inside %s", __func__);
	/* FIXME */
	return 0;
}
/*
 * BO Interfaces exposed to other part of the drivers.
 */
static struct zocl_drm_bo_ops zocl_bo_ops = {
        .init = zocl_drm_mm_init,
	.release = zocl_drm_mm_release,
	.create_userptr_bo = zocl_drm_create_userptr_bo,
	.create_device_bo = zocl_drm_create_device_bo,
	.get_info_bo = zocl_drm_get_info_bo,
	.sync_bo = zocl_drm_sync_bo,
};

int zocl_mm_register(struct zdrm_mem_manager **mm_handlr, void *parnt_adev)
{
	struct zdrm_mem_manager	*zmem_manager = NULL;

	zmem_manager = vzalloc(sizeof(struct zdrm_mem_manager));
	if (!zmem_manager)
		return -ENOMEM;

	zmem_manager->bo_ops = &zocl_bo_ops;

	rwlock_init(&zmem_manager->bo_rwlock);
	INIT_LIST_HEAD(&zmem_manager->zdrm_mm_list_head);
	zmem_manager->private_adev = parnt_adev;
	/* Store the memory manager reference here for future references */
	*mm_handlr = zmem_manager;

	return 0;
}

int zocl_mm_unregister(struct zdrm_mem_manager *mm_handlr)
{
	if (!mm_handlr)
		return -EINVAL;

	/* Free the existing memory manager here */
	vfree(mm_handlr);
	return 0;
}
