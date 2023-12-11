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

#ifndef _ZOCL_SCHED_H_
#define _ZOCL_SCHED_H_

#include <drm/drm_print.h>
#include "zocl_drv.h"
#include "zocl_common.h"

enum scheduler_type {
	KDS_SCHED,
	DRM_SCHED
};

/* Default scheduler type */
extern int sched_type;

struct zocl_scheduler {
	/* Reference to the parent driver data structure */
	struct zocl_drm_dev     *private_zdev;

	/* Register necessery sched Ops here */
	struct zocl_sched_ops   *sched_ops;
};

struct zocl_sched_ops {
        /**
         * @init: Initialize ZOCL job scheduler here.
         */
        void (*init)(struct zocl_scheduler *sched_mngr);

	/**
         * @release: Release ZOCL job scheduler.
         */
	void (*release)(struct zocl_scheduler *sched_mngr);
};

int zocl_drm_sched_register(struct zocl_scheduler *sched_mngr,
			     void *parent_zdev);
int zocl_drm_sched_unregister(struct zocl_scheduler *sched_mngr);
int zocl_kds_sched_register(struct zocl_scheduler *sched_mngr,
			     void *parent_zdev);
int zocl_kds_sched_unregister(struct zocl_scheduler *sched_mngr);

static inline int zocl_sched_register(struct zocl_scheduler *sched_mngr,
				      void *parent_zdev)
{
	int ret = 0;

	if (sched_type == DRM_SCHED)
		ret = zocl_drm_sched_register(sched_mngr, parent_zdev);
	else
		ret = zocl_kds_sched_register(sched_mngr, parent_zdev);

	return ret;
}

static inline int zocl_sched_unregister(struct zocl_scheduler *sched_mngr)
{
	int ret = 0;

	if (sched_type == DRM_SCHED)
		ret = zocl_drm_sched_unregister(sched_mngr);
	else
		ret = zocl_kds_sched_unregister(sched_mngr);

	return ret;
}

#endif
