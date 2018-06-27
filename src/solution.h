#ifndef SOLUTION_H
#define SOLUTION_H


#include"graphe.h"

class Solution {
	Graph* graph;

public:
	Solution(){graph=NULL; tColor=NULL; nbEdgesConflict=nbNodesConflict=nbIterations=nbIterationsFirst=999999;}
	Solution(Graph* gr, int nbCol){graph=gr; tColor=NULL; initRandom(nbCol);}
	Solution(const Solution& s){ graph=NULL; tColor=NULL;  *this = s;}
	~Solution();

	void initRandom(int nbCol);

	Solution& operator=(const Solution& s);
	void computeConflicts(int tConflicts[]); // calcule le nb de conflits et remplit le tableau
	int computeConflicts();
	int proxi(Solution& sol, bool changeToBestMatching=false);
	int nbSameColor(Solution& sol);
	void proxiIS(vector<Solution>& vSolRef, vector<int>& vProxi, vector<vector<pair<int,int> > >& vClosestRefSolIS);
	void proxiIS(Solution& solRef, vector<int>& vProxi, vector<int>& vClosestRefIS, int nbColors1=-1, int nbColors2=-1);
	void decresaseNbColors();
        void breakSymmetry();
	void print();
	void save(string filename);


	int* tColor; // contient pour chaque noeud sa couleur
	int nbEdgesConflict;
	int nbNodesConflict;
	unsigned long long nbIterations; // nb d'iterations locales pour la trouver
	unsigned long long nbIterationsFirst; // nb d'iterations de la première fois qu'on la trouve
};


#endif

