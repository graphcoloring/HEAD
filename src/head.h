#ifndef HEAD_H
#define HEAD_H

#include <vector>
#include "tabouSearch.h"
#include "solution.h"

using namespace std;

class Head {
  void initRandSeed();
  Solution buildChild(vector<Solution>& vParents, int startParent);
  Solution buildChild(vector<Solution>& vParents, vector<double>& vPoids);
  void printPopulation(ostream& st=std::cout);
  void buildBaseName();
  void saveConfig();
  void loadConfig(string filename);

  time_t tsaveConfig; /// Pour régulièrement enregistrer l'état du solver
  int startingRand;

 public:
  Head(){graph=NULL; startingRand=-1; maxsecondes=-1;weightParent=-1;}
  ~Head(){};

  void compute();
  void saveBestColoring(char* outputFile);

  vector<Solution> vPopulation;
  Solution bestSol;
  int proxi;
  unsigned long long nbIterations;
  int nbIterationsCross;
  int bestSolNbIterationsCross;

  Graph* graph;
  int nbColors, nbGeneration, swapIter, nbRandCross, debug;
  long long nbLocalSearch;
  double tauxAcceptWorst, weightParent, swapingRate;
  unsigned int randSeed[2];
  unsigned int userRandSeed;
  Solution* initSol;
  string startingConf="";
  int maxsecondes;
  bool tabucol;

  clock_t startTime;
  double humanTime;

  string baseName; /// Pour savoir ou enregistrer l'analyse et le configurations intermédiaires
};


#endif
