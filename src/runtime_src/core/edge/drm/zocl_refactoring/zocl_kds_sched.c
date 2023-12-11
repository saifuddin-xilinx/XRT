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

#include "zocl_sched.h"

#define zsched_err(fmt, args...)	DRM_ERROR(fmt"\n", ##args)
#define zsched_warn(fmt, args...)	DRM_WARN(fmt"\n", ##args)
#define zsched_info(fmt, args...)	DRM_INFO(fmt"\n", ##args)
#define zsched_dbg(fmt, args...)	DRM_DBG(fmt"\n", ##args)

static void zocl_kds_sched_init(struct zocl_scheduler *sched_mngr)
{
	/* FIXME */
}

static void zocl_kds_sched_release(struct zocl_scheduler *sched_mngr)
{
	/* FIXME */
}

/*
 * Scheduler Interfaces exposed to other part of the drivers.
 */
static struct zocl_sched_ops zocl_kds_sched_ops = {
        .init = zocl_kds_sched_init,
	.release = zocl_kds_sched_release,
};

int zocl_kds_sched_register(struct zocl_scheduler *sched_mngr,
			     void *parent_zdev)
{
	struct zocl_scheduler *a_sched = NULL;

	a_sched = vzalloc(sizeof(struct zocl_scheduler));
	if (!a_sched)
		return -ENOMEM;

	a_sched->sched_ops = &zocl_kds_sched_ops;

	/* Store the sched manager reference here for future references */
	a_sched->private_zdev = parent_zdev;
	sched_mngr = a_sched;

	return 0;
}

int zocl_kds_sched_unregister(struct zocl_scheduler *sched_mngr)
{
	if (!sched_mngr) {
		zsched_err("Invalid scheduler manager requested for unregister");
		return -EINVAL;
	}

	/* Free the existing sched manager here */
	vfree(sched_mngr);
	return 0;
}
