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
 *    Min Ma       <min.ma@xilinx.com>
 *    Jan Stephan  <j.stephan@hzdr.de>
 *
 * This file is dual-licensed; you may select either the GNU General Public
 * License version 2 or Apache License, Version 2.0.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include "zocl_drv.h"
#include "zocl_common.h"

/* Driver Debug Macros */
#define ZDRM2DEV(zdev)                 (&ZDEV2PDEV(zdev)->dev)
#define zdev_err(zdev, fmt, args...)   zocl_err(ZDRM2DEV(zdev), fmt"\n", ##args)
#define zdev_warn(zdev, fmt, args...)  zocl_warn(ZDRM2DEV(zdev), fmt"\n", ##args)
#define zdev_info(zdev, fmt, args...)  zocl_info(ZDRM2DEV(zdev), fmt"\n", ##args)
#define zdev_dbg(zdev, fmt, args...)   zocl_dbg(ZDRM2DEV(zdev), fmt"\n", ##args)

int kds_echo = 0;

#define ZOCL_DRIVER_NAME        "zocl"

#define ZOCL_DRIVER_DESC        "Zynq BO manager"

static char driver_date[9];

/* This should be the same as DRM_FILE_PAGE_OFFSET_START in drm_gem.c */
#if defined(CONFIG_ARM64)
#define ZOCL_FILE_PAGE_OFFSET   0x00100000
#else
#define ZOCL_FILE_PAGE_OFFSET   0x00010000
#endif

#ifndef VM_RESERVED
#define VM_RESERVED (VM_DONTEXPAND | VM_DONTDUMP)
#endif

int enable_xgq_ert = 1;
module_param(enable_xgq_ert, int, (S_IRUGO|S_IWUSR));
MODULE_PARM_DESC(enable_xgq_ert, "0 = legacy ERT mode, 1 = XGQ ERT mode (default)");

#define ZVERSAL_DRIVER_NAME        "zocl-drm-versal"

static const struct zdev_data zdev_data_mpsoc = {
        .fpga_driver_name = "pcap",
        .fpga_driver_new_name = "pcap"
};

static const struct zdev_data zdev_data_versal = {
        .fpga_driver_name = "versal_fpga",
        .fpga_driver_new_name = "versal-fpga"
};

static const struct of_device_id zocl_drm_of_match[] = {
        { .compatible = "xlnx,zocl", .data = &zdev_data_mpsoc},
        { .compatible = "xlnx,zoclsvm", .data = &zdev_data_mpsoc},
        { .compatible = "xlnx,zocl-ert", .data = &zdev_data_mpsoc},
        { .compatible = "xlnx,zocl-versal", .data = &zdev_data_versal},
        { /* end of table */ },
};
MODULE_DEVICE_TABLE(of, zocl_drm_of_match);

/*
 *
 * Initialization of Xilinx openCL DRM platform device.
 *
 * @param        pdev: Platform Device Instance
 *
 * @return       0 on success, Error code on failure.
 *
 */
static int zocl_drm_platform_probe(struct platform_device *pdev)
{
	struct zocl_drm_dev		*zdev = NULL;
	const struct of_device_id	*id = NULL;

	id = of_match_node(zocl_drm_of_match, pdev->dev.of_node);
	if (!id)
		return -EINVAL;

	/* Create zocl device and initial */
	zdev = devm_kzalloc(&pdev->dev, sizeof(*zdev), GFP_KERNEL);
	if (!zdev)
		return -ENOMEM;

	zdev->pdev = pdev;
	platform_set_drvdata(pdev, zdev);

	/* Initialize the component platform devices list head */
	mutex_init(&zdev->dev_list_lock);
	INIT_LIST_HEAD(&zdev->zxgq_dev_list_head);
	INIT_LIST_HEAD(&zdev->zirq_dev_list_head);
	INIT_LIST_HEAD(&zdev->zcsr_dev_list_head);

        zdev_info(zdev, "Platform device Probed");
        return 0;
}

/*
 *
 * Exit Xilinx openCL DRM platform device.
 *
 * @param        pdev: Platform Device Instance
 *
 * @return       0 on success, Error code on failure.
 *
 */
static int zocl_drm_platform_remove(struct platform_device *pdev)
{
	struct zocl_drm_dev *zdev = platform_get_drvdata(pdev);

	if (!zdev)
		return -EINVAL;


        zdev_info(zdev, "Platform device Removed");
        return 0;
}

struct platform_driver zocl_drm_private_driver = {
        .probe                  = zocl_drm_platform_probe,
        .remove                 = zocl_drm_platform_remove,

        .driver                 = {
                .name           = ZVERSAL_DRIVER_NAME,
                .of_match_table = zocl_drm_of_match,
        },
};

static struct platform_driver *zocl_component_platform_drivers[] = {
	&zocl_cu_driver,
	&zocl_scu_driver,
	&zocl_csr_intc_driver,
	&zocl_irq_intc_driver,
	&zocl_ospi_versal_driver,
	&zocl_xgq_driver,
	&zocl_ert_driver,
	&zocl_rpu_channel_driver,
};

static int __init zocl_init(void)
{
	/* Register the parent versal driver zyxclmm_drm */
	if (platform_driver_register(&zocl_drm_private_driver))
		return -ENODEV;

	/* Now register the other component platform drivers */
	return platform_register_drivers(zocl_component_platform_drivers,
				ARRAY_SIZE(zocl_component_platform_drivers));
}
module_init(zocl_init);

static void __exit zocl_exit(void)
{
	platform_unregister_drivers(zocl_component_platform_drivers,
			     ARRAY_SIZE(zocl_component_platform_drivers));

	platform_driver_unregister(&zocl_drm_private_driver);
}
module_exit(zocl_exit);

MODULE_VERSION(XRT_DRIVER_VERSION);

MODULE_DESCRIPTION(ZOCL_DRIVER_DESC);
MODULE_AUTHOR("Sonal Santan <sonal.santan@xilinx.com>");
MODULE_LICENSE("GPL");
