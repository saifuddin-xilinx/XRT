/**
 * Copyright (C) 2019-2020 Xilinx, Inc
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

// ------ I N C L U D E   F I L E S -------------------------------------------
// Local - Include Files
#include "XBMemAccess.h"
#include "core/common/error.h"
#include "core/common/utils.h"
#include "core/common/message.h"
#include "core/common/query_requests.h"

#include "common/system.h"

// 3rd Party Library - Include Files
#include <boost/property_tree/json_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>

// System - Include Files
#include <iostream>
#include <numeric>
#include <map>

// ------ C O N S T A N T   V A R I A B L E S ---------------------------------

// ------ L O C A L  F U N C T I O N S  A N D  S T R U C T S ------------------

// ------ N A M E S P A C E ---------------------------------------------------
using namespace XBMemAccess;

// ------ S T A T I C   V A R I A B L E S -------------------------------------

// ------ F U N C T I O N S ---------------------------------------------------
size_t XBMemAccess::getDDRMemSize(const std::shared_ptr<xrt_core::device>& dev) 
{
    std::string errmsg;
    long long ddr_size = 0;
    int ddr_bank_count = 0;

    ddr_size = xrt_core::device_query<xrt_core::query::rom_ddr_bank_size_gb>(dev);
    ddr_bank_count = xrt_core::device_query<xrt_core::query::rom_ddr_bank_count_max>(dev);

    if (!ddr_size || !ddr_bank_count)
        return 0;

    return ddr_size*ddr_bank_count / (1024 * 1024);
}

/*
 * getDDRBanks()
 *
 * Get the addrs and size of each DDR bank
 * Sort the vector based on start address.
 * Returns number of banks.
 */
int XBMemAccess::getDDRBanks(const std::shared_ptr<xrt_core::device> dev, std::vector<mem_bank_t>& aBanks )
{
    auto raw_mem = xrt_core::device_query<xrt_core::query::mem_topology_raw>(dev);
    const mem_topology *map = (mem_topology *)raw_mem.data();
    if(raw_mem.empty() || map->m_count == 0) {
        std::cout << "ERROR: Memory topology is not available, "
            << "ensure that a valid bitstream is programmed onto the card." << std::endl;
        return -EINVAL;
    }

    for(int32_t i = 0; i < map->m_count; i++) {
        if( map->m_mem_data[i].m_used && map->m_mem_data[i].m_type != MEM_STREAMING ) {
            aBanks.emplace_back( map->m_mem_data[i].m_base_address, map->m_mem_data[i].m_size*1024, i );
        }
    }
    
    std::sort (aBanks.begin(), aBanks.end(),
            [] (const mem_bank_t& a, const mem_bank_t& b) {return (a.m_base_address < b.m_base_address);});
    
    return map->m_count;
}

/*
 * readWriteHelper()
 *
 * Sanity check the user's Start Address and Size against the mem topology
 * If the start address is 0 (ie. unspecified by user) change it to the first available address
 * If the size is 0 (ie. unspecified by user) change it to the maximum available size
 * Fill the vector with the available banks
 * Set the iterator to the bank containing the start address
 * returns the number of banks the start address and size going to span
 * return -EINVAL in case of any sanity check failures
 */
int XBMemAccess::readWriteHelper (const std::shared_ptr<xrt_core::device> dev, unsigned long long& aStartAddr, 
        unsigned long long& aSize, std::vector<mem_bank_t>& vec_banks, std::vector<mem_bank_t>::iterator& startbank) {
    std::stringstream sstr;
    int nbanks = getDDRBanks(dev, vec_banks);
    if (!nbanks) {
        std::cout << "ERROR: Memory topology is not available, ensure that a valid bitstream is programmed onto the card" << std::endl;
        return -EINVAL;
    }

    //if given start address is 0 then choose start address to be the lowest address available
    unsigned long long startAddr = aStartAddr == 0 ? vec_banks.front().m_base_address : aStartAddr;
    aStartAddr = startAddr;

    //Sanity check start address
    startbank = std::find_if(vec_banks.begin(), vec_banks.end(),
            [startAddr](const mem_bank_t& item) {return (startAddr >= item.m_base_address && startAddr < (item.m_base_address+item.m_size));});

    if (startbank == vec_banks.end()) {
        std:: cout << "ERROR: Start address 0x" << std::hex << startAddr << " is not valid" << std::dec << std::endl;
        std:: cout << "Available memory banks: " << sstr.str() << std::endl;
        return -EINVAL;
    }
    //Sanity check access size
    uint64_t availableSize = std::accumulate(startbank, vec_banks.end(), (uint64_t)0,
            [](uint64_t result, const mem_bank_t& obj) {return (result + obj.m_size);}) ;

    availableSize -= (startAddr - startbank->m_base_address);
    if (aSize > availableSize) {
        std:: cout << "ERROR: Cannot access " << aSize << " bytes of memory from start address 0x" << std::hex << startAddr << std::dec << std::endl;
        std:: cout << "Available memory banks: " << sstr.str() << std::endl;
        return -EINVAL;
    }

    //if given size is 0, then the end Address is the max address of the unused bank
    unsigned long long size = (aSize == 0) ? availableSize : aSize;
    aSize = size;

    //Find the number of banks this read/write straddles, this is just for better messaging
    int bankcnt = 0;
    unsigned long long tsize = size;
    for(auto it = startbank; it!=vec_banks.end(); ++it) {
        unsigned long long available_bank_size;
        if (it != startbank) {
            available_bank_size = it->m_size;
        }
        else {
            available_bank_size = it->m_size - (startAddr - it->m_base_address);
        }
        if (tsize != 0) {
            unsigned long long accesssize = (tsize > available_bank_size) ? (unsigned long long) available_bank_size : tsize;
            ++bankcnt;
            tsize -= accesssize;
        }
        else {
            break;
        }
    }
    return bankcnt;
}
