#include "genome.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <ifstream>

std::vector<double> genome::evaluateNetwork(std::vector<double>& inputs)
{
	for (node* n : nodes) {
		n->calculated = false;
	}
	
	for (node* n : nodes) {
		if (n->type == OUTPUT) {
			n->evaluate();
		}
	}

}

void genome::mutateNewNode(std::vector<innovation>& innovations)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<> dis(0,connections.size()-1);
		
	connection* con1 = connections[dis(mt)]; // select a random connection

	// new structures being created
	node* newnode = new node();
	connection* con2, * con3;
	con2 = new connection(con1->in, newnode);
	newnode->incoming.push_back(con2);
	con3 = new connection(newnode, con1->out);
	newnode->outgoing.push_back(con3);
	newnode->type = HIDDEN;
	// coordinates
	newnode->x = (con1->in->x + con1->out->x) / 2;
	std::uniform_real_distribution<> dis(0, 0.1); // vary the y coordinate a little bit
	newnode->y = (con1->in->y + con1->out->y) / 2 + dis(mt) - 0.05;
	// set weights
	con2->weight = 1.0f;
	con3->weight = con1->weight;

	// finding innovation for newnode
	// auto here is some iterator
	auto i = std::find_if(innovations.begin(), innovations.end(), [&](innovation i) {
		return i.inNode == con1->in->id && i.outNode == con1->out->id;  }); // this requires some serious testing.
	if (i != innovations.end()) { // found an existing innovation. do not increment innovation.
		if (i->newnodes.empty()) { // newnode is new. create a new innovation for it.
			newnode->id = innovation::numNodes + 1;
			innovation::numNodes += 1;
		}
		else { // there could be multiple possible new nodes
			int counter = 0;
			for (int possibleNode : i->newnodes) {
				if (std::find_if(con1->in->outgoing.begin(), con1->in->outgoing.end(), [&](connection* c) {
					return c->out->id == possibleNode && c->enabled; }) != con1->in->outgoing.end()) {
					// found one
					counter++;
				}
			}
			if (counter == i->newnodes.size()) {
				// need to create a new innovation for the node
				innovation::numNodes++;
				i->newnodes.push_back(innovation::numNodes);
				newnode->id = innovation::numNodes;
			}
			else {
				// no need to create a new innovtation for the node
				newnode->id = i->newnodes[counter];
			}
		}
	}
	else {
		std::cout << "ERROR in genome::mutateNewNode(): connection to be split has no innovation\n";
		while (1);
	}
	// make recur
	con2->recur = false; // this is not how they do it but this makes more sense.
	con3->recur = con1->recur; 
	con1->enabled = false;
	newnode->incoming.push_back(con2);
	newnode->outgoing.push_back(con3);

	nodes.push_back(newnode);

	// connnections must be sorted by innovation number.
	// check if both connections are in the innovations array.
	bool done1 = false;
	bool done2 = false;

	for (innovation& i : innovations) {
		if (!done1 && i.inNode == con2->in->id && i.outNode == con2->out->id) {
			con2->innoNum = i.con;
			done1 = true;
		}
		else if (!done2 && i.inNode == con3->in->id && i.outNode == con3->out->id) {
			con3->innoNum = i.con;
			done2 = true;
		}
	}
	if (!done1) {
		// must create new innovation
		innovations.push_back(innovation(con2->in->id, con2->out->id, innovation::numCons + 1));
		innovation::numCons++;
	}
	if (!done2) {
		// must create new innovation
		innovations.push_back(innovation(con3->in->id, con3->in->id, innovation::numCons + 1));
		innovation::numCons++;
	}
	addSorted(connections, con2);
	addSorted(connections, con3);
	// might not need to loop twice if con3.inno is always con2.inno + 1

	innovation::numCons += 2;
}

void addSorted(std::vector<connection*> connections, connection* con) {
	std::vector<connection*>::iterator iter = connections.begin();
	for (iter; iter != connections.end(); iter++) {
		if ((*iter)->innoNum > con->innoNum) {
			break;
		}
	}
	connections.insert(iter, con);
}

