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

#include <linux/fpga/fpga-mgr.h>
#include <linux/of.h>

#include <linux/pagemap.h>
#include <linux/of_address.h>
#include <linux/iommu.h>
#include <linux/version.h>
#include <drm/drm_print.h>

#include "zocl_common.h"
#include "zocl_mem.h"

#include "zocl_xclbin.h"

/* XCLBIN manager Debug Macros */
#define XHNDLR2DEV(xhandlr)			aDEV_HANDLR(xBIN_PARENT(xhandlr))
#define XBIN_ERR(xhandlr, fmt, args...)		ADEV_ERR(XHNDLR2DEV(xhandlr), fmt"\n", ##args)
#define XBIN_WARN(xhandlr, fmt, args...)	ADEV_WARN(XHNDLR2DEV(xhandlr), fmt"\n", ##args)
#define XBIN_INFO(xhandlr, fmt, args...)	ADEV_INFO(XHNDLR2DEV(xhandlr), fmt"\n", ##args)
#define XBIN_DBG(xhandlr, fmt, args...)		ADEV_DBG(XHNDLR2DEV(xhandlr), fmt"\n", ##args)

static enum pdi_type
convert_to_pdi_type(enum CDO_TYPE type)
{
	enum pdi_type ret;

	/* TODO: Need to add Pre/Post type of CDOs. XCLBIN is not ready */
	switch (type) {
	case CT_PRIMARY:
		ret = PDI_TYPE_PRIMARY;
		break;
	case CT_LITE:
		ret = PDI_TYPE_LITE;
		break;
	default:
		ret = MAX_PDI_TYPE;
	}

	return ret;
}

static const struct axlf_section_header *
xclbin_get_section_hdr_next(const struct axlf *axlf, enum axlf_section_kind kind,
			    const struct axlf_section_header *curr)
{
	int match = false;
	int i;

	for (i = 0; i < axlf->header.num_sections; i++) {
		if (axlf->sections[i].section_kind != kind)
			continue;

		if (!curr || match)
			return &axlf->sections[i];

		if (&axlf->sections[i] == curr)
			match = true;
	}

	return NULL;
}

static const struct axlf_section_header *
xclbin_get_section_hdr(const struct axlf *axlf, enum axlf_section_kind kind)
{
	return xclbin_get_section_hdr_next(axlf, kind, NULL);
}

static inline u64 xclbin_get_length(const struct axlf *axlf)
{
	return axlf->header.length;
}

static inline const uuid_t *xclbin_get_uuid(const struct axlf *axlf)
{
	return (uuid_t *)axlf->header.uuid;
}

/* TODO */
static int zocl_validate_axlf_sections(struct zocl_xclbin_mngr *xclbin_handlr,
					const struct axlf *axlf)
{
	return 0;
}

static int
zocl_validate_axlf(struct zocl_xclbin_mngr *xclbin_handlr,
		    const struct axlf *axlf)
{
	if (zocl_validate_axlf_sections(xclbin_handlr, axlf)) {
		XBIN_ERR(xclbin_handlr, "Invalid XCLBIN requested");
		return -EINVAL;
	}

	XBIN_INFO(xclbin_handlr, "Requesting to Download Xclbin uuid %pUb\n",
	                    xclbin_get_uuid(axlf));
	return 0;
}

bool zocl_cross_boundary(struct zocl_xclbin_mngr *xclbin_handlr, u64 start,
			  u32 size, u32 order)
{
	u32 mask = ~((1 << order) - 1);
	u64 end = start + size - 1;

	if ((start & mask) != (end & mask)) {
		XBIN_WARN(xclbin_handlr,
			  "Range [0x%llx, 0x%llx] cross 0x%x bytes boundary",
			  start, end, 1 << order);
		return true;
	}

	return false;
}

static void zocl_free_pdi_no_iommu(struct zocl_xclbin_mngr *xclbin_handlr,
				    void *orig)
{
	kfree(orig);
}

