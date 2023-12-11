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

#ifndef _ZOCL_IOCTL_H
#define _ZOCL_IOCTL_H

#include <drm/drm.h>

#if defined(__cplusplus)
extern "C" {
#endif

enum zocl_drm_ioctl_id {
	/* Context */
	DRM_ZOCL_CREATE_HWCTX,
	DRM_ZOCL_DESTROY_HWCTX,

	/* BOs */
	DRM_ZOCL_CREATE_USERPTR_BO,
	DRM_ZOCL_CREATE_DEVICE_BO,
	DRM_ZOCL_GET_BO_INFO,
	DRM_ZOCL_SYNC_BO,

	/* Command Processing */
	DRM_ZOCL_EXEC_CMD,
	DRM_ZOCL_WAIT_CMD,

	/* This should be the last entry */
	DRM_ZOCL_NUM_IOCTLS
};

struct zocl_drm_create_hwctx {
	__u64	hwctx_xclbin_p;
	__u32	hwctx_handle;
	__u32	hwctx_pad;
};

struct zocl_drm_destroy_hwctx {
	__u32	hwctx_handle;
	__u32	hwctx_pad;
};

struct zocl_drm_exec_cmd {
	__u32	exec_bo_handle;
	__u32	exec_hwctx;
	__u64	exec_cmd_id;
};

struct zocl_drm_wait_cmd {
	__u32	exec_hwctx;
	__u32	exec_timeout;
	__u64	exec_cmd_id;
};

struct zocl_drm_create_userptr_bo {
	__u32	hwctx_handle;
	__u32	bo_handle;
};

struct zocl_drm_create_device_bo {
	__u32	hwctx_handle;
	__u32	bo_handle;
	__u32	pad;
};

struct zocl_drm_get_bo_info {
	__u32	bo_handle;
	__u32	pad;
};

struct zocl_drm_sync_bo {
	__u32	bo_handle;
	__u32	pad;
};

int zocl_drm_create_userptr_bo_ioctl(struct drm_device *dev, void *data,
				      struct drm_file *filp);
int zocl_drm_create_device_bo_ioctl(struct drm_device *dev, void *data,
				     struct drm_file *filp);
int zocl_drm_get_bo_info_ioctl(struct drm_device *dev, void *data,
				struct drm_file *filp);
int zocl_drm_sync_bo_ioctl(struct drm_device *dev, void *data,
			    struct drm_file *filp);

int zocl_drm_exec_cmd_ioctl(struct drm_device *dev, void *data,
			     struct drm_file *filp);

int zocl_drm_wait_cmd_ioctl(struct drm_device *dev, void *data,
			     struct drm_file *filp);

#define DRM_IOCTL_ZOCL_CREATE_HWCTX				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_ZOCL_CREATE_HWCTX,	\
		 struct zocl_drm_create_hwctx)

#define DRM_IOCTL_ZOCL_DESTROY_HWCTX				\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_ZOCL_DESTROY_HWCTX,	\
		 struct zocl_drm_destroy_hwctx)

#define DRM_IOCTL_ZOCL_EXEC_CMD					\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_ZOCL_EXEC_CMD,		\
		 struct zocl_drm_exec_cmd)

#define DRM_IOCTL_ZOCL_WAIT_CMD					\
	DRM_IOWR(DRM_COMMAND_BASE + DRM_ZOCL_WAIT_CMD,		\
		 struct zocl_drm_wait_cmd)

#define DRM_IOCTL_ZOCL_CREATE_USERPTR_BO \
	DRM_IOR(DRM_COMMAND_BASE + DRM_ZOCL_CREATE_USERPTR_BO, \
		struct zocl_drm_create_userptr_bo)

#define DRM_IOCTL_ZOCL_CREATE_DEVICE_BO \
	DRM_IOR(DRM_COMMAND_BASE + DRM_ZOCL_CREATE_DEVICE_BO,\
		struct zocl_drm_create_device_bo)

#define DRM_IOCTL_ZOCL_GET_BO_INFO \
	DRM_IOR(DRM_COMMAND_BASE + DRM_ZOCL_GET_BO_INFO,\
		struct zocl_drm_get_bo_info)

#define DRM_IOCTL_ZOCL_SYNC_BO \
	DRM_IOR(DRM_COMMAND_BASE + DRM_ZOCL_SYNC_BO,\
		struct zocl_drm_sync_bo)

#if defined(__cplusplus)
} /* end of "extern "C" {" */
#endif

#endif /* endif of _ZOCL_DRV_H */
