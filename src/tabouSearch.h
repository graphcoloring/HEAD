#ifndef TABOU_SEARCH_H
#define TABOU_SEARCH_H

#include"graphe.h"
#include"solution.h"

class TabouSearch {
	// parametres pour la durée Tabou
	static constexpr double lambda=0.6;
	static constexpr int L=10;

	Graph* graph;
	int nbColors;
	int nbLocalSearch;

	int* tConflicts; // contient pour chaque sommet le nombre de conflits
	int** tNewConflitsWithColor; // contient pour chaque noeud ne nb de conflis en plus ou moins si changement vers chaque couleur, suivi de la meilleure couleur et meilleur gain associé
	int** tTabou; // contient pour chaque noeud et chaque couleur la fin de la période taboue

	// optimisation1 permettant de connaitre la meilleure amélioration possible pour chaque noeud
	int* tBestImprove; // contient pour chaque sommet la meilleure valeur de transition
	int** ttBestImproveColor; // contient pour chaque sommet la meilleure transition de couleur (-1 si plusieurs equivalentes => stratégie classique)
	int* tNbBestImprove; // contient le nombre pour chaque sommet de couleurs fournissant la meilleure amélioration

	// optimisation2 permettant de parcourir uniquement les noeuds avec conflits
	int* tNodeWithConflict; // contient les noeuds avec conflit pour ne pas tout parcourir
	int nbNodeWithConflict;
	int* tNodeAdded; // permet de ne pas ajouter 2 fois le meme noeud

	void initTables();
	void determineBestImprove();
	void updateTables(int node, int color);
	void resetNodeWithConflict(); // recalcule pour optimisation2


	/////
	void updateAnalyseData();
	void saveAnalyse();
	string analyseBaseName; // contient le chemin de sortie pour les analyses (chaine vide => pas d'analsye)
	/////

public:
	TabouSearch();
	TabouSearch(const TabouSearch& ts){*this=ts;}
	TabouSearch(Graph* gr, int nbColors){buildTables(gr, nbColors);}
	~TabouSearch();
	TabouSearch& operator=(const TabouSearch& ts);

	void buildTables(Graph* gr, int nbColors);
	void initNbChanges();
	void printNbChanges();
	bool compute(Solution& sol, int nbLocalSearch, string outfilename="", int nbIte=-1, int *tBlockedNode=NULL); // améliore la solution sol en effectuant nbLocalSearch iterations
	void setSolution(Solution& sol);
	void initRandSolution();


	unsigned int randSeed;
	Solution currentSol; // solution qui évolue lors de la recherche Tabou
	Solution bestSol; // garde la meilleure solution rencontrée
	int nbIterations; // iteration courante de la recherche locale

	double* tNbChanges; // tmp pour compter le nb de changement
	double* tNbConflicts; // tmp pour compter le nb de conflits par sommet
	double* tTotalBestImprove; // tmp pour compter le nb de conflits par sommet
};


#endif