static void *zocl_alloc_pdi_no_iommu(struct zocl_xclbin_mngr *xclbin_handlr,
				      u32 size, dma_addr_t *addr, void **orig)
{
	void *vaddr;
	dma_addr_t paddr;

	vaddr = kzalloc(size * 2, GFP_KERNEL);
	if (!vaddr) {
		XBIN_ERR(xclbin_handlr, "Failed to allocate PDI buffer");
		return NULL;
	}

	paddr = (dma_addr_t)virt_to_phys(vaddr);

	*orig = vaddr;
	*addr = paddr;

	return vaddr;
}

static inline void
zocl_pdi_cleanup(struct zocl_xclbin_mngr *xclbin_handlr,
		  struct zocl_pdi *pdis, int num_pdis)
{
	int i;

	for (i = 0; i < num_pdis; i++) {
		if (!pdis[i].pdi_image)
			continue;

		zocl_free_pdi_no_iommu(xclbin_handlr, pdis[i].pdi_orig);
	}
}

static int
zocl_pdi_parse(struct zocl_xclbin_mngr *xclbin_handlr, struct zocl_pdi *pdis,
		 int num_pdis, const struct aie_partition *aie_part)
{
	const struct aie_pdi *pdi_array;
	int ret = 0;
	int i;

	pdi_array = get_array(aie_part, &aie_part->aie_pdis);
	for (i = 0; i < num_pdis; i++) {
		const struct cdo_group *cdo_array;
		const u8 *pdi_image;
		dma_addr_t pdi_dev_addr;
		u8 *pdi_cpu_addr;
		u32 size;

		pdi_image = get_array(aie_part, &pdi_array[i].pdi_image);
		cdo_array = get_array(aie_part, &pdi_array[i].cdo_groups);
		size = pdi_array[i].pdi_image.size;

		pdi_cpu_addr = zocl_alloc_pdi_no_iommu(xclbin_handlr, size,
							 &pdi_dev_addr,
							 &pdis[i].pdi_orig);

		if (!pdi_cpu_addr) {
			ret = -ENOMEM;
			goto cleanup_pdi_blobs;
		}
		memcpy(pdi_cpu_addr, pdi_image, size);

		/*
		 * This check is not for upstreaming!
		 * The IPU FW required that the PDI image not crossing 64MB
		 * boundary. The dma_map_* API will call iommu map callback. It
		 * will allocate iova range, which will be size alignment.
		 * It is easy to prove that if PDI size is less than 64MB, the
		 * size alignment implementaion will always satisfy the boundary
		 * requirement.
		 *
		 * Check here to warn if the iommu map behavior change.
		 */
		if (zocl_cross_boundary(xclbin_handlr, pdi_dev_addr, size, 26))
			WARN_ON(1);

		pdis[i].pdi_image = pdi_cpu_addr;
		pdis[i].pdi_addr = pdi_dev_addr;
		pdis[i].pdi_size = size;
		/* Only support 1 element in cdo_groups for now */
		pdis[i].pdi_type = convert_to_pdi_type(cdo_array[0].cdo_type);
		pdis[i].pdi_id = -1;
		pdis[i].pdi_num_dpu_ids = cdo_array[0].dpu_kernel_ids.size;
		pdis[i].pdi_dpu_ids =
			get_array(aie_part, &cdo_array[0].dpu_kernel_ids);
		uuid_copy(&pdis[i].pdi_uuid, (uuid_t *)pdi_array[i].uuid);
		XBIN_WARN(xclbin_handlr, "cache pdi id %d size %d", i, size);
	}

	return 0;

cleanup_pdi_blobs:
	zocl_pdi_cleanup(xclbin_handlr, pdis, num_pdis);
	return ret;
}

static void *
zocl_xclbin_get(struct zocl_xclbin_mngr *xclbin_handlr, uint32_t slot_id)
{
	struct zocl_xclbin *cache = NULL;
	struct zocl_xclbin *curr = NULL;

	mutex_lock(&xclbin_handlr->xclbin_lock);
	list_for_each_entry(curr, &xclbin_handlr->xclbin_lists, xclbin_entry) {
		if (slot_id == curr->slot_id) {
			cache = curr;
			break;
		}
	}

	if (cache && !kref_get_unless_zero(&cache->xclbin_ref))
		cache = NULL;

	mutex_unlock(&xclbin_handlr->xclbin_lock);

	if (cache)
		XBIN_INFO(xclbin_handlr, "get xclbin, uuid %pUb of slot %d",
			  &cache->xclbin_uuid, slot_id);

	return cache;
}

