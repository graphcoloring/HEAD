#include<iostream>
#include <stdlib.h>
#include <stdio.h>
#include<math.h>
#include "tabouSearch.h"

using namespace std;

TabouSearch::TabouSearch(){
  graph=NULL;
  tNewConflitsWithColor=tTabou=ttBestImproveColor=NULL;
  tConflicts=tBestImprove=tNbBestImprove=tNodeWithConflict=tNodeAdded=NULL;
  tNbChanges=NULL;
  tNbConflicts=NULL;
  tTotalBestImprove=NULL;
  nbColors=nbNodeWithConflict=-1;
}

TabouSearch::~TabouSearch(){
  if (graph) {
    int nbSommets=graph->nbSommets;
    delete[] tConflicts;
    delete[] tNbChanges;
    delete[] tNbConflicts;
    delete[] tTotalBestImprove;

    for(int i=0; i<nbSommets; i++){
      delete[] tNewConflitsWithColor[i];
    }
    delete[] tNewConflitsWithColor;

    for(int i=0; i<nbSommets; i++){
      delete[] tTabou[i];
    }
    delete[] tTabou;

    // optimisation1
    delete[] tBestImprove;
    delete[] tNbBestImprove;
    for (int i=0; i<nbSommets; i++) {
      delete[] ttBestImproveColor[i];
    }
    delete[] ttBestImproveColor;

    //optimisation2
    delete[] tNodeWithConflict;
    delete[] tNodeAdded;
  }
}


TabouSearch& TabouSearch::operator=(const TabouSearch& ts){
  tNewConflitsWithColor=tTabou=ttBestImproveColor=NULL;
  tConflicts=tBestImprove=tNbBestImprove=tNodeWithConflict=tNodeAdded=NULL;

  if(ts.graph)
    buildTables(ts.graph, ts.nbColors);
  return *this;
}

void TabouSearch::buildTables(Graph* gr, int nbColors){
  int nbSommets=gr->nbSommets;
  graph = gr;
  this->nbColors = nbColors;

  // recherche Tabou
  tConflicts = new int[nbSommets];

  tNbChanges = new double[nbSommets];
  tNbConflicts = new double[nbSommets];
  tTotalBestImprove = new double[nbSommets];

  tNewConflitsWithColor = new int*[nbSommets];
  for(int i=0; i<nbSommets; i++){
    tNewConflitsWithColor[i] = new int[nbColors];
  }

  tTabou = new int*[nbSommets];
  for(int i=0; i<nbSommets; i++){
    tTabou[i] = new int[nbColors];
  }

  currentSol=Solution(gr, nbColors);
  currentSol.computeConflicts(tConflicts);

  // optimisation1
  tBestImprove = new int[nbSommets];  // valeur
  tNbBestImprove = new int[nbSommets];
  ttBestImproveColor = new int*[nbSommets]; // couleur
  for (int i=0; i<nbSommets; i++) {
    ttBestImproveColor[i]=new int[nbColors];
  }

  // optimisation2
  tNodeWithConflict = new int[nbSommets];
  tNodeAdded = new int[nbSommets];

  initNbChanges();
}


void TabouSearch::initNbChanges(){
  int nbSommets=graph->nbSommets;
  /// determine les delta-conflits pour chaque transition de couleur
  for (int i=0; i<nbSommets; i++) {
    tNbChanges[i]=0.0;
    tNbConflicts[i]=0;
    tTotalBestImprove[i]=0;
  }
}

void TabouSearch::printNbChanges(){
  int nbSommets=graph->nbSommets;
  printf("\n");
  for (int i=0; i<nbSommets; i++) {
    printf("%4d ", (int)tNbChanges[i]);
  }
}


