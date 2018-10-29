#include "artdaq/ArtModules/BuildInfo_module.hh"

#include "artdaq/BuildInfo/GetPackageBuildInfo.hh"
#include "artdaq-core/BuildInfo/GetPackageBuildInfo.hh"
#include "sbndaq-artdaq-core/BuildInfo/GetPackageBuildInfo.hh"
#include "sbndaq-artdaq/BuildInfo/GetPackageBuildInfo.hh"

#include <string>

namespace icarus
{
	/**
	 * \brief Instance name for the icarus_artdaq version of BuildInfo module
	 */
	static std::string instanceName = "IcarusArtdaqBuildInfo";
	/**
	 * \brief IcarusArtdaqBuildInfo is a BuildInfo type containing information about artdaq_core, artdaq, icarus_artdaq_core and icarus_artdaq builds.
	 */
	typedef artdaq::BuildInfo<&instanceName, artdaqcore::GetPackageBuildInfo, artdaq::GetPackageBuildInfo, sbndaqcore::GetPackageBuildInfo, icarus::GetPackageBuildInfo> IcarusArtdaqBuildInfo;

	DEFINE_ART_MODULE(IcarusArtdaqBuildInfo)
}