static int
zocl_xclbin_parse_memtopology(struct zocl_xclbin_mngr *xclbin_handlr,
			const struct axlf *axlf, struct zocl_xclbin *cache)
{
	const struct axlf_section_header *hdr;

	hdr = xclbin_get_section_hdr(axlf, MEM_TOPOLOGY);
	if (!hdr) {
		XBIN_ERR(xclbin_handlr, "MEM_TOPOLOGY section not found");
		return -ENODATA;
	}

	/* Store the memory topology section */
	cache->xclbin_memtopo = get_section(axlf, hdr);

	return 0;
}

static int
zocl_xclbin_parse_iplayout(struct zocl_xclbin_mngr *xclbin_handlr,
			    const struct axlf *axlf, struct zocl_xclbin *cache)
{
	const struct axlf_section_header *hdr;
	const struct ip_layout *ips;
	struct zocl_cu *cu;
	int i, cu_index = 0;

	hdr = xclbin_get_section_hdr(axlf, IP_LAYOUT);
	if (!hdr) {
		XBIN_ERR(xclbin_handlr, "IP_LAYOUT section not found");
		return -ENODATA;
	}
	ips = get_section(axlf, hdr);

	cu = kcalloc(ips->count, sizeof(*cu), GFP_KERNEL);
	if (!cu)
		return -ENOMEM;

	for (i = 0; i < ips->count; i++) {
		const struct ip_data *ip;

		ip = &ips->ip_data[i];
		if (ip->type != IP_PS_KERNEL)
			continue;

		if (ip->sub_type != ST_DPU)
			continue;

		strncpy(cu[cu_index].cu_name, ip->name,
			sizeof(cu[cu_index].cu_name));
		cu[cu_index].cu_func = ip->functional;
		cu[cu_index].cu_dpu_id = ip->dpu_kernel_id;
		cu[cu_index].cu_index = cu_index;
		cu_index++;
	}
	cache->xclbin_num_cus = cu_index;
	cache->xclbin_cu = cu;

	return 0;
}

static int
zocl_xclbin_parse_aie(struct zocl_xclbin_mngr *xclbin_handlr,
		       const struct axlf *axlf, struct zocl_xclbin *cache)
{
	const struct axlf_section_header *hdr;
	struct zocl_partition *zdev_part;
	const struct aie_partition *aie_part;
	struct zocl_pdi *pdis;
	size_t start_cols_bytes;
	u16 *part_start_cols;
	int ret;

	hdr = xclbin_get_section_hdr(axlf, AIE_PARTITION);
	if (unlikely(!hdr)) {
		XBIN_ERR(xclbin_handlr, "AIE_PARTITION not found, data corrupted?");
		return -EINVAL;
	}

	zdev_part = &cache->xclbin_zdev_part;

	aie_part = get_section(axlf, hdr);
	pdis = kcalloc(aie_part->aie_pdis.size, sizeof(*pdis), GFP_KERNEL);
	if (!pdis) {
		XBIN_ERR(xclbin_handlr, "No memory for PDIs");
		return -ENOMEM;
	}

	ret = zocl_pdi_parse(xclbin_handlr, pdis,
			      aie_part->aie_pdis.size, aie_part);
	if (ret) {
		XBIN_ERR(xclbin_handlr, "PDI parse failed");
		goto free_pdis;
	}

	zdev_part->part_pdis = pdis;
	zdev_part->part_num_pdis = aie_part->aie_pdis.size;
	zdev_part->part_ncols = aie_part->info.column_width;
	zdev_part->part_ops = aie_part->operations_per_cycle;

	zdev_part->part_nparts = aie_part->info.start_columns.size;
	zdev_part->part_start_cols =
		get_array(aie_part, &aie_part->info.start_columns);
	start_cols_bytes = aie_part->info.start_columns.size * sizeof(16);

	part_start_cols = kmalloc(start_cols_bytes, GFP_KERNEL);
	if (!part_start_cols) {
		ret = -ENOMEM;
		goto free_parsed_pdi;
	}

	memcpy(part_start_cols,
	       get_array(aie_part, &aie_part->info.start_columns),
						 start_cols_bytes);

	zdev_part->part_start_cols = part_start_cols;

	return 0;

free_parsed_pdi:
	zocl_pdi_cleanup(xclbin_handlr, pdis, aie_part->aie_pdis.size);
free_pdis:
	kfree(pdis);
	return ret;
}

