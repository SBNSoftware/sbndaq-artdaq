#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "TMath.h"
#include "ChannelMap.h"

using namespace std;

//namespace AFrame {
namespace sbndaq {

//ChannelMap::ChannelMap() {}
//ChannelMap::~ChannelMap() {}

void ChannelMap::initialize_config(const std::string& config_file) {


/*

        The Data in the Config File for channel Mapping

        std::vector<int> mac5;
        std::vector<float> xpos;
        std::vector<float> top_measure;
        std::vector<float> bottom_measure;
        std::vector<int> FEB_dir;  
        std::vector<int> channel_ascending;
*/

        ifstream file;
        file.open(config_file);
        string line;
        //getline(file, line);
        //while ( (getline(file, line, ',')) ) {
        int count = 0;
        while ( (getline(file, line)) && count < 6) {
                //std::cout << std::endl;
                //std::cout << "Line " << count << std::endl;

                std::string delimiter = ",";
                size_t pos = 0;
                string token;
                while ( (pos = line.find(delimiter)) != string::npos) {

                        token = line.substr(0, pos);
                        // fill the config vectors for this A-Frame setup
                        if (count == 0) {
                                mac5.push_back(stoi(token));
                        }
                        if (count == 1) {

                                xpos.push_back(std::stof(token));
                        }
                        if (count == 2) {

                                top_measure.push_back(std::stof(token));
			}
                        if (count == 3) {
                                bottom_measure.push_back(std::stof(token));
                        }
                        if (count == 4) {
                                FEB_dir.push_back(stoi(token));
                        }
                        if (count == 5) {
                                channel_ascending.push_back(stoi(token));
                        }
                        //if (count == 6) {
                        //      cable_delays.push_back(stoi(token));
                        //}
                        //std::cout << "token " << token << std::endl;
                        line.erase(0, pos + delimiter.length());
                }
                //std::cout << "line is " << line << std::endl;
                count += 1;

        }
        //std::cout << "Finished reading config file" << std::endl;
        file.close();

} // end of initialize_config function


void ChannelMap::ChannelMapClear() {

        // Clear the Config Vectors 
        mac5.clear();
        xpos.clear();
        top_measure.clear();
        bottom_measure.clear();
        FEB_dir.clear();
        channel_ascending.clear();

        // Clear the vectors used to form positions
        zpos.clear();
        angle.clear();

} // end of Clear function

void ChannelMap::CalculateParams(std::vector<float> _top_measure, std::vector<float> _bottom_measure) {
        // TODO The measurements taken at DAB are from the edges of the module 
        float middle_top = _top_measure.at(0);
        float middle_bottom = _bottom_measure.at(0);

        float z0_right = (middle_bottom / 2.0);
        float z0_left = -1*(middle_bottom / 2.0); // needs to be negative

        float z1_right = (middle_top / 2.0);
        float z1_left = -1*(middle_top / 2.0);

        int flag = 0;
        //std::cout << "End of top_meas " << top_measure.at(top_measure.size()-1) << std::endl;
        //std::cout << "End of FEB " << mac5.at(mac5.size()-1) << std::endl;
        for (long unsigned int i = 0; i < top_measure.size(); ++i) {
                //std::cout << "top_meas " << top_measure.at(i) << std::endl;
                if (i == 0) {
                        continue;
                }

                if (top_measure.at(i) < 0) {
                        //std::cout << "top measure < 0" << std::endl;
                        // add negative displacements 
                        float temp0 = bottom_measure.at(i) + z0_left;
                        float temp1 = top_measure.at(i) + z1_left;
                        zpos.push_back(temp0);

                        float temp = (abs(temp0 - temp1)/ module_width_h);
                        float alpha = TMath::ACos(temp);

                        angle.push_back(alpha);

                }
		else if (  (top_measure.at(i) > 0) && (flag == 0) ) {
                        //std::cout << "Double pushback" << std::endl;
                        zpos.push_back(z0_left);
                        zpos.push_back(z0_right);

                        float temp1 = (abs(z0_left - z1_left)/ module_width_h);
                        float temp2 = (abs(z0_right - z1_right)/ module_width_h);
                        float angle1 = TMath::ACos(temp1);
                        float angle2 = TMath::ACos(temp2);

                        angle.push_back(angle1);
                        angle.push_back(angle2);
                        flag = 1;

                        //std::cout << "top measure > 0" << std::endl;
                        float temp3 = bottom_measure.at(i) + z0_right;
                        float temp4 = top_measure.at(i) + z1_right;
                        zpos.push_back(temp3);

                        float temp = (abs(temp3 - temp4)/ module_width_h);
                        float alpha = TMath::ACos(temp);

                        angle.push_back(alpha);

                }
                else {
                        //std::cout << "top measure > 0" << std::endl;
                        float temp0 = bottom_measure.at(i) + z0_right;
                        float temp1 = top_measure.at(i) + z1_right;
                        zpos.push_back(temp0);

                        float temp = (abs(temp0 - temp1)/ module_width_h);
                        float alpha = TMath::ACos(temp);

                        angle.push_back(alpha);

                }
        }

} // end of calculate params


int ChannelMap::GetFEBIndex(int _mac5) {
        for (long unsigned int i = 0; i < mac5.size(); ++i) {
                if (mac5.at(i) == _mac5) {
                        return i;
                }
        }
	// TODO Maybe throw an exception here and have the probram abort so the user knows that the config is wrong
        std::cout << "Problem: Could not Find FEB index ???" << std::endl;
        std::cout << "FEB " << _mac5 << std::endl;
        return -5;
}

// use the fact that vertical FEBs always have FEB_dir 2 since the FEB side is at the bottom of the frame
int ChannelMap::is_Horiz(int _mac5) {
        int index = GetFEBIndex(_mac5);
        if (FEB_dir.at(index) == 2) {
                return 0;
        }
        else {
                return 1;
        }
}

//  Without the correction for light sharing
// This returns distance to the lower edge of the strip from the bottom of the A-Frame 
float ChannelMap::GetHypotenuse(int ascend, int _strip_h, int is_horiz) {
        float strip_w = 0.0;
        if (is_horiz) {
                strip_w = strip_w_h;
}
        else {
                strip_w = strip_w_v;
        }

        if (ascend == 1) {
                float d = alumin_t + strip_w*_strip_h;
                return d;
        }
        else {
                float d = alumin_t + (16 - _strip_h)*strip_w - (strip_w/2.0);
                return d;
        }
}

float ChannelMap::Tanh_Method(int adcA, int adcB, int is_horiz) {
        float strip_w;
        if (is_horiz) {
                strip_w = strip_w_h;
        }
        else {
                strip_w = strip_w_v;
        }
        //d1 = (strip_w/2) + (strip_w/2)*np.tanh(np.log(peB/peA))
        float corr = (strip_w/2.0) + (strip_w/2.0)*TMath::TanH(TMath::Log( static_cast<float>(adcB)/adcA ));
        return corr;
}

TVector3 ChannelMap::CalculatePosHoriz(int _mac5, int strip, int adcA, int adcB, int mode) {
        int h_index = ChannelMap::GetFEBIndex(_mac5);

        float d = GetHypotenuse(channel_ascending.at(h_index), strip, 1);
        //std::cout << "Calculated d" << std::endl;
        float corr = 0.0;
        if (mode == 1) { // Tanh Weighting
                if (channel_ascending.at(h_index) == 1) {
                        corr = Tanh_Method(adcA, adcB, 1);
                }
                else {
                        corr = strip_w_h - Tanh_Method(adcA, adcB, 1);

                }
                d += corr; // add the correction from TanH weighting            
        }
        else {
                d += (strip_w_h /2.0); // center of the strip   
        }



        float y = d*TMath::Sin(angle.at(h_index)); // the y-position of the horizontal module hit
        float z0 = zpos.at(h_index); // position from the origin to the bottom of this module
        float z1 = d*TMath::Cos(angle.at(h_index));
        float z = -9999.0;
        if (z0 < 0) {
                z = z0 + z1;
        }
        else {
                z = z0 - z1;
        }
        //std::cout << "calculated xyz Horiz" << std::endl;

        TVector3 pos(0.0, y, z); // set the x-position to zero which is the center of the horizontal modules for now until higher-level reco
        return pos;

} // End of horizontal position calculator 


TVector3 ChannelMap::CalculatePosVert(int _mac5, int strip, int adcA, int adcB, int mode) {
        long unsigned int v_index = ChannelMap::GetFEBIndex(_mac5);
        float d = GetHypotenuse(channel_ascending.at(v_index), strip, 0);
        //std::cout << "Calculated d" << std::endl;
        float corr = 0.0;
        if (mode == 1) { // Tanh Weighting
                if (channel_ascending.at(v_index) == 1) {
                        corr = Tanh_Method(adcA, adcB, 0);
                }
                else {
                        corr = strip_w_v - Tanh_Method(adcA, adcB, 0);

                }
                d += corr; // add the correction from TanH weighting            
        }
        else {
                d += (strip_w_v /2.0); // center of the strip   
        }
        float x = -9999.0;
        //std::cout << "v_index " << v_index << std::endl;
        //std::cout << "z.size() " << zpos.size() << std::endl;
        //std::cout << "z[index] " << zpos.at(v_index) << std::endl;
        //std::cout << "x[index] " << xpos.at(v_index) << std::endl;
        if (zpos.at(v_index) > 0) {
                // We have a vertical on the right (when facing the primary side of the A-Frame)
                if (v_index == mac5.size() - 2) {
                        // We have the vertical on the right closest to us
                        x = -1*( (module_length_h/2) - xpos.at(v_index) - d );

                }
                else {
                        // We have the vertical on the right farthest away from us
                        x = (module_length_h/2) - xpos.at(v_index) - (module_width_v - d);

                }
        }
        else {
                // We have a vertical on the left (when facing the primary side of the A-Frame)
                if (v_index == 0) {
                        // We have the vertical on the left closest to us
                        x = -1*( (module_length_h/2) - xpos.at(v_index) - d );
                }
                else {
                        // We have the vertical on the left farthest from us
                        x = (module_length_h/2) - xpos.at(v_index) - (module_width_v - d);
                }

        }
        //std::cout << "Calculated x" << std::endl;

        float y = (module_length_v/2)*TMath::Sin(angle.at(v_index));

        //std::cout << "Calculated y" << std::endl;     

        float z = -9999.0;
        if (zpos.at(v_index) > 0) {
                z = zpos.at(v_index) - ( (module_length_v/2)*TMath::Cos(angle.at(v_index)) );
        }
        else {
                z = zpos.at(v_index) + ( (module_length_v/2)*TMath::Cos(angle.at(v_index)) );

        }
        //std::cout << "Calculated z" << std::endl;     

        //std::cout << "calculated xyz Vert" << std::endl;
        TVector3 pos(x, y, z);
        return pos;

} // end of vertical position calculator        


// Give a reconstructed hit position--> returns the distance to the FEB side of the module
float ChannelMap::GetDistToFEB(int _FEB, TVector3 _pos) {
        int index = GetFEBIndex(_FEB);
        int is_horiz = is_Horiz(_FEB);
        int dir = FEB_dir.at(index);
        // Note if dir == 1 the horizontal FEB is facing you
        if (is_horiz) {
                float x = _pos.X();
                if (dir == 1) {
                        float d = (module_length_h/2) + x;
                        return d;
                }
                else {
                        float d = (module_length_h/2) - x;
                        return d;
                }
        }
        else {
                float d = _pos.Y()/TMath::Sin(angle.at(index));
                return d;
        }
}

} // end namespace AFrame









