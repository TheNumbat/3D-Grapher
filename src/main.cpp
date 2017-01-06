
#include "state.h"

// Must do before useable : error messages, static cam bug, help menu

// TODO:
	// Bugs
		// Rects have to be set every frame
		// Something is wrong with static 3d cam when not at 0, 0, 0
	// UI
		// Error messages
			// Unknown function name
			// Unused variable
			// Unbalanced parenthesis
			// Unable to parse value
			// ANYWHERE in()/eval()
	// Math Features
		// Graph types
		// Highlight curve along a set x/y
		// Partials
		// Graph intersections
		// Level Curves
		// E Regions
		// Tangent Planes
		// More than urnary functions (e.g. max, min)
		// Maxs & mins (abs and rel)
		// Line/Surface/Flux integrals

int main(int argc, char** args) {

	state st;
	st.run();

	return 0;
}