void genome::mutateNewConnection(std::vector<innovation>& innovations, bool recur) {

	// find 2 new nodes - check if recur. check onenote
	std::random_device rd();
	std::mt19937 mt(rd);
	std::uniform_int_distribution<> dis(0, nodes.size()-1);
	
	int tries = 0;
	bool found = false;
	while (!found && tries < 20) {
		// extract nodes
		node* node1 = nodes[dis(mt)];
		node* node2 = nodes[dis(mt)];
		if (node1->type == node2->type && !node1->type == HIDDEN) {
			// found invalide connection - try again
			tries++;
			continue;
		}
		// check if connection node1->node2 exists
		if (std::find_if(node1->outgoing.begin(), node1->outgoing.end(), [&](connection* c) {
			return c->out->id == node2->id; }) != node1->outgoing.end()) {
			// node1->node2 exists. check if node2->node1 exists
			if (std::find_if(node2->outgoing.begin(), node2->outgoing.end(), [&](connection* c) {
				return c->out->id == node1->id; }) != node2->outgoing.end()) {
				// node2->node1 exists. can't create a connection
				tries++;
				continue;
			}
			else {
				// can create connection between node2->node1;
				// instead of code duplication, swapping them is acceptable
				std::swap(node1, node2);
			}
		}
		else {
			// create connection between node1->node2
		}
		bool isrecur = isRecur(node1, node2);
		if (isrecur) {
			if (!recur)  {
				// not allowed recur connections. find another one
				tries++;
				continue;
			}
		}
		// if got to here, create connection
	
		if (tries = 20) return;
		connection* con1 = new connection(node1, node2, isrecur);
		std::uniform_real_distribution<> weightDis(-1, 1);
		con1->weight = weightDis(mt);
		// find innovation
		auto iter = std::find_if(innovations.begin(), innovations.end(), [&](innovation i) {
			return i.inNode == node1->id && i.outNode == node2->id; });
		if (iter != innovations.end()) {
			// innovation already exists
			con1->innoNum = iter->con;
		}
		else {
			con1->innoNum = innovation::numCons;
			// create a new innovation
			innovations.push_back(innovation(node1->id, node2->id, innovation::numCons));
			innovation::numCons++;
		}
		// add the connection to the node vectors
		node1->outgoing.push_back(con1);
		node2->incoming.push_back(con1);
		addSorted(connections, con1);
		return;
	}
	
}

void genome::mutateWeightShift()
{
	std::random_device rd;
	std::mt19937 mt(rd);
	std::uniform_real_distribution<> shift(-0.2, 0.2); // do some math with the sigmoid and see on average how much it changes

	std::uniform_int_distribution<> index(0, connections.size()- 1);
	connections[index(mt)]->weight += shift(mt);
}

void genome::mutateWeightRandom()
{
	std::random_device rd;
	std::mt19937 mt(rd);
	std::uniform_real_distribution<> shift(-1, 1); // do some math with the sigmoid and see on average how much it changes

	std::uniform_int_distribution<> index(0, connections.size() - 1);
	connections[index(mt)]->weight = shift(mt);
}

void genome::mutateConnectionToggle()
{
	std::random_device rd;
	std::mt19937 mt(rd);
	std::uniform_int_distribution<> index(0, connections.size() - 1);
	connections[index(mt)]->enabled = !connections[index(mt)]->enabled;
}

bool search(node* origin, node* dest) {
	if (origin->id == dest->id) return true;
	for (connection* c: origin->outgoing){
		if (!c->out->visited) {
			if (search(c->out, dest)) return true;
		}
	}
	return false;
}
 
bool genome::loadGenome(const char* filename)
{
	std::ifstream file;
	file.open(filename);
	std::string line;
	
	while (getline(file, line)) {
		std::stringstream ss(line);

	}
	return false;
}

bool genome::isRecur(node* from, node* to) { // checks if 2 nodes would form a recursive connection
	for (node* n : nodes) {
		n->visited = false;
	}
	from->visited = true;
	return search(from, to);
}

void genome::drawGenome(olc::PixelGameEngine* engine, int minx, int maxx, int miny, int maxy) {

	for (node* node)

}




