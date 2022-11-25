#pragma once

#include <vector>


class innovation {
public:
	innovation(int in, int out, int c) : inNode(in), outNode(out), con(c) { newnodes = std::vector<int>(); }

	int inNode, outNode;
	int con;
	std::vector<int> newnodes;
	static int numNodes;
	static int numCons;
};

