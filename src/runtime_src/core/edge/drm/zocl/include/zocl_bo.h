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

#ifndef _ZOCL_BO_H_
#define _ZOCL_BO_H_

struct zocl_drm_mem_manager {
	/* DRM MM node for PL-DDR */
	struct drm_mm           *zdrm_mem_manager;    i
	struct mutex             zdrm_mm_lock;

	/* Zocl driver memory list head */
	struct list_head         zdrm_mm_list_head;

	struct iommu_domain     *zocl_domain;
	phys_addr_t              zocl_host_mem;
	resource_size_t          zocl_host_mem_len;
};

#endif
