#include<cstring>

#include "graphe.h"


Graph::~Graph(){
	for(int i=0; i<nbSommets; i++){
		delete[] tConnect[i];
	}
	delete[] tConnect;
	delete[] tVoisins;
}


void Graph::loadGraph(){
	GInputFile infile(filename);
	infile.open();

	char* buf;
	char* tok;
	int nbArretesAjoutee=0;

	while ( (buf=infile.readUncommentedLine()) ) {
		tok=infile.getNextToken();
                if (tok != NULL){
		if(tok && strcmp(tok, "nbVertices")==0){ // lecture d'une arrete
			loadMatrixGraph(infile);
			infile.close();
			buildVoisins();
			return;
		}


		if(*tok=='p'){ // lecture de la taille du graphe
			tok=infile.getNextToken(); //pas interessant
			tok=infile.getNextToken();
			nbSommets=atoi(tok);
			tok=infile.getNextToken();
			nbArretes=atoi(tok);

			// creation du graphe
			tConnect=new char*[nbSommets];
			for(int i=0; i<nbSommets; i++){
				tConnect[i]=new char[nbSommets];
				for (int j=0; j<nbSommets; j++)
					tConnect[i][j]=0;
			}

			//printf("Sommets ajoutes: %d\n", nbSommets);
		}

		if(*tok=='e'){ // lecture d'une arrete
			tok=infile.getNextToken();
			int v1=atoi(tok);
			tok=infile.getNextToken();
			int v2=atoi(tok);
			tConnect[v1-1][v2-1]=1;
			tConnect[v2-1][v1-1]=1;
			nbArretesAjoutee++;
		}}
	}

	//printf("Arretes ajoutees: %d / %d\n",nbArretesAjoutee ,nbArretes);

	infile.close();

	buildVoisins();
}





void Graph::loadMatrixGraph(GInputFile& infile){
	printf("Debut de chargement d'un graphe matrice haute\n");
	char* tok;
	tok=infile.getNextToken();
    nbSommets=atoi(tok);
    nbArretes=0;
    printf("Sommets ajoutes: %d\n", nbSommets);

    // creation du graphe
    // creation du graphe

    tConnect=new char*[nbSommets];
    for(int i=0; i<nbSommets; i++){
        tConnect[i]=new char[nbSommets];
    }

    for(int i=0; i<nbSommets; i++){
        infile.readUncommentedLine();
    	for(int j=0; j<nbSommets; j++){
			tok=infile.getNextToken(' ');
			tConnect[i][j]=atoi(tok);
			if(tConnect[i][j])
                nbArretes++;
		}
	}
	printf("Arretes ajoutees: %d\n",nbArretes);
}




void Graph::buildVoisins(){
	tVoisins=new vector<int>[nbSommets];
	for (int i=0; i<nbSommets; i++) {
		for (int j=0; j<nbSommets; j++) {
			if(tConnect[i][j])
				tVoisins[i].push_back(j);
		}
	}
}