bool TabouSearch::compute(Solution& sol, int nbLocalSearch, string baseName, int nbIter, int *tBlockedNode){
  FILE *f=NULL;
  this->nbLocalSearch = nbLocalSearch;

  analyseBaseName=baseName;
  if (baseName!="") {
    f = fopen((baseName+"TabouFitness.xls").c_str(), "a");
    fprintf(f, "\nIter%d ", nbIter);
  }

  bool found=false;
  currentSol=sol;
  bestSol.nbEdgesConflict=bestSol.nbNodesConflict=graph->nbArretes+1;

  currentSol.computeConflicts(tConflicts);
  initTables();
  nbIterations=0;

  initNbChanges();

  //// Si on doit bloquer des noeuds, on bloque la couleur spécifiée pour le noeud
  if(tBlockedNode!=NULL)
    for(int i=0; i<graph->nbSommets; i++)
      if(tBlockedNode[i]>=0)
        tTabou[i][tBlockedNode[i]]=nbLocalSearch;
	////
  for (int i=0; i<nbLocalSearch && currentSol.nbEdgesConflict > 0 ; i++) {
    determineBestImprove();
    if (currentSol.nbEdgesConflict<=bestSol.nbEdgesConflict) {  //// si <= : derniere meilleure rencontrée ; si < : premiere meilleure
      if (currentSol.nbEdgesConflict<bestSol.nbEdgesConflict)
        currentSol.nbIterationsFirst=nbIterations;
      bestSol = currentSol;
    }

    if (f) { /// pour analyse
      fprintf(f, "%d ", currentSol.nbEdgesConflict);
      updateAnalyseData();
    }
  }

  if (baseName!="") {
    fclose(f);
    saveAnalyse();
  }

  if (bestSol.nbEdgesConflict==0)
    found=true;
  sol=bestSol;
  return found;
}

void TabouSearch::initTables(){
  initNbChanges();

  int nbSommets=graph->nbSommets;
  /// determine les delta-conflits pour chaque transition de couleur
  for (int i=0; i<nbSommets; i++) {
    int nbCurrentConflict=tConflicts[i];
    for (int j=0; j<nbColors; j++) {
      tNewConflitsWithColor[i][j]=-nbCurrentConflict;
    }

    int nbVoisins=graph->tVoisins[i].size();
    for (int j=0; j<nbVoisins; j++) {
      tNewConflitsWithColor[i][currentSol.tColor[graph->tVoisins[i][j]]]++;
    }
  }

  /// initialise les durées tabou
  for (int i=0; i<nbSommets; i++) {
    for (int j=0; j<nbColors; j++) {
      tTabou[i][j]=-1;
    }
  }

  /// optimisation1: Determine les meilleures transition de chaque sommet
  for (int i=0; i<nbSommets; i++) {
    int bestVal=graph->nbArretes+1;
    int nbBestVal=0;
    for (int j=0; j<nbColors; j++) {
      int val=tNewConflitsWithColor[i][j];
      if (val<bestVal){
        nbBestVal=0;
        bestVal = val;
      }

      if(val<=bestVal){
        ttBestImproveColor[i][nbBestVal++]=j;
      }

    }
    tBestImprove[i] = bestVal;
    tNbBestImprove[i] = nbBestVal;
  }


  /// optimisation2: Determination des noeuds avec conflict (par défaut tous les noeuds)
  resetNodeWithConflict();
}

void TabouSearch::resetNodeWithConflict(){
  nbNodeWithConflict=0;
  for (int i=0; i<graph->nbSommets; i++)
    tNodeAdded[i]=0;

  for (int i=0; i<graph->nbSommets; i++){
    if(tConflicts[i]>0){
      tNodeWithConflict[nbNodeWithConflict++]=i;
      tNodeAdded[i]=1;
    }
  }
}

