#ifndef AFRAME_CHANNEL_MAP_H
#define AFRAME_CHANNEL_MAP_H

// Standard Library Includes
#include <iostream>
#include <fstream>
// ROOT Includes
#include "TObject.h"
#include "TVector3.h"



/*

  Class to configure the Aframe geometry and calculate positions within single modules 
  

*/

//namespace AFrame {
namespace sbndaq {


class ChannelMap {

public:
        ChannelMap() {}
        ~ChannelMap() {}

        // below are vectors from the config file
        std::vector<int> mac5;
        std::vector<float> xpos;
        std::vector<float> top_measure;
        std::vector<float> bottom_measure;
        std::vector<int> FEB_dir; // is it facing towards you or away from you  
        std::vector<int> channel_ascending; // are the channels ascending from the bottom of the A-frame upwards 
        // in the case of vertical modules, are the channels ascending away from you when looking at the A-Frame

        // We can put cable delays here if necessary 
        //std::vector<int> cable_delays;

 	// below are vectors to be formed from the config info --> used in position mapping
        std::vector<float> zpos;
        std::vector<float> angle;


        void initialize_config(const std::string& config_file);

        // Clear the vector members of the ChannelMap
        void ChannelMapClear();

        // Initialize the zpos and angle vectors after extracting the config file info
        void CalculateParams(std::vector<float> _top_measure, std::vector<float> _bottom_measure);

        int GetFEBIndex(int _mac5);

        int is_Horiz(int _mac5);
	int is_Outer(int _mac5);
	int is_Left(int _mac5);
        float GetHypotenuse(int ascend, int _strip_h, int is_horiz);
        float Tanh_Method(int adcA, int adcB, int is_horiz);
        TVector3 CalculatePosHoriz(int _mac5, int strip, int adcA, int adcB, int mode);
        TVector3 CalculatePosVert(int _mac5, int strip, int adcA, int adcB, int mode);

        float GetDistToFEB(int _FEB, TVector3 _pos);

	// Add Error calculation which only needs the horizontal FEB id
	std::vector<float> CalculateError(int _mac5_v, int _mac5_h);



private:
        // Put things like module dimensions here
        // These Module dimensions are for the Y and Bern Modules
        float module_width_h = 180.0; // cm
        float module_width_v = 95.9; // cm
        float module_length_h = 450.0; // cm
        float module_length_v = 272.2; // cm

        float width_offset = 25.0; // cm
        float strip_w_h = 11.2; // cm
        float strip_w_v = 5.95; // cm
        float strip_l_h = 449.2; // cm TODO Note: this is for the Y-modules
        float strip_l_v = 271.5; // cm
        float strip_t = 1.0; // cm --> This thickness is the same for X, Y, and Bern Modules
        float alumin_t = 0.4; // TODO assume symmetric for now

// Required for ROOT I/O with TTree and TFile                                                                                                                                      
//ClassDef(ChannelMap, 1)   // Class holding AFrame Channel Mapping for positions


};

} // end namespace AFrame

#endif


