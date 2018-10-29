#include "art/Framework/IO/Sources/Source.h"
#include "artdaq/ArtModules/detail/SharedMemoryReader.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/IO/Sources/SourceTraits.h"

#include <string>
using std::string;

/**
 * \brief The icarus_artdaq namespace
 */
namespace icarus
{
	/**
	 * \brief DemoInput is an art::Source using the detail::RawEventQueueReader class
	 */
	typedef art::Source< artdaq::detail::SharedMemoryReader<icarus::makeFragmentTypeMap> > IcarusInput;
}

namespace art
{
	/**
	* \brief  Specialize an art source trait to tell art that we don't care about
	* source.fileNames and don't want the files services to be used.
	*/
	template <>
	struct Source_generator<artdaq::detail::SharedMemoryReader<icarus::makeFragmentTypeMap>>
	{
		static constexpr bool value = true; ///< Used to suppress use of file services on art Source
	};
}

DEFINE_ART_INPUT_SOURCE(icarus::IcarusInput)
