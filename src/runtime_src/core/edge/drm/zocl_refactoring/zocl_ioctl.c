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

#include <drm/drm.h>
#include "zocl_common.h"
#include "zocl_ioctl.h"

int zocl_drm_create_userptr_bo_ioctl(struct drm_device *dev, void *data,
				 struct drm_file *filp)
{
	struct zocl_dev *zdev = to_zdev(dev);
	int ret = 0;
	uint64_t bo_size = 0;

	ZDEV_INFO(zdev, "done");
	zocl_mm_create_userptr_bo(zDRV_MEMHNDLR(zdev), bo_size);
	return ret;
}

int zocl_drm_create_device_bo_ioctl(struct drm_device *dev, void *data,
				 struct drm_file *filp)
{
	struct zocl_dev *zdev = to_zdev(dev);
	int ret = 0;
	uint64_t bo_size = 0;
	uint32_t bo_flags = 0;

	ZDEV_INFO(zdev, "done");
	zocl_mm_create_device_bo(zDRV_MEMHNDLR(zdev), filp, bo_size, bo_flags);
	return ret;
}

int zocl_drm_get_bo_info_ioctl(struct drm_device *dev, void *data,
				 struct drm_file *filp)
{
	struct zocl_dev *zdev = to_zdev(dev);
	int ret = 0;
	uint32_t bo_handle = 0;

	ZDEV_INFO(zdev, "done");
	zocl_mm_get_info_bo(zDRV_MEMHNDLR(zdev), bo_handle);
	return ret;
}

int zocl_drm_sync_bo_ioctl(struct drm_device *dev, void *data,
				 struct drm_file *filp)
{
	struct zocl_dev *zdev = to_zdev(dev);
	int ret = 0;
	uint32_t bo_handle = 0;

	ZDEV_INFO(zdev, "done");
	zocl_mm_sync_bo(zDRV_MEMHNDLR(zdev), bo_handle);
	return ret;
}
