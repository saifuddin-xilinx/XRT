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
#include "OO_MemWrite.h"
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

OO_MemWrite::OO_MemWrite( const std::string &_longName, bool _isHidden)
    : OptionOptions(_longName, _isHidden, "Write to a given memory address")
    , m_device("")
    , m_baseAddress("")
    , m_sizeBytes("")
    , m_fill("")
    , m_inputFile("")
    , m_help(false)

{
  m_optionsDescription.add_options()
    ("device,d", boost::program_options::value<decltype(m_device)>(&m_device), "The Bus:Device.Function (e.g., 0000:d8:00.0) device of interest")
    ("address", boost::program_options::value<decltype(m_baseAddress)>(&m_baseAddress)->required(), "Base address to start from")
    ("size", boost::program_options::value<decltype(m_sizeBytes)>(&m_sizeBytes)->required(), "Size (bytes) to write")
    ("fill,f", boost::program_options::value<decltype(m_fill)>(&m_fill), "The byte value to fill the memory with")
    ("input,i", boost::program_options::value<decltype(m_inputFile)>(&m_inputFile), "The binary file to read from")
    ("help,h", boost::program_options::bool_switch(&m_help), "Help to use this sub-command")
  ;

  m_positionalOptions.
    add("address", 1 /* max_count */).
    add("size", 1 /* max_count */)
  ;
}

/*
 * writeBank()
 *
 * Write to the specified address within a bank
 * Caller's responsibility to do sanity checks. No sanity checks done here
 */
int 
writeBank(xclDeviceHandle mHandle, unsigned long long aStartAddr, unsigned long long aSize, char *inputBuf) {
  unsigned long long endAddr;
  unsigned long long size;
  unsigned long long blockSize = 0x20000;//128KB

  endAddr = aStartAddr + aSize;
  size = endAddr-aStartAddr;

  std::cout << "INFO: Writing DDR/HBM/PLRAM with " << std::dec << size << " byte from address 0x" 
        << std::hex << aStartAddr << std::endl;

  unsigned long long count = size;
  uint64_t incr = 0;
  for(uint64_t phy=aStartAddr; phy<endAddr; phy+=incr) {
      incr = (count >= blockSize) ? blockSize : count;
      if (xclUnmgdPwrite(mHandle, 0, inputBuf, incr, phy) < 0) {
          //error
          std::cout << "Error (" << strerror (errno) << ") writing 0x" << std::hex << incr << " bytes to DDR/HBM/PLRAM at offset 0x" << std::hex << phy << std::dec << "\n";
          return -1;
      }
      count -= incr;
      inputBuf += incr;
  }

  if (count != 0) {
      std::cout << "Error! Written " << std::dec << size-count << " bytes, requested " << size << std::endl;
      return -1;
  }

  return count;
}

void
OO_MemWrite::execute(const SubCmdOptions& _options) const
{
  XBU::verbose("SubCommand: write-mem");

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

  // -- process either Input Pattern or Input Binary File ----------------
  char *inputBuf = 0;
  if (!m_fill.empty()) {
    unsigned int aPattern = std::stoi(m_fill);
    size_t idx = 0;
    try {
      aPattern = std::stoi(m_fill, &idx, 0);
    }
    catch (const std::exception& ex) {
      //out of range, invalid argument ex
      std::cout << "ERROR: Value supplied to fill must be a value between 0 and 255\n";
      return;
    }
    if (aPattern > 0xff || idx < m_fill.length()) {
      std::cout << "ERROR: Value supplied to fill must be a value between 0 and 255\n";
      return;
    }
  
    // -- Initialize input Buffer -----------------------------------------
    if (xrt_core::posix_memalign((void **)&inputBuf, getpagesize(), sizeBytes)) {
        std::cout << "ERROR: Memory allocation failed." <<  std::endl;
        return;
    }
    std::memset(inputBuf, aPattern, sizeBytes);
  }
  else {
    // -- process Input Binary File ----------------------------------------
    std::ifstream fInput;
    fInput.open(m_inputFile, std::ios::in | std::ios::binary | std::ios::ate);
    if (!fInput.is_open())
      throw xrt_core::error((boost::format("Unable to open the file '%s' for reading.") % m_inputFile).str());

    size_t input_buf_size = fInput.tellg();
    if (xrt_core::posix_memalign((void **)&inputBuf, getpagesize(), input_buf_size)) {
        std::cout << "ERROR: Memory allocation failed." <<  std::endl;
        return;
    }

    fInput.seekg (0, std::ios::beg);
    fInput.read(inputBuf, input_buf_size);
    if ((fInput.rdstate() & std::ifstream::failbit) != 0) {
        std::cout << "ERROR : Reading from the binary file" << "\n";
    }
    sizeBytes = (input_buf_size < sizeBytes) ? input_buf_size : sizeBytes;
  }

  // -- All Input validation Done Here -------------------------------------

  auto const dev = xrt_core::get_userpf_device(index);
  auto const m_handle = dev->get_device_handle();
  std::vector<XBM::mem_bank_t> vec_banks;
  std::vector<XBM::mem_bank_t>::iterator startbank;
  int bankcnt = 0;

  //Sanity check the address and size against the mem topology
  if ((bankcnt = XBM::readWriteHelper(dev, baseAddress, sizeBytes, vec_banks, startbank)) == -1) {
      return;
  }

  if (bankcnt > 1) {
      std::cout << "INFO: Writing " << std::dec << sizeBytes << " bytes from DDR/HBM/PLRAM address 0x"  << std::hex << baseAddress
          << " straddles " << bankcnt << " banks" << std::dec << std::endl;
  }
  else {
      std::cout << "INFO: Writing to single bank, " << std::dec << sizeBytes << " bytes from DDR/HBM/PLRAM address 0x"  << std::hex << baseAddress
          << std::dec << std::endl;
  }

  unsigned long long inputBufOff = 0;
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
          unsigned long long writesize = (sizeBytes > available_bank_size) ? (unsigned long long) available_bank_size : sizeBytes;
          if( writeBank(m_handle, baseAddress, writesize, inputBuf + inputBufOff) == -1) {
              return;
          }
          sizeBytes -= writesize;
          inputBufOff += writesize;
      }
      else {
          break;
      }
  }

  return;
}

