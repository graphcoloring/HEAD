
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>

#include "util/gfile.h"
#include "head.h"


#define ANALYSE 0

using namespace std;

void Head::compute(){
  startTime=clock();
  struct timeval tv;
  gettimeofday(&tv, NULL);
  humanTime = (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0;

  buildBaseName();
  initRandSeed();
  tsaveConfig=time(NULL);

  vPopulation.resize(4); // 2 pour la population courante, 2 pour l'archive

  nbIterations=nbIterationsCross=bestSolNbIterationsCross=0;

  if (startingConf!=""){ /// case where we start from a previous configuration
    printf("Initial configuration load: %s\n", startingConf.c_str());
    loadConfig(startingConf);
  }
  else { /// case by default: random initilization
    vPopulation[0] = Solution(graph, nbColors);
    vPopulation[1] = Solution(graph, nbColors);
    vPopulation[2] = Solution(graph, nbColors); ///  individu elite1
    vPopulation[3] = Solution(graph, nbColors); ///  individu elite2
    bestSol=vPopulation[0];
  }
  vector<Solution> vFils(2);
  vector<TabouSearch> vTs(2, TabouSearch(graph, nbColors));

  int seuil = swapingRate*graph->nbSommets;
  proxi=0;
  bool found=false;
  int currentElite=0;

  vTs[0].randSeed = randSeed[0];
  vTs[1].randSeed = randSeed[1];

  if (tabucol){
    vTs[0].compute(vPopulation[0], nbLocalSearch);
    bestSol=vPopulation[0];
    nbIterations = bestSol.nbIterations;
  }
  else {
    while (!found && proxi<seuil && (nbGeneration<0 || nbIterationsCross<nbGeneration) && (difftime(time(NULL), humanTime)<maxsecondes || maxsecondes<0)) {
      //////// Save running state (each 10 minutes)
      if(difftime(time(NULL), tsaveConfig)>=600) // 600 secondes / 10 minutes
        saveConfig();

      nbIterationsCross++;

      vFils[0]=buildChild(vPopulation, 0);
      vFils[1]=buildChild(vPopulation, 1);

#pragma omp parallel for
      for (int i=0; i<2; i++) {
        vTs[i].compute(vFils[i], nbLocalSearch);
      }

      for (int i=0; i<2; i++) {
        nbIterations+=vTs[i].currentSol.nbIterations;
        if (vFils[i].nbEdgesConflict==0)
          found=true;
      }
      for (int i=0; i<2; i++) {
        if (vFils[i].nbEdgesConflict<bestSol.nbEdgesConflict){
          bestSol=vFils[i];
          bestSolNbIterationsCross=nbIterationsCross;
        }
        if (vFils[i].nbEdgesConflict<=vPopulation[i].nbEdgesConflict || rand()/(double)RAND_MAX<tauxAcceptWorst )
          vPopulation[i]= vFils[i];
        if (vFils[i].nbEdgesConflict<=vPopulation[2+currentElite].nbEdgesConflict)
          vPopulation[2+currentElite] = vFils[i];
      }

      proxi=vPopulation[0].proxi(vPopulation[1]);

      if (debug) {
        printPopulation();
      }

      if (swapIter>0 && (nbIterationsCross%swapIter==0 || proxi >= seuil)) {
        currentElite = (currentElite+1)%2;

        int indivToReplace = rand()/(double)RAND_MAX * 2;
        if(vPopulation[(indivToReplace+1)%2].proxi(vPopulation[2+currentElite]) >= seuil) /// if elite and indivToReplace are too similar, choose the other individual for the replacement
          indivToReplace=(indivToReplace+1)%2;

        vPopulation[indivToReplace]=vPopulation[2+currentElite];
        vPopulation[2+currentElite].nbEdgesConflict=999999;

        proxi=vPopulation[0].proxi(vPopulation[1]);

        if (debug)
          printf("\nSwap\n");
      }
    }
  }
}





///////////////////////////////////////
/////////////   HEAD PRIME   //////////
///////////////////////////////////////


void Head::initRandSeed(){
    if(startingRand==-1){
        srand(time(NULL));
        startingRand=rand();
    }
    unsigned int rdSeed=(userRandSeed>0) ? userRandSeed : time(NULL)+startingRand;
    srand(rdSeed);
    randSeed[0] = rdSeed;
    randSeed[1] = rdSeed+1;
}



/// Crossover operator: GPX algorithm
/// Parents are tColor (couleur de chaque sommet)
Solution Head::buildChild(vector<Solution>& vParents, int startParent){
	///// teste s'il faut accorder un poids différent à chaque parent
	if (weightParent>=0) {
		vector<double> vPoids(2);
		vPoids[0]=weightParent;
		vPoids[1]=1-weightParent;
		return buildChild(vParents, vPoids);
	}

	Solution res(graph, nbColors);

	int nbParents = 2;
	double tSizeOfColors[nbParents][nbColors];

	for (int i=0; i<nbParents; i++) {
		for (int j=0; j<nbColors; j++){
			tSizeOfColors[i][j]=0;
		}

		for (int j=0; j<graph->nbSommets; j++) {
			tSizeOfColors[i][vParents[i].tColor[j]]++;
		}
	}

	for (int i=0; i<graph->nbSommets; i++)
		res.tColor[i]=-1;

	double valMax;
	int colorMax;

	for (int i=0; i<nbColors; i++) {
		int indice=(startParent+i)%nbParents;
		Solution& currentParent = vParents[indice];
		double* currentSizeOfColors = tSizeOfColors[indice];
		valMax=-1;
		colorMax=-1;

		if (i<nbRandCross) {
			int startColor=rand()/(double)RAND_MAX * nbColors;
			for (int j=0; j<nbColors && colorMax<0; j++) {
				int color=(startColor+j)%nbColors;
				double currentVal = currentSizeOfColors[color];
				if (currentVal>0) {
					valMax=currentVal;
					colorMax=color;
				}
			}
		}
		else {
			int startColor=rand()/(double)RAND_MAX * nbColors;
			for (int j=0; j<nbColors; j++) {
				int color=(startColor+j)%nbColors;
				double currentVal = currentSizeOfColors[color];

				if (currentVal>valMax) {
					valMax=currentVal;
					colorMax=color;
				}
			}
		}

		for (int j=0; j<graph->nbSommets; j++) {
			if (currentParent.tColor[j]==colorMax && res.tColor[j]<0) {
				res.tColor[j]=i;

				for (int k=0; k<nbParents; k++) {
					tSizeOfColors[k][vParents[k].tColor[j]]--;
				}
			}
		}
	}

	int nbNotAttribute=0;
	for (int i=0; i<graph->nbSommets; i++) {
		if (res.tColor[i]<0) {
            nbNotAttribute++;
			res.tColor[i]=(rand()/(double)RAND_MAX) * nbColors ;
		}
	}
	return res;
}


/// Crossover operator: weighted parents for the unbalance crossover
Solution Head::buildChild(vector<Solution>& vParents, vector<double>& vPoids){
	Solution res(graph, nbColors);

	int nbParents = 2;
	int tSizeOfColors[nbParents][nbColors];

	for (int i=0; i<nbParents; i++) {
		for (int j=0; j<nbColors; j++){
			tSizeOfColors[i][j]=0;
		}

		for (int j=0; j<graph->nbSommets; j++) {
			tSizeOfColors[i][vParents[i].tColor[j]]++;
		}
	}

	for (int i=0; i<graph->nbSommets; i++)
		res.tColor[i]=-1;

	double valMax;
	int colorMax;

	double totalPoids=0;
	for (unsigned int i=0; i<vPoids.size(); i++) {
		totalPoids+=vPoids[i];
	}

	for (int i=0; i<nbColors; i++) {
		double randVal=rand()/(double)RAND_MAX * totalPoids - vPoids[0];
		int indice=0;
		while (randVal>=0) {
			indice++;
			randVal-=vPoids[indice];
		}

		Solution& currentParent = vParents[indice];
		int* currentSizeOfColors = tSizeOfColors[indice];
		valMax=-1;
		colorMax=-1;


		if (i<nbRandCross) {
			int startColor=rand()/(double)RAND_MAX * nbColors;
			for (int j=0; j<nbColors && colorMax<0; j++) {
				int color=(startColor+j)%nbColors;
				double currentVal = currentSizeOfColors[color];
				if (currentVal>0) {
					valMax=currentVal;
					colorMax=color;
				}
			}
		}
		else {
			int startColor=rand()/(double)RAND_MAX * nbColors;
			for (int j=0; j<nbColors; j++) {
				int color=(startColor+j)%nbColors;
				double currentVal = (double)currentSizeOfColors[color];

				if (currentVal>valMax) {
					valMax=currentVal;
					colorMax=color;
				}
			}
		}


		for (int j=0; j<graph->nbSommets; j++) {
			if (currentParent.tColor[j]==colorMax && res.tColor[j]<0) {
				res.tColor[j]=i;

				for (int k=0; k<nbParents; k++) {
					tSizeOfColors[k][vParents[k].tColor[j]]--;
				}
			}
		}
	}

	for (int i=0; i<graph->nbSommets; i++) {
		if (res.tColor[i]<0) {
			res.tColor[i]=(rand()/(double)RAND_MAX) * nbColors ;
		}
	}

	return res;
}







///////////////////////////////////////
/////////////  PRINT - SAVE  //////////
///////////////////////////////////////


void Head::printPopulation(ostream& st){
	st<<"k-"<<nbColors<<" "<<nbIterationsCross<<": ";
	st<<"fitness="<<vPopulation[0].nbEdgesConflict<<" "<<vPopulation[1].nbEdgesConflict<<" ("<<vPopulation[2].nbEdgesConflict<<" "<< vPopulation[3].nbEdgesConflict<<")\t";
    st<<"\tprox="<<proxi;
	st<<" best="<<bestSol.nbEdgesConflict<<"(it"<<bestSolNbIterationsCross<<")"<<endl;
}

void Head::saveBestColoring(char* outputFile){
  FILE *f;
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  f = fopen(outputFile, "a");
  if (f!=NULL){
    printf("Save coloring in : %s\n", outputFile);
    fprintf(f, "# Finish with %d edge(s) in conflict\n", bestSol.nbEdgesConflict);
    fprintf(f, "#%s#NbLS : %llu\tNbCross: %d\tTime(min): %0.2f ls%lld\tswap%d\trandCross%d\tweightOfParent%0.0f\tacceptWorst%f\tSeed : %u\n", asctime (timeinfo), nbIterations, nbIterationsCross, (clock() - startTime) / (double)CLOCKS_PER_SEC / 60. , nbLocalSearch, swapIter,nbRandCross,weightParent*100,tauxAcceptWorst,randSeed[0]);
    for (int i=0; i<graph->nbSommets; i++) {
      fprintf(f, "%d ", bestSol.tColor[i]);
    }
    fprintf(f, "\n");
    fclose(f);
  }
  else
    printf("Problem to save in file : %s\n", outputFile);
}

void Head::saveConfig(){
    tsaveConfig=time(NULL);

    char computerName[255];
    gethostname(computerName, 255);

    ostringstream oss;
	oss<<"."<<computerName<<"_"<<baseName<<"_save.txt" ;
    string filename=oss.str();
    FILE *f;

    if(debug){
        printf("Save in %s\n", filename.c_str());
    }

    f = fopen(filename.c_str(), "w");

    if(!f){
        printf("File %s can not be open.\n", filename.c_str());
    }
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    fprintf (f, "#%s", asctime (timeinfo) );
    
    oss.str("");
    printPopulation(oss);
    fprintf(f, "#Current state : %s\n\n", oss.str().c_str());
	fprintf(f, "#Parameters :\n");
	fprintf(f, "%s\t%d\t%lld\t%d\t%d\t%d\t%f\t%f\n\n", graph->filename.c_str(), nbColors, nbLocalSearch, nbGeneration, nbRandCross, swapIter, tauxAcceptWorst, weightParent*100);
    fprintf(f, "#Iteration / cross / bestCross:\n");
    fprintf(f, "%llu\t%d\t%d\n\n",nbIterations, nbIterationsCross, bestSolNbIterationsCross);

    double elapsedTime=(clock()-startTime) / (double)CLOCKS_PER_SEC/60.0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double humanT = ((double) tv.tv_sec + (double) tv.tv_usec / 1000000.0 - humanTime)/60.0;
    fprintf(f, "#cpu time / humain time:\n");
    fprintf(f, "%f\t%f\n\n",elapsedTime, humanT);
    fclose(f);

    for(unsigned int i=0; i<vPopulation.size(); i++)
        vPopulation[i].save(filename);

    f = fopen(filename.c_str(), "a");
    fprintf(f, "# BestSolutions\n");
    fclose(f);
    bestSol.save(filename);
}


void Head::loadConfig(string filename){
    GInputFile infile(filename);
    infile.open();
    //char* buf;
    //char* tok;

    /// Paramètres (sont chargés dans le main)
    infile.readUncommentedLine(); // Paramètres

    /// Starting iteration
    infile.readUncommentedLine();
    nbIterations=infile.getNextIntToken();
    nbIterationsCross=infile.getNextIntToken();
    bestSolNbIterationsCross=infile.getNextIntToken();

    printf("\tParametres charges\n");


    /// Starting time (chargé dans le main)
    infile.readUncommentedLine(); // Starting times

    /// Population
    int nbSommets=graph->nbSommets;
    for(int i=0; i<4; i++) {
        infile.readUncommentedLine();
		Solution sol(graph, nbColors);
		for (int i=0; i<nbSommets; i++)
			sol.tColor[i]=infile.getNextIntToken();
		sol.computeConflicts();
        vPopulation[i]=sol;
	}
	printf("\tPopulation chargee %d %d %d %d\n", vPopulation[0].tColor[0], vPopulation[1].tColor[0], vPopulation[2].tColor[0], vPopulation[3].tColor[0]);


    /// Bestsol
    infile.readUncommentedLine();
    bestSol=Solution(graph, nbColors);
    for (int i=0; i<nbSommets; i++)
        bestSol.tColor[i]=infile.getNextIntToken();
    bestSol.computeConflicts();
    printf("\tBestsol chargee\n");

	infile.close();

	if(debug)
        printf("Configuration chargee !!\n");
}

void Head::buildBaseName(){
    ostringstream oss;
	oss<< graph->filename << "_k" << nbColors;
	baseName=oss.str();
    unsigned int pos=baseName.find_last_of('/')+1;

    if(pos>baseName.length()) pos=0;
    baseName=baseName.substr(pos);
}
