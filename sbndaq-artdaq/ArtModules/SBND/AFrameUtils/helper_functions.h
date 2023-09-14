#ifndef AFRAME_HELPER_FUNCTIONS_H
#define AFRAME_HELPER_FUNCTIONS_H

// Standard Library Includes
#include <iostream>
#include <fstream>
// ROOT Includes
#include "TObject.h"
#include "TVector3.h"



namespace sbndaq {


float CalculateComponent(float err1, float err2, float x1, float x2) {

	// calculate numerator
	float num = (x1/err1) + (x2/err2);
	// calculate denominator
	float den = 2.0*((1/err1) + (1/err2));
	
	// return one component of the hit position
	return num/den;
	
}

// Function to take pseudo hits and 
TVector3 CalculateHitPos(std::vector<float> _errors, TVector3 vpos, TVector3 hpos) {

	// Order should be vertical first then horizontal second
	float x = CalculateComponent(_errors.at(0), _errors.at(1), vpos.X(), hpos.X());
	float y = CalculateComponent(_errors.at(2), _errors.at(3), vpos.Y(), hpos.Y());
	float z = CalculateComponent(_errors.at(4), _errors.at(5), vpos.Z(), hpos.Z());
	TVector3 pos(x, y, z);
	return pos;
}


std::vector<float> CalculateTotalError(std::vector<float> _errors) {
	// TODO For now, assume the covariance is zero such that the uniform distribution errors add in quadrature 
	float x_err = std::sqrt( std::pow(_errors.at(0), 2) + std::pow(_errors.at(1), 2) );	
	float y_err = std::sqrt( std::pow(_errors.at(2), 2) + std::pow(_errors.at(3), 2) );	
	float z_err = std::sqrt( std::pow(_errors.at(4), 2) + std::pow(_errors.at(5), 2) );	

	std::vector<float> err;
	err.push_back(x_err); err.push_back(y_err); err.push_back(z_err);
	return err;

}


} // end namespace sbndaq

#endif


