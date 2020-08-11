/**
 * Copyright (C) 2020 Licensed under the Apache License, Version
 * 2.0 (the "License"). You may not use this file except in
 * compliance with the License. A copy of the License is located
 * at
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
#include "OO_MemRead.h"
#include "XBMemAccess.h"
#include "core/common/system.h"
#include "core/common/device.h"
#include "core/common/memalign.h"
#include "core/common/utils.h"
#include "tools/common/XBUtilities.h"

namespace XBU = XBUtilities;
namespace XBM = XBMemAccess;

// 3rd Party Library - Include Files
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
namespace po = boost::program_options;

// System - Include Files
#include <iostream>
#include <fstream>

// ----- C L A S S   M E T H O D S -------------------------------------------

OO_MemRead::OO_MemRead( const std::string &_longName, bool _isHidden )
    : OptionOptions(_longName, _isHidden, "Read from the given memory address" )
    , m_device("")
    , m_baseAddress("")
    , m_sizeBytes("")
    , m_outputFile("")
    , m_help(false)
{
  m_optionsDescription.add_options()
    ("device,d", boost::program_options::value<decltype(m_device)>(&m_device), "The Bus:Device.Function (e.g., 0000:d8:00.0) device of interest")
    ("output,o", boost::program_options::value<decltype(m_outputFile)>(&m_outputFile), "Output file")
    ("address", boost::program_options::value<decltype(m_baseAddress)>(&m_baseAddress)->required(), "Base address to start from")
    ("size", boost::program_options::value<decltype(m_sizeBytes)>(&m_sizeBytes)->required(), "Size (bytes) to read")
    ("help,h", boost::program_options::bool_switch(&m_help), "Help to use this sub-command")
  ;

  m_positionalOptions.
    add("address", 1 /* max_count */).
    add("size", 1 /* max_count */)
  ;
}

/*
 * readBank()
 *
 * Read from specified address, specified size within a bank
 * Caller's responsibility to do sanity checks. No sanity checks done here
 */
static int 
readBank(xclDeviceHandle mHandle, std::ofstream& aOutFile, unsigned long long aStartAddr, unsigned long long aSize) {
    unsigned long long blockSize = 0x20000;
    auto buf = xrt_core::aligned_alloc(getpagesize(), blockSize);
    if (!buf)
        return -1;
    std::memset(buf.get(), 0, blockSize);

    size_t count = aSize;
    uint64_t incr;
    auto guard = xrt_core::utils::ios_restore(std::cout);
    for (uint64_t phy = aStartAddr; phy < aStartAddr+aSize; phy += incr) {
        incr = (count >= blockSize) ? blockSize : count;
        if (xclUnmgdPread(mHandle, 0, buf.get(), incr, phy) < 0) {
            //error
            std::cout << "Error (" << strerror (errno) << ") reading 0x" << std::hex << incr << " bytes from DDR/HBM/PLRAM at offset 0x" << std::hex << phy << std::dec << "\n";
            return -1;
        }
        count -= incr;
        if (incr) {
            aOutFile.write(reinterpret_cast<const char*>(buf.get()), incr);
            if ((aOutFile.rdstate() & std::ifstream::failbit) != 0) {
                std::cout << "Error writing to file at offset " << aSize-count << "\n";
            }
        }
        std::cout << "INFO: Read size 0x" << std::hex << incr << " B from addr 0x" << phy
            << ". Total Read so far 0x" << aSize-count << std::endl;
    }
    if (count != 0) {
        std::cout << "Error! Read " << std::dec << aSize-count << " bytes, requested " << aSize << std::endl;
        return -1;
    }

    return count;
}

void
OO_MemRead::execute(const SubCmdOptions& _options) const
{
  XBU::verbose("SubCommand: read-mem");
  
  bool help = false;
  po::options_description hiddenOptions("Hidden Options");

  po::options_description allOptions("All Options");
  allOptions.add(m_optionsDescription);
  allOptions.add(hiddenOptions);

  // Parse sub-command ...
  po::variables_map vm;

  try {
    po::store(po::command_line_parser(_options).options(allOptions).run(), vm);
    po::notify(vm); // Can throw
  } catch (po::error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    printHelp();

    // Re-throw exception
    throw;
  }

  // Check to see if help was requested or no command was found
  if (help == true)  {
    printHelp();
    return;
  }

  // -- process device option --------------------------------------------
  std::string deviceBDF;
  deviceBDF = boost::algorithm::to_lower_copy(m_device);
  auto index = xrt_core::utils::bdf2index(deviceBDF, true /*_inUserDomain*/);         // Can throw
  if (index < 0)
     return; // Invaid device index 

  // -- process Input Address and Size -----------------------------------
  unsigned long long baseAddress = std::stoll(m_baseAddress, 0, 0);
  unsigned long long sizeBytes = std::stoll(m_sizeBytes, 0, 0); 

  // -- process Output File ----------------------------------------------
  std::ofstream fOutput;
  fOutput.open(m_outputFile, std::ios::out | std::ios::binary);
  if (!fOutput.is_open())
      throw xrt_core::error((boost::format("Unable to open the file '%s' for writing.") % m_outputFile).str());

  // -- All Input validation Done Here -----------------------------------
  auto const dev = xrt_core::get_userpf_device(index); 
  auto const m_handle = dev->get_device_handle();
  std::vector<XBM::mem_bank_t> vec_banks;
  std::vector<XBM::mem_bank_t>::iterator startbank;
  int bankcnt = 0;

  //Sanity check the address and size against the mem topology
  if ((bankcnt = XBM::readWriteHelper(dev, baseAddress, sizeBytes, vec_banks, startbank)) == -EINVAL) {
      return;
  }

  if (bankcnt > 1) {
      std::cout << "INFO: Reading " << std::dec << sizeBytes << " bytes from DDR/HBM/PLRAM address 0x"  << std::hex << baseAddress
          << " straddles " << bankcnt << " banks" << std::dec << std::endl;
  }
  else {
      std::cout << "INFO: Reading from single bank, " << std::dec << sizeBytes << " bytes from DDR/HBM/PLRAM address 0x"  << std::hex << baseAddress
          << std::dec << std::endl;
  }
  
  size_t count = sizeBytes;
  for(auto it = startbank; it!=vec_banks.end(); ++it) {
      unsigned long long available_bank_size;
      if (it != startbank) {
          baseAddress = it->m_base_address;
          available_bank_size = it->m_size;
      }
      else {
          available_bank_size = it->m_size - (baseAddress - it->m_base_address);
      }
      if (sizeBytes != 0) {
          unsigned long long readsize = (sizeBytes > available_bank_size) ? (unsigned long long) available_bank_size : sizeBytes;
          if( readBank(m_handle, fOutput, baseAddress, readsize) == -1) {
              std::cout << "Error! Read " << std::dec << sizeBytes-count << " bytes, requested " << sizeBytes << std::endl;
              return;
          }
          sizeBytes -= readsize;
      }
      else {
          break;
      }
  }

  fOutput.close();
  std::cout << "INFO: Read data saved in file: " << m_outputFile << "; Num of bytes: " << std::dec << count - sizeBytes << " bytes " << std::endl;

  return;
}

