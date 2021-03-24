#include "artdaq-core/Data/ContainerFragment.hh"
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
        
        //overrides
        std::pair<bool, std::string> GetInstanceNameForFragment(artdaq::Fragment const&) const override;
        std::set<std::string> GetAllProductInstanceNames() const override;

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

std::set<std::string> IcarusFragmentNameHelper::GetAllProductInstanceNames() const
{
  std::set<std::string> output;
  for (const auto& map_iter : type_map_)
    {
      std::string instance_name = map_iter.second;
      if (output.count(instance_name) == 0u)
	{
	  output.insert(instance_name);  
	  TLOG(TLVL_TRACE) << "Adding product instance name \"" << map_iter.second << "\" to list of expected names";
	}
    }
  auto container_type = type_map_.find(artdaq::Fragment::type_t(artdaq::Fragment::ContainerFragmentType));
  if (container_type != type_map_.end())
    {
      std::string container_type_name = container_type->second;
      std::set<std::string> tmp_copy = output;
      for (const auto& set_iter : tmp_copy)
	{
	  output.insert(container_type_name + set_iter);
	}
    }


  //add TPCEE/EW/etc.
  auto tpc_type = type_map_.find(sbndaq::detail::FragmentType::PHYSCRATEDATA);
  if(tpc_type==type_map_.end())
    return output;
  else{
    output.insert(tpc_type->second+"TPCEE");
    output.insert(tpc_type->second+"TPCEW");
    output.insert(tpc_type->second+"TPCWE");
    output.insert(tpc_type->second+"TPCWW");
  }

  return output;
}


std::pair<bool, std::string>
IcarusFragmentNameHelper::GetInstanceNameForFragment(artdaq::Fragment const& fragment) const {

  auto type_map_end = type_map_.end();
  bool success_code = true;
  std::string instance_name;

  auto primary_type = type_map_.find(fragment.type());
  if (primary_type != type_map_end)
    {
      TLOG(TLVL_TRACE) << "Found matching instance name " << primary_type->second << " for Fragment type " << fragment.type();
      instance_name = primary_type->second;
      if (fragment.type() == artdaq::Fragment::ContainerFragmentType)
	{
	  artdaq::ContainerFragment cf(fragment);
	  auto contained_type = type_map_.find(cf.fragment_type());
	  if (contained_type != type_map_end)
	    {
	      instance_name += contained_type->second;
	    }
	}

      if(fragment.type()==sbndaq::detail::FragmentType::PHYSCRATEDATA){

	auto idmask = fragment.fragmentID() & 0xff00;
	
	if(idmask==0x10000 || idmask==0x1100)
	  instance_name += "TPCEE";
	else if(idmask==0x12000 || idmask==0x1300)
	  instance_name += "TPCEW";
	else if(idmask==0x13000 || idmask==0x1500)
	  instance_name += "TPCWE";
	else if(idmask==0x14000 || idmask==0x1700)
	  instance_name += "TPCWW";
      }

    }

  else
    {
      TLOG(TLVL_TRACE) << "Could not find match for Fragment type " << fragment.type() << ", returning " << unidentified_instance_name_;
      instance_name = unidentified_instance_name_;
      success_code = false;
    }
  return std::make_pair(success_code, instance_name);
}



}	// namespace icarus

DEFINE_ARTDAQ_FRAGMENT_NAME_HELPER(icarus::IcarusFragmentNameHelper)
