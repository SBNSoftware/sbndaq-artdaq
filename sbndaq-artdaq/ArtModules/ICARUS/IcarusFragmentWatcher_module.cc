////////////////////////////////////////////////////////////////////////
// Class:       IcarusFragmentWatcher
// Module Type: analyzer
// File:        IcarusFragmentWatcher_module.cc
// Description: Collects and reports statistics on missing and empty fragments
//
// The model that is followed here is to publish to the metrics system
// the full history of what has happened so far.  In that way, each update
// is self-contained.  So, the map of fragment IDs that have missing or
// empty fragments will contain the total number of events in which each
// fragment ID was missing or empty.
//
// This module is icarus-specific implementation of the artdaq FragmentWatcher_module.cc 
//
// TRACE messages, though, contain a mix of per-event and overall results.
// To enable TLVL_TRACE messages that have overall resuts (for debugging),
// use 'tonM -n <appname>_IcarusFragmentWatcher 4'.
////////////////////////////////////////////////////////////////////////

#define TRACE_NAME (app_name + "_IcarusFragmentWatcher").c_str()
#include "artdaq/DAQdata/Globals.hh"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq-core/Data/Fragment.hh"

#include <bitset>
#include <iostream>
#include <map>

#define TLVL_BAD_FRAGMENTS TLVL_WARNING
#define TLVL_EVENT_SUMMARY TLVL_TRACE
#define TLVL_EXPECTED_FRAGIDS TLVL_DEBUG+5
#define TLVL_BASIC_MODE TLVL_DEBUG+6
#define TLVL_FRACTIONAL_MODE TLVL_DEBUG+7

namespace icarus {
class IcarusFragmentWatcher;
}

/// <summary>
/// An art::EDAnalyzer module which checks events for certain error conditions (missing fragments, empty fragments, etc)
/// </summary>
class icarus::IcarusFragmentWatcher : public art::EDAnalyzer
{
public:
	/**
	 * \brief IcarusFragmentWatcher Constructor
	 * \param pset ParameterSet used to configure FragmentWatcher
	 *
	 * FragmentWatcher accepts the following Parameters:
	 * mode_bitmask (default: 0x1): Mask of modes to use. BASIC_COUNTS_MODE = 0, FRACTIONAL_COUNTS_MODE = 1, DETAILED_COUNTS_MODE = 2
	 * metrics_reporting_level (default: 1): Level to use for metrics reporting
	 * metrics: A artdaq::MetricManager::Config ParameterSet used to configure MetricManager reporting for this module
	 */
	explicit IcarusFragmentWatcher(fhicl::ParameterSet const& pset);
	/**
	 * \brief Virtual Destructor. Shuts down MetricManager if one is present
	 */
	~IcarusFragmentWatcher() override;

	/**
   * \brief Analyze each event, using the configured mode bitmask
   * \param evt art::Event to analyze
   */
	void analyze(art::Event const& evt) override;

private:
	IcarusFragmentWatcher(IcarusFragmentWatcher const&) = delete;
	IcarusFragmentWatcher(IcarusFragmentWatcher&&) = delete;
	IcarusFragmentWatcher& operator=(IcarusFragmentWatcher const&) = delete;
	IcarusFragmentWatcher& operator=(IcarusFragmentWatcher&&) = delete;

  std::string getNameFromKey(const int key);

	std::bitset<3> mode_bitset_;
	int metrics_reporting_level_;

	int events_processed_;
	std::set<int> expected_fragmentID_list_;

	int events_with_missing_fragments_;
	int events_with_empty_fragments_;
  std::map<int, int> events_with_missing_fragments_by_subsystem_;
  std::map<int, int> events_with_empty_fragments_by_subsystem_;

	int events_with_10pct_missing_fragments_;
	int events_with_10pct_empty_fragments_;
	int events_with_50pct_missing_fragments_;
	int events_with_50pct_empty_fragments_;

	std::map<int, int> missing_fragments_by_fragmentID_;
	std::map<int, int> empty_fragments_by_fragmentID_;
  std::map<int, int> missing_fragments_by_subsystem_; // Gropu togheter missing fragments from the same subsystem 
  std::map<int, int> empty_fragments_by_subsystem_; // Group togheter empty fragments from the same subsystem 

	const int BASIC_COUNTS_MODE = 0;
	const int FRACTIONAL_COUNTS_MODE = 1;
	const int DETAILED_COUNTS_MODE = 2;
  
  enum Subsystems {
    kTPC = 0x1000,
    kPMT = 0x2000,
    kCRT = 0x3000 
  };
};