void TabouSearch::determineBestImprove(){
  /// choisit aléatoirement parmis les meilleurs noeuds
  nbIterations++;
  currentSol.nbIterations=nbIterations;

  int bestVal=graph->nbArretes+1;
  int bestNode=-1;
  int bestColor=-1;
  int nbBestVal=0;

  //    int debut=rand()/(double)RAND_MAX * nbNodeWithConflict; // on démarre de n'importe quel sommet et on prend le meilleur strict.
  //    for(int ind=0; ind<nbNodeWithConflict; ind++){
  //        int i=tNodeWithConflict[(debut+ind)%nbNodeWithConflict];
  for(int ind=0; ind<nbNodeWithConflict; ind++){
    int i=tNodeWithConflict[ind];

    if(tConflicts[i]>0 && tBestImprove[i]<=bestVal){
      int color = currentSol.tColor[i];
      int currentBestImprove = tBestImprove[i];
      int currentNbBestImprove = tNbBestImprove[i];
      int added=0; /// permet de savoir si on a reussi à ajouter une valeure (1=true)

      for (int j=0; j<currentNbBestImprove; j++) {
        int currentCol=ttBestImproveColor[i][j];

        if ((currentCol!=color) && ((tTabou[i][currentCol]<nbIterations) || (((currentBestImprove+currentSol.nbEdgesConflict) < bestSol.nbEdgesConflict) && (tTabou[i][currentCol] != nbLocalSearch))) ) {
          added=1;
          if (currentBestImprove < bestVal) {
            bestVal=currentBestImprove;
            bestNode=i;
            bestColor=currentCol;
            nbBestVal=1;
          }
          else{
            nbBestVal++;
            int val=(rand_r(&randSeed)/(double)RAND_MAX) * nbBestVal ;
            if(val==0){
              bestNode=i;
              bestColor=currentCol;
            }
          }
        }
      }

      if (currentBestImprove < bestVal && added == 0) { /// on doit tout vérifier
        for (int j=0; j<nbColors; j++) {
          int currentImprove=tNewConflitsWithColor[i][j];
          if((currentImprove<bestVal) && (j!=color) && ((tTabou[i][j]<nbIterations) || (((currentImprove+currentSol.nbEdgesConflict) < bestSol.nbEdgesConflict) && (tTabou[i][j] != nbLocalSearch))) ){
            bestVal=currentImprove;
            bestNode=i;
            bestColor=j;
            nbBestVal=1;
          }
          else if ((currentImprove==bestVal) && (j!=color) && ((tTabou[i][j]<nbIterations) || (((currentImprove+currentSol.nbEdgesConflict) < bestSol.nbEdgesConflict) && (tTabou[i][j] != nbLocalSearch))) ){ // on tire aleatoirement 1 des 2
            nbBestVal++;
            int val=(rand_r(&randSeed)/(double)RAND_MAX) * nbBestVal ;
            if(val==0){
              bestVal=currentImprove;
              bestNode=i;
              bestColor=j;
            }
          }
        }
      }
    }
  }
  if (bestNode>-1) {
    updateTables(bestNode, bestColor);
  }
}

