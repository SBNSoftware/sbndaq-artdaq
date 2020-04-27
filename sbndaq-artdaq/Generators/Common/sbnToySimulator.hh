#ifndef sbndaq_artdaq_Generators_sbnToySimulator_hh
#define sbndaq_artdaq_Generators_sbnToySimulator_hh

#include "sbndaq-artdaq-core/Overlays/Common/sbnToyFragment.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "fhiclcpp/fwd.h"


namespace sbndaq {
/**
 * \brief sbnToySimulator based on artdaq-demo ToySimulator
 * generates empty fragments in steady time intervals.
 * Its purpose is to provide fragments in push mode which
 * will trigger readout of other generators (e.g. CRT)
 * in pull mode, with no need to run other DAQ components
 * which normally run in push mode
 *
 */
class sbnToySimulator : public artdaq::CommandableFragmentGenerator
  {
    public:
      explicit sbnToySimulator(fhicl::ParameterSet const& ps);

      virtual ~sbnToySimulator();

    private:
      bool getNext_(artdaq::FragmentPtrs& output) override;

      void start() override;
      void stop() override;
      void stopNoMutex() override {}

      sbnToyFragment::Metadata metadata_;

      int fragment_id_;

      std::chrono::time_point<std::chrono::system_clock> timestamp_;
      std::chrono::duration<int, std::ratio<1, 1000>> time_step_;

  };
}  // namespace sbndaq

#endif /* sbndaq_artdaq_Generators_sbnToySimulator_hh */
