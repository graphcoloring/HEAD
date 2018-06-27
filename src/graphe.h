#ifndef GRAPHE_H
#define GRAPHE_H


#include<vector>
#include<string>

#include "util/gfile.h"

using namespace std;

class Graph {
	void buildVoisins(); // cree le vecteur des voisins de chaque sommet

public:
	Graph(){filename=""; nbSommets=0; nbArretes=0; tConnect=NULL; tVoisins=NULL;}
	Graph(string fname){filename=fname; loadGraph();}
	~Graph();
	void loadGraph();
	void loadMatrixGraph(GInputFile& infile);

	int nbSommets;
	int nbArretes;
	char** tConnect; // tableau carre des connections noeud à noeud
	vector<int>* tVoisins; // tableau qui pour chaque noeud contient la liste des voisins
	string filename;
};


#endif
