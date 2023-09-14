/**
 *
 * \This Cluster Class represents a grouping of reco objects in time
 *
 *
 */

#ifndef AFRAME_CLUSTER_H
#define AFRAME_CLUSTER_H

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include "Hit.h"
#include "AframeTrack.h"
#include <TObject.h>

namespace sbndaq {

//class Cluster : public TObject {
class Cluster {

public:
        Cluster() {}
        ~Cluster() {}

	int ID; // index of the cluster
        
        // Store Weighted hits as well as pseudo hit info from the ChannelMap class
	std::vector<Hit> hits;

	// Use the Weighted hits and pseudo hit info to for track objects with the TrackMaker Class
	std::vector<AframeTrack> tracks;
	
	void ClearHits() {hits.clear();}
	void ClearTracks() {tracks.clear();}
	
	void SetID(int _clusterID) {ID = _clusterID;}
	int GetID() const {return ID;}
	
	int GetHitCount() const {return hits.size();}
	void AddHit(Hit hit) {return hits.push_back(hit);}
	Hit* GetHit(int i) {return &hits[i];}
	
	int GetTrackCount() const {return tracks.size();}
	void AddTrack(AframeTrack track) {return tracks.push_back(track);}
	AframeTrack* GetTrack(int i) {return &tracks[i];}


private:
	// Probably don't need anything for this
//ClassDef(Cluster, 1)

};

} // end namespace sbndaq

#endif
