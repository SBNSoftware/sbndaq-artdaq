/**
 *
 * \This Track Class represents a reconstructed track
 *
 *
 */

#ifndef AFRAME_TRACK_H
#define AFRAME_TRACK_H

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include "Hit.h"
#include "TVector3.h"
#include <TObject.h>

namespace sbndaq {

//class AframeTrack : public TObject {
class AframeTrack {

public:
        AframeTrack() {}
        ~AframeTrack() {}

	int ID; // index of the track
        
        // Store Weighted hits as well as additional reco hits that are within the modules. Add track start, end and dir 
	
	// Weighted hits associated with this track
 	std::vector<Hit> hits;

	
	void ClearHits() {hits.clear();}
	
	int GetHitCount() const {return hits.size();}
	void AddHit(Hit hit) {return hits.push_back(hit);}
	Hit* GetHit(int i) {return &hits[i];}
	


private:
	// Probably don't need anything for this

//ClassDef(AframeTrack, 1)

};

} // end namespace sbndaq

#endif
