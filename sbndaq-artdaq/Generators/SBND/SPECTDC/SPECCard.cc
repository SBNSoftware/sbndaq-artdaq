#include "sbndaq-artdaq/Generators/SBND/SPECTDC/SPECTDC_Interface.hh"
#define TRACE_NAME "SPECCard"
#include "fhiclcpp/ParameterSet.h"
#include "sbndaq-artdaq/Generators/SBND/SPECTDC/StringLiterals.hh"

// clang-format off
#include "fmctdc-lib/fmctdc-lib.h"
#include "fmctdc-lib/fmctdc-lib-private.h"
// clang-format on

using namespace sbndaq::SPECTDCInterface;
namespace lit = sbndaq::literal;
namespace utls = sbndaq::SPECTDCInterface::TDCTimeUtils;
using sbndaq::PoolBuffer;
using sbndaq::TDCTimestamp;

std::string as_hexstring(std::string const& s);
//
// SPECCard
//
SPECCard::SPECCard(fhicl::ParameterSet const& ps, PoolBufferUPtr_t& b)
    : deviceid{as_hexstring(ps.get<std::string>(lit::deviceid, lit::autodetect))},
      verbose{ps.get<bool>(lit::verbose, false)},
      fmctdc{ps.get<fhicl::ParameterSet>(lit::fmctdc, fhicl::ParameterSet()), b} {
  if (verbose) {
    // TLOG(TLVL_INFO) << "Loaded config:\n" << as_fhicl();
    std::cout << "Loaded config:\n" << as_fhicl() << "\n";
  }
}

SPECCard::~SPECCard() {
  TLOG(TLVL_DEBUG_30) << "Autoclosing TDC device id=0x" << std::hex << deviceid << ".";
  close();
  TLOG(TLVL_DEBUG_30) << "Autoclosed TDC device id=0x" << std::hex << deviceid << ".";
}

bool SPECCard::open() {
  TLOG(TLVL_DEBUG_1) << "Opening TDC device id=" << deviceid << ".";
  auto card_count = fmctdc_init();
  if (card_count < 0) {
    TLOG(TLVL_ERROR) << "fmctdc_init() returned error: " << fmctdc_strerror(errno) << ".";
    return false;
  }

  if (deviceid == lit::autodetect) {
    TLOG(TLVL_DEBUG) << "Found " << card_count << " TDC card(s).";
    __fmctdc_board* b;
    fmctdc_board* ub;
    ub = fmctdc_open(0, -1);
    b = (decltype(b))ub;
    std::stringstream os;
    os << "0x" << std::hex << b->dev_id;
    deviceid = os.str();
    TLOG(TLVL_DEBUG_1) << "Selected TDC device id=" << deviceid << ".";
  }

  fmctdc.deviceid = std::stoul(deviceid, nullptr, 16);
  isOpen = fmctdc.open();
  TLOG(TLVL_DEBUG_1) << "Opened TDC device id=" << deviceid
                     << ", status=" << (isOpen ? lit::rc_success : lit::rc_failure) << ".";
  return isOpen;
}

void SPECCard::close() {
  if (!isOpen) return;

  TLOG(TLVL_DEBUG_2) << "Closing TDC device id=" << deviceid;
  fmctdc.close();
  fmctdc_exit();
  TLOG(TLVL_DEBUG_2) << "Closed TDC device id=" << deviceid;
  isOpen = false;
}

bool SPECCard::configure() {
  TLOG(TLVL_DEBUG_3) << "Configuring TDC device id=" << deviceid;

  if (!isOpen) {
    if (!open()) {
      close();
      TLOG(TLVL_ERROR) << "Failed configuring TDC device id=0x" << std::hex << deviceid << ".";
      return false;
    };
  }

  if (isConfigured) return true;

  isConfigured = fmctdc.configure();
  TLOG(TLVL_DEBUG_11) << "Configured TDC device id=" << deviceid
                      << ", status=" << (isConfigured ? "configured" : "not configured") << ".";

  if (!isConfigured) {
    close();
    TLOG(TLVL_ERROR) << "Failed configuring TDC device id=0x" << std::hex << deviceid << ".";
    return false;
  }
  TLOG(TLVL_DEBUG_3) << "Configured TDC device id=0x" << std::hex << deviceid << ".";

  return true;
}

void SPECCard::read() { fmctdc.read(); }
void SPECCard::monitor() { fmctdc.monitor(); }
bool SPECCard::start() { return fmctdc.start(); }
bool SPECCard::stop() { return fmctdc.stop(); }