icarus::IcarusFragmentWatcher::IcarusFragmentWatcher(fhicl::ParameterSet const& pset)
    : EDAnalyzer(pset)
    , mode_bitset_(std::bitset<3>(pset.get<int>("mode_bitmask", 0x1)))
    , metrics_reporting_level_(pset.get<int>("metrics_reporting_level", 1))
    , events_processed_(0)
    , expected_fragmentID_list_()
    , events_with_missing_fragments_(0)
    , events_with_empty_fragments_(0)
    , events_with_missing_fragments_by_subsystem_()
    , events_with_empty_fragments_by_subsystem_()
    , events_with_10pct_missing_fragments_(0)
    , events_with_10pct_empty_fragments_(0)
    , events_with_50pct_missing_fragments_(0)
    , events_with_50pct_empty_fragments_(0)
    , missing_fragments_by_fragmentID_()
    , empty_fragments_by_fragmentID_()
    , missing_fragments_by_subsystem_()
    , empty_fragments_by_subsystem_()
{
}

icarus::IcarusFragmentWatcher::~IcarusFragmentWatcher()
{
}

std::string icarus::IcarusFragmentWatcher::getNameFromKey( const int key ){
  
  switch(key){
    case kTPC : return "TPC";
    case kPMT : return "PMT";
    case kCRT : return "CRT";
    default : return "Unknown";
  }

}

