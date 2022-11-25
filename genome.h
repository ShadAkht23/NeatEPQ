#pragma once
#include <vector>
#include "gene.h"
#include "innovation.h"
#include "olcPixelGameEngine.h"

class genome{

public:
	// output nodes should not have outgoing connections

	std::vector<node*> nodes; // sorted by x value
	std::vector<connection*> connections;



	std::vector<double> evaluateNetwork(std::vector<double>& inputs);
	
	// cross over and distance
	static double distance(genome* a, genome* b);

	bool loadGenome(const char* filename);
	bool isRecur(node* from, node* to); // use BFS or DFS to implement this.

	void drawGenome(olc::PixelGameEngine* engine, int minx, int maxx, int miny, int maxy);

	// mutations
	void mutateNewNode(std::vector<innovation>& innovations);
	void mutateNewConnection(std::vector<innovation>& innovations, bool recur);
	void mutateWeightShift();
	void mutateWeightRandom();
	void mutateConnectionToggle();
};