void TabouSearch::updateTables(int node, int color){
  tNbChanges[node]++;
  int prevColor=currentSol.tColor[node];
  currentSol.tColor[node]=color;

  int rd=(rand_r(&randSeed)/(double)RAND_MAX) * L;

  tTabou[node][prevColor]=(int) (nbIterations + rd + lambda*currentSol.nbNodesConflict);

  int nbVoisins=graph->tVoisins[node].size();
  for (int i=0; i<nbVoisins; i++) {
    int indiceSommet=graph->tVoisins[node][i];

    /// répercution sur les voisins
    if (currentSol.tColor[indiceSommet] == prevColor) {
      tConflicts[indiceSommet]--;
      tConflicts[node]--;
      currentSol.nbEdgesConflict--;
      if (tConflicts[indiceSommet]==0) currentSol.nbNodesConflict--;
      if (tConflicts[node]==0) currentSol.nbNodesConflict--;

      for(int j=0; j<nbColors; j++){
        tNewConflitsWithColor[indiceSommet][j]++;
        tNewConflitsWithColor[node][j]++;
      }
      tBestImprove[indiceSommet]++;
      tBestImprove[node]++;
    }
    else if (currentSol.tColor[indiceSommet] == color) {
      tConflicts[indiceSommet]++;
      tConflicts[node]++;
      currentSol.nbEdgesConflict++;
      if (tConflicts[indiceSommet]==1){
        currentSol.nbNodesConflict++;
        if (tNodeAdded[indiceSommet]!=1) {
          tNodeAdded[indiceSommet]=1;
          tNodeWithConflict[nbNodeWithConflict++]=indiceSommet;
        }
      }
      if (tConflicts[node]==1){
        currentSol.nbNodesConflict++;
        if (tNodeAdded[node]!=1) {
          tNodeAdded[node]=1;
          tNodeWithConflict[nbNodeWithConflict++]=node;
        }
      }
      for(int j=0; j<nbColors; j++){
        tNewConflitsWithColor[indiceSommet][j]--;
        tNewConflitsWithColor[node][j]--;
      }
      tBestImprove[indiceSommet]--;
      tBestImprove[node]--;
    }

    tNewConflitsWithColor[indiceSommet][prevColor]--;
    tNewConflitsWithColor[indiceSommet][color]++;

    /////////////////////
    //// ajout pour garder la meilleur transition
    int bestImprove=tBestImprove[indiceSommet];

    if (tNewConflitsWithColor[indiceSommet][prevColor] < bestImprove) {
      tBestImprove[indiceSommet]--;
      ttBestImproveColor[indiceSommet][0] = prevColor;
      tNbBestImprove[indiceSommet] = 1;
    }
    else if(tNewConflitsWithColor[indiceSommet][prevColor] == bestImprove){
      ttBestImproveColor[indiceSommet][tNbBestImprove[indiceSommet]] = prevColor;
      tNbBestImprove[indiceSommet]++;
    }

    if ( (tNewConflitsWithColor[indiceSommet][color]-1) == bestImprove) {// si c'était le meilleur
      ///
      int nbBestImprove=tNbBestImprove[indiceSommet];
      if (nbBestImprove>1) {
        tNbBestImprove[indiceSommet]--;
        int pos, found=0;
        for (pos=0; found!=1; pos++) {
          if (ttBestImproveColor[indiceSommet][pos] == color)
            found=1;
        }

        for (pos=pos; pos<nbBestImprove; pos++) {
          ttBestImproveColor[indiceSommet][pos-1] = ttBestImproveColor[indiceSommet][pos];
        }
      }
      else {
        ///
        int nbBestVal=0;
        int bestVal=graph->nbArretes+1;
        for (int j=0; j<nbColors; j++) {
          int val=tNewConflitsWithColor[indiceSommet][j];
          if (val<bestVal) {
            bestVal=val;
            ttBestImproveColor[indiceSommet][0] = j;
            nbBestVal=1;
          }
          else if(val==bestVal){
            ttBestImproveColor[indiceSommet][nbBestVal] = j;
            nbBestVal++;
          }
        }
        tBestImprove[indiceSommet] = bestVal;
        tNbBestImprove[indiceSommet] = nbBestVal;
      }
    }
  }


  if (nbIterations % 100 == 0) { // arbitrairement tous les 100 on recalcule les noeuds avec conflit
    resetNodeWithConflict();
  }
}

void TabouSearch::updateAnalyseData(){
  int nbSommets=graph->nbSommets;
  for (int i=0; i<nbSommets; i++) {
    tNbConflicts[i]+=tConflicts[i];
    tTotalBestImprove[i]+=tBestImprove[i];
  }
}

void TabouSearch::saveAnalyse(){
  FILE *f=NULL;
  int nbSommets=graph->nbSommets;

  //// Nb Changes
  f = fopen((analyseBaseName+"NbChanges.xls").c_str(), "a");
  for (int i=0; i<nbSommets; i++) {
    fprintf(f, "%d ", (int)tNbChanges[i]);
  }
  fprintf(f, "\n");
  fclose(f);

  //// Nb Conflicts
  f = fopen((analyseBaseName+"NbConflicts.xls").c_str(), "a");
  for (int i=0; i<nbSommets; i++) {
    fprintf(f, "%d ", (int)tNbConflicts[i]);
  }
  fprintf(f, "\n");
  fclose(f);

  //// Best improvement per node
  f = fopen((analyseBaseName+"BestImprove.xls").c_str(), "a");
  for (int i=0; i<nbSommets; i++) {
    fprintf(f, "%d ", (int)tTotalBestImprove[i]);
  }
  fprintf(f, "\n");
  fclose(f);
}