static int
zocl_xclbin_parse(struct zocl_xclbin_mngr *xclbin_handlr,
		   const struct axlf *axlf, struct zocl_xclbin *cache)
{
	int err;

	err = zocl_xclbin_parse_memtopology(xclbin_handlr, axlf, cache);
	if (err) {
		XBIN_ERR(xclbin_handlr,
			 "parse MEM_TOPOLOGY section failed, err %d", err);
		return err;
	}

	err = zocl_xclbin_parse_iplayout(xclbin_handlr, axlf, cache);
	if (err) {
		XBIN_ERR(xclbin_handlr,
			 "parse IP_LAYOUT section failed, err %d", err);
		return err;
	}

	err = zocl_xclbin_parse_aie(xclbin_handlr, axlf, cache);
	if (err) {
		XBIN_ERR(xclbin_handlr, "parse AIE section failed, err %d", err);
		return err;
	}

	return 0;
}

static void
zocl_xclbin_mem_free(struct zocl_xclbin_mngr *xclbin_handlr,
		      struct zocl_xclbin *cache)
{
	struct zocl_partition *part;

	if (!cache)
		return;

	kfree(cache->xclbin_cu);
	part = &cache->xclbin_zdev_part;
	zocl_pdi_cleanup(xclbin_handlr, part->part_pdis, part->part_num_pdis);
	kfree(part->part_pdis);
	kfree(cache);
}

/*
 * This function is the main entry point to load xclbin. It's takes an userspace
 * pointer of xclbin and copy the xclbin data to kernel space. Then load that
 * xclbin to the FPGA. It also initialize other modules like, memory, aie, CUs
 * etc.
 *
 * @param       xclbin_handlr:	XCLBIN manager Handler
 * @param:	xclbin		xclbin userspace structure
 * @param       slot_id:        targeted slot id for this sclbin
 *
 * @return      0 on success, Error code on failure.
 */
static int zocl_download_xclbin(struct zocl_xclbin_mngr *xclbin_handlr,
			const void __user *xclbin_data, uint32_t slot_id)
{
	void *parent_zdev = xBIN_PARENT(xclbin_handlr);
	struct zocl_xclbin *cache = NULL;
	struct axlf axlf_bin;
	void *blob = NULL;
	u64 xclbin_size = 0;
	int ret = 0;

	if (copy_from_user(&axlf_bin, xclbin_data, sizeof(struct axlf)))
		return -EFAULT;

	ret = zocl_validate_axlf(xclbin_handlr, &axlf_bin);
	if (ret)
		return ret;

	XBIN_DEVLOCK(xclbin_handlr);
#if 0
	cache = zocl_xclbin_get(xclbin_handlr, slot_id);
	if (cache) {
		XBIN_INFO(xclbin_handlr, "xclbin exists, dup register request");
		goto done;
	}
#endif
	cache = kzalloc(sizeof(*cache), GFP_KERNEL);
	if (!cache) {
		ret = -ENOMEM;
		goto err;
	}

	xclbin_size = xclbin_get_length(&axlf_bin);
	blob = vmalloc(xclbin_size);
	if (!blob) {
		ret = -ENOMEM;
		goto err;
	}

	ret = copy_from_user(blob, xclbin_data, xclbin_size);
	if (ret) {
		XBIN_ERR(xclbin_handlr, "copy from user failed for xclbin %d", ret);
		vfree(blob);
		ret = -EFAULT;
		goto err;
	}

	if (unlikely(memcmp(&axlf_bin, blob, sizeof(axlf_bin)))) {
		XBIN_ERR(xclbin_handlr, "xclbin size was changed in user space");
		ret = -EINVAL;
		goto err;
	}

	uuid_copy(&cache->xclbin_uuid, xclbin_get_uuid(&axlf_bin));

	ret = zocl_xclbin_parse(xclbin_handlr, (struct axlf *)blob, cache);
	if (ret)
		goto err;

	kref_init(&cache->xclbin_ref);
	cache->slot_id = slot_id;

	mutex_lock(&xclbin_handlr->xclbin_lock);
	list_add_tail(&cache->xclbin_entry, &xclbin_handlr->xclbin_lists);
	mutex_unlock(&xclbin_handlr->xclbin_lock);

	/* Now Initialize the memory manager here */
	ret = zocl_mm_init(aDRV_MEMHNDLR(parent_zdev), cache->xclbin_memtopo);
	if (ret) {
		XBIN_ERR(xclbin_handlr, "Memory Initialization failed");
		goto err;
	}
#if 0
done:
#endif
	/* Store the XCLBIN manager pointer for future reference */
	cache->xclbin_mngr = xclbin_handlr;
	XBIN_DEVUNLOCK(xclbin_handlr);
	XBIN_INFO(xclbin_handlr, "Download Xclbin uuid %pUb to slor %d finished successfully",
		  &cache->xclbin_uuid, slot_id);
	return 0;
err:
	XBIN_DEVUNLOCK(xclbin_handlr);
	if (cache)
		zocl_xclbin_mem_free(xclbin_handlr, cache);

	return ret;
}

