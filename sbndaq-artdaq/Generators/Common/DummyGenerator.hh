#ifndef sbndaq_artdaq_Generators_DummyGenerator_hh
#define sbndaq_artdaq_Generators_DummyGenerator_hh

#include "sbndaq-artdaq-core/Overlays/Common/DummyFragment.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "fhiclcpp/fwd.h"


namespace sbndaq {
/**
 * \brief DummyGenerator based on artdaq-demo ToySimulator
 * generates empty fragments in steady time intervals.
 * Its purpose is to provide fragments in push mode which
 * will trigger readout of other generators (e.g. CRT)
 * in pull mode, with no need to run other DAQ components
 * which normally run in push mode
 *
 */
class DummyGenerator : public artdaq::CommandableFragmentGenerator
  {
    public:
      explicit DummyGenerator(fhicl::ParameterSet const& ps);

      virtual ~DummyGenerator();

    private:
      bool getNext_(artdaq::FragmentPtrs& output) override;

      void start() override;
      void stop() override;
      void stopNoMutex() override {}

      DummyFragment::Metadata metadata_;

      int fragment_id_;

      std::chrono::time_point<std::chrono::system_clock> timestamp_;
      std::chrono::duration<int, std::ratio<1, 1000>> time_step_;

  };
}  // namespace sbndaq

#endif /* sbndaq_artdaq_Generators_DummyGenerator_hh */