void icarus::IcarusFragmentWatcher::analyze(art::Event const& evt)
{
	events_processed_++;

	// get all the artdaq fragment collections in the event.
	std::vector<art::Handle<std::vector<artdaq::Fragment>>> fragmentHandles;
#if ART_HEX_VERSION < 0x30900
	evt.getManyByType(fragmentHandles);
#else
	fragmentHandles = evt.getMany<std::vector<artdaq::Fragment>>();
#endif

	std::set<int> missing_fragmentID_list_this_event(expected_fragmentID_list_);
	// Check for missing Fragment IDs, updating the master list as necessary
	for (auto const& hndl : fragmentHandles)
	{
		for (auto const& fragment : *hndl)
		{
			int fragID = fragment.fragmentID();
			TLOG(TLVL_EXPECTED_FRAGIDS) << "Inserting fragment ID " << fragID << " into the list of expected_fragmentIDs.";
			expected_fragmentID_list_.insert(fragID);
			missing_fragmentID_list_this_event.erase(fragID);
		}
	}

	// track the number of missing fragments by fragment ID
	for (int const& fragID : missing_fragmentID_list_this_event)
	{
		if (missing_fragments_by_fragmentID_.count(fragID) == 0)
		{
			missing_fragments_by_fragmentID_[fragID] = 1;
      events_with_missing_fragments_by_subsystem_[ fragID & 0xF000 ] = 1;  // the MSB of the fragID encoding holds information on the subsystem
    }
		else
		{
			missing_fragments_by_fragmentID_[fragID] += 1;
      missing_fragments_by_subsystem_[ fragID & 0xF000 ] +=1;
		}
	}

	// check if this event has any Empty fragments
	int empty_fragment_count_this_event = 0;
	std::set<int> empty_fragmentID_list_this_event;
	for (auto const& hndl : fragmentHandles)
	{
		std::string instance_name = hndl.provenance()->productInstanceName();
		std::size_t found = instance_name.find("Empty");
		if (found != std::string::npos)
		{
			empty_fragment_count_this_event += hndl->size();

			// track the number of empty fragments by fragment ID
			for (auto const& fragment : *hndl)
			{
				int fragID = fragment.fragmentID();
				if (empty_fragments_by_fragmentID_.count(fragID) == 0)
				{
					empty_fragments_by_fragmentID_[fragID] = 1;
          empty_fragments_by_subsystem_[ fragID & 0xF000 ] = 1;
				}
				else
				{
					empty_fragments_by_fragmentID_[fragID] += 1;
          empty_fragments_by_subsystem_[fragID & 0xF000 ] += 1;
				}
				empty_fragmentID_list_this_event.insert(fragID);
			}
		}
	}

	// common metric reporting for multiple modes
	if (metricMan != nullptr && (mode_bitset_.test(BASIC_COUNTS_MODE) || mode_bitset_.test(FRACTIONAL_COUNTS_MODE)))
	{
		metricMan->sendMetric("EventsProcessed", events_processed_, "events", metrics_reporting_level_,
		                      artdaq::MetricMode::LastPoint);
	}

	size_t missing_fragment_count_this_event = missing_fragmentID_list_this_event.size();
	size_t total_fragments_this_event = expected_fragmentID_list_.size() - missing_fragment_count_this_event;
	TLOG(TLVL_EVENT_SUMMARY) << "Event " << evt.event() << ": this event: total_fragments=" << total_fragments_this_event
	                         << ", missing_fragments=" << missing_fragment_count_this_event << ", empty_fragments="
	                         << empty_fragment_count_this_event << " (" << events_processed_ << " events processed)";
	// log TRACE message if there are missing fragments
	if (missing_fragment_count_this_event > 0)
	{
		std::ostringstream oss;
		bool firstLoop = true;
		for (auto const& fragID : missing_fragmentID_list_this_event)
		{
			if (!firstLoop) { oss << ", "; }
			oss << fragID;
			firstLoop = false;
		}
		TLOG(TLVL_BAD_FRAGMENTS) << "Event " << evt.event() << ": total_fragments=" << total_fragments_this_event
		                         << ", fragmentIDs for " << missing_fragment_count_this_event << " missing_fragments: " << oss.str();
	}
	// log TRACE message if there are empty fragments
	if (!empty_fragmentID_list_this_event.empty())
	{
		std::ostringstream oss;
		bool firstLoop = true;
		for (auto const& fragID : empty_fragmentID_list_this_event)
		{
			if (!firstLoop) { oss << ", "; }
			oss << fragID;
			firstLoop = false;
		}
		TLOG(TLVL_BAD_FRAGMENTS) << "Event " << evt.event() << ": total_fragments=" << total_fragments_this_event
		                         << ", fragmentIDs for " << empty_fragment_count_this_event << " empty_fragments: " << oss.str();
	}

	// reporting for the BASIC_COUNTS_MODE
	if (metricMan != nullptr && mode_bitset_.test(BASIC_COUNTS_MODE))
	{
		if (missing_fragment_count_this_event > 0) { ++events_with_missing_fragments_; }
		if (empty_fragment_count_this_event > 0) { ++events_with_empty_fragments_; }

		metricMan->sendMetric("EventsWithMissingFragments", events_with_missing_fragments_, "events",
		                      metrics_reporting_level_, artdaq::MetricMode::LastPoint);
		metricMan->sendMetric("EventsWithEmptyFragments", events_with_empty_fragments_, "events",
		                      metrics_reporting_level_, artdaq::MetricMode::LastPoint);


    // we report also the missing and empy fragments divided by subsystem
    for( auto const &mapIter : missing_fragments_by_subsystem_ ){
      
      if ( mapIter.second > 0 && missing_fragment_count_this_event >0 )
        events_with_missing_fragments_by_subsystem_[mapIter.first]++;

      std::string MetricName = "EventsWithMissingFragmentsOn"+getNameFromKey(mapIter.first);
      metricMan->sendMetric( MetricName, events_with_missing_fragments_by_subsystem_[mapIter.first], 
                                    "events", metrics_reporting_level_, artdaq::MetricMode::LastPoint );
    } 

    for ( auto const &mapIter : empty_fragments_by_subsystem_ ){
      
      if ( mapIter.second > 0 && empty_fragment_count_this_event >0 )
        events_with_empty_fragments_by_subsystem_[mapIter.first]++;

      std::string MetricName = "EventsWithEmptyFragmentsOn"+getNameFromKey(mapIter.first);
      metricMan->sendMetric( MetricName, events_with_empty_fragments_by_subsystem_[mapIter.first], 
                                                "events", metrics_reporting_level_, artdaq::MetricMode::LastPoint );
    }


		TLOG(TLVL_BASIC_MODE) << "Event " << evt.event() << ": events_with_missing_fragments=" << events_with_missing_fragments_
		                      << ", events_with_empty_fragments=" << events_with_empty_fragments_;
	}

	// reporting for the FRACTIONAL_COUNTS_MODE
	if (metricMan != nullptr && mode_bitset_.test(FRACTIONAL_COUNTS_MODE))
	{
		if (((static_cast<double>(missing_fragment_count_this_event) * 100.0) / static_cast<double>(expected_fragmentID_list_.size())) >= 10.0)
		{
			++events_with_10pct_missing_fragments_;
		}
		if (((static_cast<double>(missing_fragment_count_this_event) * 100.0) / static_cast<double>(expected_fragmentID_list_.size())) >= 50.0)
		{
			++events_with_50pct_missing_fragments_;
		}

		if (((static_cast<double>(empty_fragment_count_this_event) * 100.0) / static_cast<double>(expected_fragmentID_list_.size())) >= 10.0)
		{
			++events_with_10pct_empty_fragments_;
		}
		if (((static_cast<double>(empty_fragment_count_this_event) * 100.0) / static_cast<double>(expected_fragmentID_list_.size())) >= 50.0)
		{
			++events_with_50pct_empty_fragments_;
		}

		metricMan->sendMetric("EventsWith10PctMissingFragments", events_with_10pct_missing_fragments_, "events",
		                      metrics_reporting_level_, artdaq::MetricMode::LastPoint);
		metricMan->sendMetric("EventsWith50PctMissingFragments", events_with_50pct_missing_fragments_, "events",
		                      metrics_reporting_level_, artdaq::MetricMode::LastPoint);

		metricMan->sendMetric("EventsWith10PctEmptyFragments", events_with_10pct_empty_fragments_, "events",
		                      metrics_reporting_level_, artdaq::MetricMode::LastPoint);
		metricMan->sendMetric("EventsWith50PctEmptyFragments", events_with_50pct_empty_fragments_, "events",
		                      metrics_reporting_level_, artdaq::MetricMode::LastPoint);

		TLOG(TLVL_FRACTIONAL_MODE) << "Event " << evt.event() << ": events_with_10pct_missing_fragments=" << events_with_10pct_missing_fragments_
		                           << ", events_with_10pct_empty_fragments=" << events_with_10pct_empty_fragments_;
		TLOG(TLVL_FRACTIONAL_MODE) << "Event " << evt.event() << ": events_with_50pct_missing_fragments=" << events_with_50pct_missing_fragments_
		                           << ", events_with_50pct_empty_fragments=" << events_with_50pct_empty_fragments_;
	}

	// reporting for the DETAILED_COUNTS_MODE
	if (metricMan != nullptr && mode_bitset_.test(DETAILED_COUNTS_MODE))
	{
		// only send an update when the missing or empty fragment counts, by FragmentID, changed,
		// as indicated by a non-zero number of missing or empty fragments in this event
		if (missing_fragment_count_this_event > 0 || empty_fragment_count_this_event > 0)
		{
			std::ostringstream oss;
			oss << "<eventbuilder_snapshot app_name=\"" << app_name << "\"><events_processed>" << events_processed_
			    << "</events_processed>";
			oss << "<missing_fragment_counts>";
			for (auto const& mapIter : missing_fragments_by_fragmentID_)
			{
				oss << "<count fragment_id=" << mapIter.first << ">" << mapIter.second << "</count>";
			}
			oss << "</missing_fragment_counts>";
			oss << "<empty_fragment_counts>";
			for (auto const& mapIter : empty_fragments_by_fragmentID_)
			{
				oss << "<count fragment_id=" << mapIter.first << ">" << mapIter.second << "</count>";
			}
			oss << "</empty_fragment_counts>";
			oss << "</eventbuilder_snapshot>";

			metricMan->sendMetric("EmptyFragmentSnapshot", oss.str(), "xml_string",
			                      metrics_reporting_level_, artdaq::MetricMode::LastPoint);
		}
	}

#if 0
	==================================================== =

		event_builder_snapshot : {
	name: "EventBuilder5"
		timestamp : "20190408T124433"
		events_built : 105

		sender_list : ["felix501", "felix501", "ssp101", "ssp102"]
		valid_fragment_counts : [105, 105, 102, 104]
		empty_fragment_counts : [0, 0, 2, 0]
		missing_fragment_counts : [0, 0, 1, 1]
	}

	==================================================== =

		<event_builder_snapshot name = "EventBuilder5">
		< timestamp>20190408T124433< / timestamp>
		< events_built>105 < / events_built

		<sender_list>
		<sender index = 0>felix501< / sender>
		<sender index = 1>felix502< / sender>
		<sender index = 2>ssp101< / sender>
		<sender index = 3>ssp102< / sender>
		< / sender_list>

		<valid_fragment_counts>
		< count index = 0>105 < / count >
		< count index = 1>105 < / count >
		< count index = 2>102 < / count >
		< count index = 3>104 < / count >
		< / valid_fragment_counts>

		<empty_fragment_counts>
		< count index = 2>2 < / count >
		< / empty_fragment_counts>

		<missing_fragment_counts>
		< count index = 2>1 < / count >
		< count index = 3>1 < / count >
		< / missing_fragment_counts>
		< / event_builder_snapshot>

		==================================================== =
#endif
}

DEFINE_ART_MODULE(icarus::IcarusFragmentWatcher)  // NOLINT(performance-unnecessary-value-param)
