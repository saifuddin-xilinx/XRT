/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Authors:
 *	ch.vamshi.krishna@amd.com
 *	saifuddin.kaijar@amd.com
 *	Himanshu.Choudhary@amd.com
 *	yidong.zhang@amd.com
 *
 */

#ifndef _ZOCL_XCLBIN_H
#define _ZOCL_XCLBIN_H

#include <linux/uuid.h>
#include <linux/kref.h>
/* XCLBIN Headers */
#include "xclbin.h"

enum pdi_type {
	PDI_TYPE_PRE,
	PDI_TYPE_PRIMARY,
	PDI_TYPE_POST,
	PDI_TYPE_LITE,
	MAX_PDI_TYPE
};

struct zocl_pdi {
	uuid_t                          pdi_uuid;
	int                             pdi_id;
	void                            *pdi_image;
	u64                             pdi_addr;
	u32                             pdi_size;
	enum pdi_type                   pdi_type;
	const u64                       *pdi_dpu_ids;
	u32                             pdi_num_dpu_ids;
	void                            *pdi_orig;
};

struct zocl_partition {
	struct zocl_pdi                *part_pdis;
	u32                             part_num_pdis;
	u32                             part_ncols;
	u32                             part_nparts;
	const u16                       *part_start_cols;
	u32                             part_ops;
};

struct zocl_cu {
	char                            cu_name[64];
	u32                             cu_index;
	u32                             cu_func;
	u32                             cu_dpu_id;
	u32                             cu_pdi_id;
};

struct zocl_xclbin {
	struct list_head                xclbin_entry;
	u32				slot_id;
	uuid_t                          xclbin_uuid;
	struct kref                     xclbin_ref;

	const void __user		*xclbin_ptr;

	struct mem_topology             *xclbin_memtopo;
	struct zocl_cu                 *xclbin_cu;
	struct zocl_partition          xclbin_adev_part;
	u32                             xclbin_num_cus;

	/* Pointer for XCLBIN Manager */
	void				*xclbin_mngr;
};

struct zocl_xclbin_mngr {
	void				*private_adev;

	struct mutex			xclbin_lock; /* protect xclbins */
	struct list_head		xclbin_lists;

	/* Register necessery BO Ops here */
        struct zocl_xclbin_ops        *xbin_ops;
};

struct zocl_xclbin_ops {
        /**
         * @xclbin_download: Downloading a new XCLBIN to the requested slot.
         */
        int (*xclbin_download)(struct zocl_xclbin_mngr *xbin_handlr,
		       const void __user *xclbin_data, uint32_t slot_id);

	/**
         * @get_xclbin: Return the XCLBIN pointer from the requested slot.
         */
         void *(*get_xclbin)(struct zocl_xclbin_mngr *xbin_handlr,
			     uint32_t slot_id);

        /**
         * @release: Release existing XCLBIN. This will be donei before
         * download a new XCLBIN to an existing slot.
         */
        int (*xclbin_release)(struct zocl_xclbin_mngr *xbin_handlr,
					uint32_t slot_id);
};

#define xBIN_HANDLR(xbin_handlr)				\
        ((struct zocl_xclbin_mngr *)(xbin_handlr))
#define xBIN_OPS(xbin_handlr)					\
        ((struct zocl_xclbin_ops *)(xBIN_HANDLR(xbin_handlr))->xbin_ops)
#define xBIN_PARENT(xbin_handlr)				\
	((void *)(xBIN_HANDLR(xbin_handlr))->private_adev)
#define XBIN_DEVLOCK(xbin_handlr)			\
	mutex_lock(&((struct zocl_dev *)(xBIN_PARENT(xbin_handlr)))->dev_lock)
#define XBIN_DEVUNLOCK(xbin_handlr)			\
	mutex_unlock(&((struct zocl_dev *)(xBIN_PARENT(xbin_handlr)))->dev_lock)

static inline int
zocl_xclbin_download(struct zocl_xclbin_mngr *xbin_handlr,
		const void __user *xclbin_data, uint32_t slot_id)
{
        if(xBIN_OPS(xbin_handlr))
                return xBIN_OPS(xbin_handlr)->xclbin_download(xbin_handlr,
						xclbin_data, slot_id);

        return -EINVAL;
}

static inline void *
zocl_get_xclbin(struct zocl_xclbin_mngr *xbin_handlr, uint32_t slot_id)
{
        if(xBIN_OPS(xbin_handlr))
                return xBIN_OPS(xbin_handlr)->get_xclbin(xbin_handlr,
						slot_id);

        return NULL;
}

static inline int
zocl_xclbin_release(struct zocl_xclbin_mngr *xbin_handlr, uint32_t slot_id)
{
        if(xBIN_OPS(xbin_handlr))
                return xBIN_OPS(xbin_handlr)->xclbin_release(xbin_handlr,
							    slot_id);

        return -EINVAL;
}

int zocl_xclbin_register(struct zocl_xclbin_mngr **xbin_handlr,
			  void *parnt_adev);
int zocl_xclbin_unregister(struct zocl_xclbin_mngr *xbin_handlr);

#endif /* endif of _ZOCL_XCLBIN_H */
