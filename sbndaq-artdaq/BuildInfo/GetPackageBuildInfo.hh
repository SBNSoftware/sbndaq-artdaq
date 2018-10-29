#ifndef icarus_artdaq_BuildInfo_GetPackageBuildInfo_hh
#define icarus_artdaq_BuildInfo_GetPackageBuildInfo_hh

#include "artdaq-core/Data/PackageBuildInfo.hh"

#include <string>

namespace icarus {

  struct GetPackageBuildInfo {

    static artdaq::PackageBuildInfo getPackageBuildInfo();
  };

}

#endif /* icarus_artdaq_BuildInfo_GetPackageBuildInfo_hh */
