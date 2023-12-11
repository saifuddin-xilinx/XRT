/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Authors:
 *      ch.vamshi.krishna@amd.com
 *      saifuddin.kaijar@amd.com
 */

#ifndef _ZOCL_BO_H_
#define _ZOCL_BO_H_

#include <drm/drm_file.h>

/* XCLBIN Headers */
#include "xclbin.h"

#include "zocl_common.h"

/* Added this bitmask aligned with XRT core utils */
#define ZOCL_BO_FLAGS_USERPTR           (1 << 28)
#define ZOCL_BO_FLAGS_CMA               (1 << 29)
#define ZOCL_BO_FLAGS_EXECBUF           (1 << 31)

/*
 * Get the bank index from BO creation flags.
 * bits  0 ~ 15: DDR BANK index
 */
#define MEM_BANK_SHIFT_BIT      11
#define GET_MEM_INDEX(x)        ((x) & 0xFFFF)
#define GET_SLOT_INDEX(x)       (((x) >> MEM_BANK_SHIFT_BIT) & 0x7FF)
#define SET_MEM_INDEX(x, y)     (((x) << MEM_BANK_SHIFT_BIT) | y)

enum zocl_mem_type {
	ZOCL_MEM_TYPE_CMA               = 0,
	ZOCL_MEM_TYPE_RANGE_ALLOC       = 1,
};

/* Maintain a driver level stastics for the bo/mem of the device */
struct drm_zocl_mm_stat {
	size_t				memory_usage;
	u32				bo_count;
};

/*
 * Memory structure in zocl driver. There will be an array of this
 * structure where each element is representing each section in
 * the memory topology in xclbin.
 */
struct zocl_mem_info {
	u32				am_mem_idx;
	unsigned int			am_used;
	enum zocl_mem_type		am_type;
	u64				am_base_addr;
	u64				am_end_addr;
	u64				am_size;
	struct drm_zocl_mm_stat	am_stat;
	struct list_head		link;
};

struct zdrm_mem_manager {
        /* Reference to the parent driver data structure */
        void                            *private_adev;

        /* DRM MM node for PL-DDR */
        struct drm_mm                   *zdrm_mm;
        struct mutex                     zdrm_mm_lock;

        /* Driver memory list head */
        struct list_head                 zdrm_mm_list_head;

        /* This RW lock is to protect the memory sysfs nodes exported
         * by zocl driver.*/
        rwlock_t			 bo_rwlock;

        phys_addr_t                      zocl_host_mem;
        resource_size_t                  zocl_host_memlen;

        /* Register necessery BO Ops here */
        struct zocl_drm_bo_ops         *bo_ops;
};

struct zocl_drm_bo_ops {
        /**
         * @init: Initialize Memory management here. This will be done once while
	 * first XCLBIN is downloading.
         */
        int (*init)(struct zdrm_mem_manager *mm_handlr,
				struct mem_topology *m_topo);

	/**
         * @release: Release existing memory management. This will be done if we
	 * need to reinitialize the cuurent memory manager or driver is unloading.
         */
	int (*release)(struct zdrm_mem_manager *mm_handlr);

	/**
         * @create_userptr_bo: Create a user Pointer BO here for the requested
	 * size.
         */
	int (*create_userptr_bo)(struct zdrm_mem_manager *mm_handlr,
				 uint64_t size);

	/**
         * @create_device_bo: Create a BO in device memory for the requested
	 * size.
         */
	int (*create_device_bo)(struct zdrm_mem_manager *mm_handlr,
		struct drm_file *filp, uint64_t bo_size, uint32_t bo_flags);

	/**
         * @get_info_bo: Return the details about the requested BO.
         */
	int (*get_info_bo)(struct zdrm_mem_manager *mm_handlr,
				 uint32_t bo_handlr);

	/**
         * @sync_bo: SYNC the requested BO.
         */
	int (*sync_bo)(struct zdrm_mem_manager *mm_handlr,
				 uint32_t bo_handlr);

};

#define aMEM_MANAGER(mm_handlr)				\
	((struct zdrm_mem_manager *)(mm_handlr))
#define ZBO_OPS(mm_handlr)				\
	((struct zocl_drm_bo_ops *)(aMEM_MANAGER(mm_handlr))->bo_ops)
#define aMEM_DRVHANDLR(mm_handlr)			\
	((void *)(aMEM_MANAGER(mm_handlr))->private_adev)
#define aMEM_DRMHANDLR(mm_handlr)			\
	((void *)(aDRV_DRMHNDLR(aMEM_DRVHANDLR(mm_handlr))))

int zocl_mm_register(struct zdrm_mem_manager **mm_handlr, void *parnt_adev);
int zocl_mm_unregister(struct zdrm_mem_manager *mm_handlr);

static inline int
zocl_mm_init(struct zdrm_mem_manager *mem_mngr, struct mem_topology *m_topo)
{
	return ZBO_OPS(mem_mngr) ?
		ZBO_OPS(mem_mngr)->init(mem_mngr, m_topo) : -EINVAL;
}

static inline int
zocl_mm_release(struct zdrm_mem_manager *mem_mngr)
{
	return ZBO_OPS(mem_mngr) ?
		ZBO_OPS(mem_mngr)->release(mem_mngr) : -EINVAL;
}

static inline int
zocl_mm_create_userptr_bo(struct zdrm_mem_manager *mem_mngr, uint64_t size)
{
	return ZBO_OPS(mem_mngr) ?
		ZBO_OPS(mem_mngr)->create_userptr_bo(mem_mngr, size) : -EINVAL;
}

static inline int
zocl_mm_create_device_bo(struct zdrm_mem_manager *mem_mngr,
		struct drm_file *filp, uint64_t bo_size, uint32_t bo_flags)
{
	return ZBO_OPS(mem_mngr) ?
		ZBO_OPS(mem_mngr)->create_device_bo(mem_mngr,
						    filp, bo_size, bo_flags) : -EINVAL;
}

static inline int
zocl_mm_get_info_bo(struct zdrm_mem_manager *mem_mngr, uint32_t bo_handlr)
{
	return ZBO_OPS(mem_mngr) ?
		ZBO_OPS(mem_mngr)->get_info_bo(mem_mngr, bo_handlr) : -EINVAL;
}

static inline int
zocl_mm_sync_bo(struct zdrm_mem_manager *mem_mngr, uint32_t bo_handlr)
{
	return ZBO_OPS(mem_mngr) ?
		ZBO_OPS(mem_mngr)->sync_bo(mem_mngr, bo_handlr) : -EINVAL;
}

#endif
