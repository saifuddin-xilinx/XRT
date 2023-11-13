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

#include "zocl_drv.h"

enum scheduler_type {
	KDS_SCHED,
	DRM_SCHED
};
extern int sched_type;

struct zocl_sched_ops {
        /**
         * @init: Initialize ZOCL job scheduler here.
         */
        void (*init)(struct zocl_drm_dev *zdev, void *sched_handlr);

	/**
         * @release: Release ZOCL job scheduler.
         */
	void (*release)(struct zocl_drm_dev *zdev, void *sched_handlr);
};

int zocl_drm_sched_register(struct zocl_drm_dev *zdev, void *sched_handlr);
int zocl_drm_sched_unregister(void *sched_handlr);
int zocl_kds_sched_register(struct zocl_drm_dev *zdev, void *sched_handlr);
int zocl_kds_sched_unregister(void *sched_handlr);

static inline int zocl_sched_register(struct zocl_drm_dev *zdev,
				      void *sched_handlr)
{
	int ret = 0;

	if (sched_type == DRM_SCHED)
		ret = zocl_drm_sched_register(zdev, sched_handlr);
	else
		ret = zocl_kds_sched_register(zdev, sched_handlr);

	return ret;
}

static inline int zocl_sched_unregister(void *sched_handlr)
{
	int ret = 0;

	if (sched_type == DRM_SCHED)
		ret = zocl_drm_sched_unregister(sched_handlr);
	else
		ret = zocl_kds_sched_unregister(sched_handlr);

	return ret;
}

#endif
