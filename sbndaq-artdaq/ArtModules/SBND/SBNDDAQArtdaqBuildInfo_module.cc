#include "artdaq/ArtModules/BuildInfo_module.hh"

#include "artdaq/BuildInfo/GetPackageBuildInfo.hh"
#include "artdaq-core/BuildInfo/GetPackageBuildInfo.hh"
#include "sbndaq-artdaq-core/BuildInfo/GetPackageBuildInfo.hh"
#include "sbndaq-artdaq/BuildInfo/GetPackageBuildInfo.hh"

#include <string>

namespace sbnddaq
{
	/**
	 * \brief Instance name for the icarus_artdaq version of BuildInfo module
	 */
	static std::string instanceName = "SBNDDAQArtdaqBuildInfo";
	/**
	 * \brief SBNDDAQArtdaqBuildInfo is a BuildInfo type containing information about artdaq_core, artdaq, icarus_artdaq_core and icarus_artdaq builds.
	 */
	typedef artdaq::BuildInfo<&instanceName, artdaqcore::GetPackageBuildInfo, artdaq::GetPackageBuildInfo, sbndaqcore::GetPackageBuildInfo, icarus::GetPackageBuildInfo> SBNDDAQArtdaqBuildInfo;

	DEFINE_ART_MODULE(SBNDDAQArtdaqBuildInfo)
}
