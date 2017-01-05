﻿
#include "state.h"

// Must do before useable : error messages, static cam bug, help menu

// TODO:
	// Bugs
		// Rects have to be set every frame
		// Something is wrong with static 3d cam when not at 0, 0, 0
		// Lower resolution messes with cylindrical
	// Graph types
	// UI
		// Error messages
			// Unknown function name
			// Unused variable
			// Unbalanced parenthesis
			// Unable to parse value
			// ANYWHERE in()/eval()
	// Math Features
		// Highlight curve along a set x/y
		// Partials
		// Graph intersections
		// Level Curves
		// Vector feilds
		// E Regions
		// Tangent Planes
		// Polar & Spherical Graphs
		// More than urnary functions (e.g. max, min)
		// Maxs & mins (abs and rel)
		// Parametric Lines & Surfaces
		// Line/Surface/Flux integrals

int main(int argc, char** args) {

	state st;
	st.run();

	return 0;
}
