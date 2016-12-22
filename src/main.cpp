
#include <iostream>
#include <fstream>
#include <string>

#include "state.h"

using namespace std;

// TODO:
	// UI
		// Improve UI
		// Settings
		// Axis scales
	// Rendering
		// Transparency, blending, maybe sorting
		// Lighting
		// Antialiasing
	// Math Features
		// Axis normalization
		// Highlight curve along a set x/y
		// Partials
		// Graph intersections
		// Level Curves
		// Vector feilds
		// 2D and 4D graphs
		// E Regions
		// Tangent Planes
		// Polar & Spherical Graphs
		// More functions (sec, csc, cot, max, min)
		// Negatives & remove asterisks
		// Maxs & mins (abs and rel)
		// Parametric Lines & Surfaces
		// Line/Surface/Flux integrals
	// Notes
		// To encode binary file as data array: xxd -i infile.bin outfile.h

int main(int argc, char** args) {

	state st;
	st.run();

	return 0;
}
