#include "gene.h"

float node::evaluate() {
	if (calculated) return output;
	float sum = 0;
	for (connection* con : incoming) {
		if (con->recur) {
			sum += con->in->prevOutput * con->weight;
		}
		else {
			sum += con->in->evaluate() * con->weight;
		}
	}
	calculated = true;
	return sum / (2 + 2 * abs(sum)) + 0.5;
	// put output through sigmoid.
}