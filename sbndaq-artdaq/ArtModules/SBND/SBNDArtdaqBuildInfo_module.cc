#include "artdaq/ArtModules/BuildInfo_module.hh"

#include "artdaq/BuildInfo/GetPackageBuildInfo.hh"
#include "artdaq-core/BuildInfo/GetPackageBuildInfo.hh"
#include "sbndaq-artdaq-core/BuildInfo/GetPackageBuildInfo.hh"
#include "sbndaq-artdaq/BuildInfo/GetPackageBuildInfo.hh"

#include <string>

namespace sbnd
{
  /**
   * \brief Instance name for the sbnd_artdaq version of BuildInfo module
   */
  static std::string instanceName = "SBNDArtdaqBuildInfo";
  /**
   * \brief SBNDArtdaqBuildInfo is a BuildInfo type containing information about artdaq_core, artdaq, sbnd_artdaq_core and sbnd_artdaq builds.
   */
  typedef artdaq::BuildInfo<&instanceName, artdaqcore::GetPackageBuildInfo, artdaq::GetPackageBuildInfo, sbndaqcore::GetPackageBuildInfo, sbndaq::GetPackageBuildInfo> SBNDArtdaqBuildInfo;

  DEFINE_ART_MODULE(SBNDArtdaqBuildInfo)
}
