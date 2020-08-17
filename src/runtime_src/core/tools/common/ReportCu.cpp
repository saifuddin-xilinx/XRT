/**
 * Copyright (C) 2020 Xilinx, Inc
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
#include "ReportCu.h"
#include "tools/common/XBUtilities.h"
#include "core/common/query_requests.h"
#include "core/common/device.h"

namespace qr = xrt_core::query;
namespace XBU = XBUtilities;

enum class cu_stat : unsigned short {
  usage = 0,
  addr,
  stat
};

void
schedulerUpdateStat(const xrt_core::device *device) const
{
    try {
      auto const m_handle = device->get_device_handle();
      XBU::xclbin_lock xclbin_lock(device);
      xclUpdateSchedulerStat(m_handle);
    }
    catch (const std::exception&) {
      // xclbin_lock failed, safe to ignore
    }
}

uint32_t parseComputeUnitStat(const std::vector<std::string>& custat, uint32_t offset, cu_stat kind) const
{
  uint32_t ret = 0;

  if (custat.empty())
    return ret;

  for (auto& line : custat) {
    uint32_t ba = 0, cnt = 0, sta = 0;
    std::sscanf(line.c_str(), "CU[@0x%x] : %d status : %d", &ba, &cnt, &sta);

    if (offset != ba)
      continue;

    if (kind == cu_stat::usage)
      ret = cnt;
    else if (kind == cu_stat::stat)
      ret = sta;

    return ret;
  }

  return ret;
}

boost::property_tree::ptree
populate_cus(const xrt_core::device *device, const std::string& desc)
{
  if (!std::getenv("XCL_SKIP_CU_READ"))
    schedulerUpdateStat(device);

  boost::property_tree::ptree pt;
  std::vector<char> ip_buf;
  std::vector<std::string> cu_stats;

  try {
    ip_buf = xrt_core::device_query<qr::ip_layout_raw>(device);
    cu_stats = xrt_core::device_query<qr::kds_custat>(device);
  } catch (const std::exception& ex){
    pt.put("error_msg", ex.what());
  }

  if(ip_buf.empty() || cu_stats.empty())
    return pt;

  const ip_layout *layout = (ip_layout *)ip_buf.data();

  for (int i = 0; i < layout->m_count; i++) {
    const auto& ip = *layout->m_ip_data[i];
    if (ip.m_type != IP_KERNEL)
      continue;

    uint32_t status = parseComputeUnitStat(custat, ip.m_base_address, cu_stat::stat);
    uint32_t usage = parseComputeUnitStat(custat, ip.m_base_address, cu_stat::usage);
    boost::property_tree::ptree ptCu;
    ptCu.put( "name",         ip.m_name);
    ptCu.put( "base_address", ip.m_base_address);
    ptCu.put( "usage",        usage);
    ptCu.put( "status",       xrt_core::utils::parse_cu_status(status));
    pt::add_child( std::string("board.compute_unit" + std::to_string(i)), ptCu);
  }

  return pt;
}

void
ReportCu::getPropertyTreeInternal( const xrt_core::device * _pDevice, 
                                              boost::property_tree::ptree &_pt) const
{
  // Defer to the 20202 format.  If we ever need to update JSON data, 
  // Then update this method to do so.
  getPropertyTree20202(_pDevice, _pt);
}

void 
ReportCu::getPropertyTree20202( const xrt_core::device * _pDevice, 
                                           boost::property_tree::ptree &_pt) const
{
  boost::property_tree::ptree cu_array;
  cu_array.push_back(std::make_pair("", populate_cus(_pDevice, "Compute Units")));

  // There can only be 1 root node
  _pt.add_child("cus", cu_array);
}

void 
ReportCu::writeReport( const xrt_core::device * _pDevice,
                                  const std::vector<std::string> & /*_elementsFilter*/, 
                                  std::iostream & _output) const
{
  boost::property_tree::ptree _pt;
  boost::property_tree::ptree empty_ptree;
  getPropertyTreeInternal(_pDevice, _pt);

  _output << boost::format("%s\n") % _pt.get<std::string>("cus.description");
  //_output << "Compute Units\n";
  boost::property_tree::ptree& cus = _pt.get_child("cus.board.compute_unit");
  for(auto& kv : cus) {
    boost::property_tree::ptree& pt_cu = kv.second;
    _output << boost::format("  %-16s\n") % pt_cu.get<std::string>("name");
    _output << boost::format("    %-16s: %x\n") % pt_cu.get<std::hex>("base_address");
    _output << boost::format("    %-16s: %d\n") % pt_cu.get<std::dec>("usage");
    _output << boost::format("    %-16s: %s\n") % pt_cu.get<std::string>("status");
  }
  _output << std::endl;

}
