#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "artdaq/ArtModules/ArtdaqFragmentNamingService.h"

#include "TRACE/tracemf.h"
#define TRACE_NAME "IcarusFragmentNamingService"

/**
 * \brief IcarusFragmentNamingService extends ArtdaqFragmentNamingService.
 * This implementation uses artdaq-demo's SystemTypeMap and directly assigns names based on it
 */
class IcarusFragmentNamingService : public ArtdaqFragmentNamingService
{
public:
	/**
	 * \brief DefaultArtdaqFragmentNamingService Destructor
	 */
	virtual ~IcarusFragmentNamingService() = default;

	/**
	 * \brief IcarusFragmentNamingService Constructor
	 */
	IcarusFragmentNamingService(fhicl::ParameterSet const&, art::ActivityRegistry&);

private:
};

IcarusFragmentNamingService::IcarusFragmentNamingService(fhicl::ParameterSet const& ps, art::ActivityRegistry& r)
    : ArtdaqFragmentNamingService(ps, r)
{
	TLOG(TLVL_DEBUG) << "IcarusFragmentNamingService CONSTRUCTOR START";
	SetBasicTypes(sbndaq::makeFragmentTypeMap());
	TLOG(TLVL_DEBUG) << "IcarusFragmentNamingService CONSTRUCTOR END";
}

DECLARE_ART_SERVICE_INTERFACE_IMPL(IcarusFragmentNamingService, ArtdaqFragmentNamingServiceInterface, LEGACY)
DEFINE_ART_SERVICE_INTERFACE_IMPL(IcarusFragmentNamingService, ArtdaqFragmentNamingServiceInterface)
