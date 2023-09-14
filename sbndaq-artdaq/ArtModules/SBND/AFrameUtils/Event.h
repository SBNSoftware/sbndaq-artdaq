/**
 *
 * \This Event Class represents a single readout
 *
 *
 */

#ifndef AFRAME_EVENT_H
#define AFRAME_EVENT_H

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include "Cluster.h"
#include <TObject.h>

namespace sbndaq {

//class Event : public TObject {
class Event {

public:
        Event() {}
        ~Event() {}

	// Member Attributes
        int Run;
	int ID;
        
        // The main purpose of this class is to store Clusters of reco objects in time
	std::vector<Cluster> clusters;
	
	// Member Functions
	void EventClear() {clusters.clear();}	
	
	void SetID(int _eventID) {ID = _eventID;}
	int GetID() const {return ID;}
	
	// Run # for this event
	void SetRun(int _runID) {Run = _runID;}
	int GetRun() const {return Run;}

	int GetClusterCount() const {return clusters.size();}
	void AddCluster(Cluster cluster) {return clusters.push_back(cluster);}
	Cluster* GetCluster(int i) {return &clusters[i];}

private:
	// Probably don't need anything for this

//ClassDef(Event, 1)

};

} // end namespace sbndaq

#endif
