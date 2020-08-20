#include "artdaq-core/Data/FragmentNameHelper.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"

#include "TRACE/tracemf.h"
#define TRACE_NAME "SBNDFragmentNameHelper"

namespace sbnd {
/**
 * \brief SBNDFragmentNameHelper extends ArtdaqFragmentNamingService.
 * This implementation uses SBND's SystemTypeMap and directly assigns names based on it
 */
class SBNDFragmentNameHelper : public artdaq::ArtdaqFragmentNameHelper {
 public:
	/**
	 * \brief DefaultArtdaqFragmentNamingService Destructor
	 */
	~SBNDFragmentNameHelper() override = default;

	/**
	 * \brief SBNDFragmentNameHelper Constructor
	 */
	SBNDFragmentNameHelper(std::string unidentified_instance_name, std::vector<std::pair<artdaq::Fragment::type_t, std::string>> extraTypes);

 private:
	SBNDFragmentNameHelper(SBNDFragmentNameHelper const&) = delete;
	SBNDFragmentNameHelper(SBNDFragmentNameHelper&&) = delete;
	SBNDFragmentNameHelper& operator=(SBNDFragmentNameHelper const&) = delete;
	SBNDFragmentNameHelper& operator=(SBNDFragmentNameHelper&&) = delete;
};

SBNDFragmentNameHelper::SBNDFragmentNameHelper(std::string unidentified_instance_name,
																							 std::vector<std::pair<artdaq::Fragment::type_t, std::string>> extraTypes)
		: ArtdaqFragmentNameHelper(unidentified_instance_name, extraTypes) {
	TLOG(TLVL_DEBUG) << "SBNDFragmentNameHelper CONSTRUCTOR START";
	SetBasicTypes(sbndaq::makeFragmentTypeMap());
	TLOG(TLVL_DEBUG) << "SBNDFragmentNameHelper CONSTRUCTOR END";
}
}	// namespace sbnd

DEFINE_ARTDAQ_FRAGMENT_NAME_HELPER(sbnd::SBNDFragmentNameHelper)
