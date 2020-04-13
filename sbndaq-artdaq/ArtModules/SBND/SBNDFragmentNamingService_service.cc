#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq/ArtModules/ArtdaqFragmentNamingService.h"

#include "TRACE/tracemf.h"
#define TRACE_NAME "SBNDFragmentNamingService"

/**
 * \brief SBNDFragmentNamingService extends ArtdaqFragmentNamingService.
 * This implementation uses artdaq-demo's SystemTypeMap and directly assigns names based on it
 */
class SBNDFragmentNamingService : public ArtdaqFragmentNamingService
{
public:
	/**
	 * \brief DefaultArtdaqFragmentNamingService Destructor
	 */
	virtual ~SBNDFragmentNamingService() = default;

	/**
	 * \brief SBNDFragmentNamingService Constructor
	 */
	SBNDFragmentNamingService(fhicl::ParameterSet const&, art::ActivityRegistry&);

private:
};

SBNDFragmentNamingService::SBNDFragmentNamingService(fhicl::ParameterSet const& ps, art::ActivityRegistry& r)
    : ArtdaqFragmentNamingService(ps, r)
{
	TLOG(TLVL_DEBUG) << "SBNDFragmentNamingService CONSTRUCTOR START";
	SetBasicTypes(sbndaq::makeFragmentTypeMap());
	TLOG(TLVL_DEBUG) << "SBNDFragmentNamingService CONSTRUCTOR END";
}

DECLARE_ART_SERVICE_INTERFACE_IMPL(SBNDFragmentNamingService, ArtdaqFragmentNamingServiceInterface, LEGACY)
DEFINE_ART_SERVICE_INTERFACE_IMPL(SBNDFragmentNamingService, ArtdaqFragmentNamingServiceInterface)
