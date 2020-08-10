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

#ifndef __XBMemAccess_h_
#define __XBMemAccess_h_

// Include files
// Please keep these to the bare minimum
#include "core/common/device.h"

#include <string>
#include <memory>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

namespace XBMemAccess {
    struct mem_bank_t {
        uint64_t m_base_address;
        uint64_t m_size;
        int m_index;
        mem_bank_t (uint64_t aAddr, uint64_t aSize, int aIndex) : m_base_address(aAddr), m_size(aSize), m_index(aIndex) {}
    };

    size_t getDDRMemSize(const std::shared_ptr<xrt_core::device>& dev);
    int getDDRBanks(const std::shared_ptr<xrt_core::device> dev, std::vector<mem_bank_t>& aBanks);
    int readWriteHelper (const std::shared_ptr<xrt_core::device> dev, unsigned long long& aStartAddr, 
            unsigned long long& aSize, std::vector<mem_bank_t>& vec_banks, std::vector<mem_bank_t>::iterator& startbank);
};

#endif

