#pragma once
#include <vector>

enum nodetype {
	INPUT, HIDDEN, OUTPUT
};


class connection;

class node {
public:
	node() {
		output = 0;
		prevOutput = 0;
		calculated = false;
		visited = false;
	}
	float x, y;
	float drawX, drawY;
	std::vector<connection*> incoming;
	std::vector<connection*> outgoing; // may not need this. consider removing this.
	bool calculated;
	float output, prevOutput;
	bool visited; // needed for the isRecur method
	nodetype type;
	int id; // unique identifier for node

	float evaluate();
};

class connection {
public:
	connection(node* n1, node* n2, bool r = false) {
		in = n1;
		out = n2;
		enabled = true;
		recur = r;
	}
	double weight;
	node *in, *out;
	bool enabled;
	int innoNum;
	bool recur;
};