static void
__zocl_release_xclbin(struct kref *ref)
{
	struct zocl_xclbin *cache = NULL;
	struct zocl_xclbin_mngr *xclbin_handlr = NULL;

	cache = container_of(ref, struct zocl_xclbin, xclbin_ref);
	xclbin_handlr = cache->xclbin_mngr;

	XBIN_DBG(xclbin_handlr, "releasing XCLBIN, UUID(%pUb)", &cache->xclbin_uuid);

	mutex_lock(&xclbin_handlr->xclbin_lock);
	list_del(&cache->xclbin_entry);
	mutex_unlock(&xclbin_handlr->xclbin_lock);

	zocl_xclbin_mem_free(xclbin_handlr, cache);
}

static void zocl_xclbin_put(struct zocl_xclbin_mngr *xclbin_handlr,
			     struct zocl_xclbin *cache)
{
	XBIN_DBG(xclbin_handlr, "put XCLBIN, UUID(%pUb)", &cache->xclbin_uuid);
	XBIN_DEVLOCK(xclbin_handlr);
	kref_put(&cache->xclbin_ref, __zocl_release_xclbin);
	XBIN_DEVUNLOCK(xclbin_handlr);
}

static int zocl_release_xclbin(struct zocl_xclbin_mngr *xclbin_handlr,
				uint32_t slot_id)
{
	struct zocl_xclbin *cache = zocl_xclbin_get(xclbin_handlr, slot_id);

	if (cache)
		zocl_xclbin_put(xclbin_handlr, cache);

	return 0;
}


/*
 * XCLBIN Interfaces exposed to other part of the drivers.
 */
static struct zocl_xclbin_ops xclbin_ops = {
        .xclbin_download = zocl_download_xclbin,
        .get_xclbin = zocl_xclbin_get,
        .xclbin_release = zocl_release_xclbin,
};

int zocl_xclbin_register(struct zocl_xclbin_mngr **xclbin_handlr,
			  void *prv_zdev)
{
	struct zocl_xclbin_mngr *xbin_handlr = NULL;

	xbin_handlr = vzalloc(sizeof(struct zocl_xclbin_mngr));
	if (!xbin_handlr)
		return -ENOMEM;

	xbin_handlr->xbin_ops = &xclbin_ops;

	INIT_LIST_HEAD(&xbin_handlr->xclbin_lists);

	/* Store the XCLBIN handler here for future references */
	xbin_handlr->private_zdev = prv_zdev;
	*xclbin_handlr = xbin_handlr;

	return 0;
}

int zocl_xclbin_unregister(struct zocl_xclbin_mngr *xclbin_handlr)
{
	if (!xclbin_handlr)
		return -EINVAL;

	/* Free the existing XCLBIN handler here */
	vfree(xclbin_handlr);
	return 0;
}
