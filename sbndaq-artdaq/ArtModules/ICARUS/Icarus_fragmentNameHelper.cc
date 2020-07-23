#include "artdaq-core/Data/FragmentNameHelper.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "TRACE/tracemf.h"
#define TRACE_NAME "IcarusFragmentNameHelper"

namespace icarus {
/**
 * \brief IcarusFragmentNameHelper extends ArtdaqFragmentNamingService.
 * This implementation uses Icarus' SystemTypeMap and directly assigns names based on it
 */
class IcarusFragmentNameHelper : public artdaq::ArtdaqFragmentNameHelper {
 public:
	/**
	 * \brief DefaultArtdaqFragmentNamingService Destructor
	 */
	~IcarusFragmentNameHelper() override = default;

	/**
	 * \brief IcarusFragmentNameHelper Constructor
	 */
	IcarusFragmentNameHelper(std::string unidentified_instance_name, std::vector<std::pair<artdaq::Fragment::type_t, std::string>> extraTypes);

 private:
	IcarusFragmentNameHelper(IcarusFragmentNameHelper const&) = delete;
	IcarusFragmentNameHelper(IcarusFragmentNameHelper&&) = delete;
	IcarusFragmentNameHelper& operator=(IcarusFragmentNameHelper const&) = delete;
	IcarusFragmentNameHelper& operator=(IcarusFragmentNameHelper&&) = delete;
};

IcarusFragmentNameHelper::IcarusFragmentNameHelper(std::string unidentified_instance_name,
																									 std::vector<std::pair<artdaq::Fragment::type_t, std::string>> extraTypes)
		: ArtdaqFragmentNameHelper(unidentified_instance_name, extraTypes) {
	TLOG(TLVL_DEBUG) << "IcarusFragmentNameHelper CONSTRUCTOR START";
	SetBasicTypes(sbndaq::makeFragmentTypeMap());
	TLOG(TLVL_DEBUG) << "IcarusFragmentNameHelper CONSTRUCTOR END";
}
}	// namespace icarus

DEFINE_ARTDAQ_FRAGMENT_NAME_HELPER(icarus::IcarusFragmentNameHelper)
