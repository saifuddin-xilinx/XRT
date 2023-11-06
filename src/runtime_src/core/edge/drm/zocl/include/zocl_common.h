/* SPDX-License-Identifier: GPL-2.0 OR Apache-2.0 */
/*
 * A GEM style (optionally CMA backed) device manager for ZynQ based
 * OpenCL accelerators.
 *
 * Copyright (C) 2016-2022 Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Authors:
 *    Sonal Santan <sonal.santan@xilinx.com>
 *    Umang Parekh <umang.parekh@xilinx.com>
 *    Jan Stephan  <j.stephan@hzdr.de>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#ifndef _ZOCL_COMMON_H_
#define _ZOCL_COMMON_H_
#include <drm/drm.h>
#include <drm/drm_device.h>
#include <drm/drm_drv.h>
#include <drm/drm_gem.h>
#include <drm/drm_mm.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
#include <drm/drm_gem_dma_helper.h>
#else
#include <drm/drm_gem_cma_helper.h>
#endif
#include <linux/poll.h>

#if defined(CONFIG_ARM64)
#define ZOCL_PLATFORM_ARM64   1
#else
#define ZOCL_PLATFORM_ARM64   0
#endif

#ifndef XRT_DRIVER_VERSION
#define XRT_DRIVER_VERSION ""
#endif

#ifndef XRT_HASH
#define XRT_HASH ""
#endif

#ifndef XRT_HASH_DATE
#define XRT_HASH_DATE ""
#endif

/* Ensure compatibility with newer kernels and backported Red Hat kernels. */
/* The y2k38 bug fix was introduced with Kernel 3.17 and backported to Red Hat
 * 7.2.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
	#define ZOCL_TIMESPEC struct timespec64
	#define ZOCL_GETTIME ktime_get_real_ts64
	#define ZOCL_USEC tv_nsec / NSEC_PER_USEC
#elif defined(RHEL_RELEASE_CODE)
	#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7, 2)
		#define ZOCL_TIMESPEC struct timespec64
		#define ZOCL_GETTIME ktime_get_real_ts64
		#define ZOCL_USEC tv_nsec / NSEC_PER_USEC
	#else
		#define ZOCL_TIMESPEC struct timeval
		#define ZOCL_GETTIME do_gettimeofday
		#define ZOCL_USEC tv_usec
	#endif
#else
	#define ZOCL_TIMESPEC struct timeval
	#define ZOCL_GETTIME do_gettimeofday
	#define ZOCL_USEC tv_usec
#endif

/* drm_gem_object_put_unlocked was introduced with Kernel 4.12 and backported to
 * Red Hat 7.5
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
	#define ZOCL_DRM_GEM_OBJECT_PUT_UNLOCKED drm_gem_object_put
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
	#define ZOCL_DRM_GEM_OBJECT_PUT_UNLOCKED drm_gem_object_put_unlocked
#elif defined(RHEL_RELEASE_CODE)
	#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7, 5)
		#define ZOCL_DRM_GEM_OBJECT_PUT_UNLOCKED \
			drm_gem_object_put_unlocked
	#else
		#define ZOCL_DRM_GEM_OBJECT_PUT_UNLOCKED \
			drm_gem_object_unreference_unlocked
	#endif
#else
	#define ZOCL_DRM_GEM_OBJECT_PUT_UNLOCKED \
		drm_gem_object_unreference_unlocked
#endif

/* drm_dev_put was introduced with Kernel 4.15 and backported to Red Hat 7.6. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
	#define ZOCL_DRM_DEV_PUT drm_dev_put
#elif defined(RHEL_RELEASE_CODE)
	#if RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(7, 6)
		#define ZOCL_DRM_DEV_PUT drm_dev_put
	#else
		#define ZOCL_DRM_DEV_PUT drm_dev_unref
	#endif
#else
	#define ZOCL_DRM_DEV_PUT drm_dev_unref
#endif

/* access_ok lost its first parameter with Linux 5.0. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
	#define ZOCL_ACCESS_OK(TYPE, ADDR, SIZE) access_ok(ADDR, SIZE)
#else
	#define ZOCL_ACCESS_OK(TYPE, ADDR, SIZE) access_ok(TYPE, ADDR, SIZE)
#endif

#define ZDEV2PDEV(dev)                 ((dev)->pdev)

#define zocl_err(dev, fmt, args...)     dev_err(dev, "%s: "fmt, __func__, ##args)
#define zocl_warn(dev, fmt, args...)    dev_warn(dev, "%s: "fmt, __func__, ##args)
#define zocl_info(dev, fmt, args...)    dev_info(dev, "%s: "fmt, __func__, ##args)
#define zocl_dbg(dev, fmt, args...)     dev_dbg(dev, "%s: "fmt, __func__, ##args)

/* Sub device driver */
extern struct platform_driver zocl_cu_xgq_driver;
extern struct platform_driver zocl_csr_intc_driver;
extern struct platform_driver zocl_irq_intc_driver;
extern struct platform_driver zocl_rpu_channel_driver;
extern struct platform_driver zocl_xgq_ert_driver;
extern struct platform_driver zocl_cu_driver;
extern struct platform_driver zocl_scu_driver;
extern struct platform_driver zocl_ospi_versal_driver;
extern struct platform_driver zocl_ert_driver;
#endif
