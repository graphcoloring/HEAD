#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <iomanip>
#include <getopt.h>


#include "graphe.h"
#include "head.h"
#include "util/gfile.h"



#define TRYFORHELP  "Try `%s -h' for more information.\n",argv[0]

void printhelp(char *prog);
void read_options(int argc, char **argv, char* graphName, int *nbColor,
                  long long *nbLocalSearch, int *nbGeneration, int *nbRun,
                  int *randSeed, int *nbRandCross, int *swapIter,
                  double *swapingRate, bool *quite, bool *tabucol,
                  double *tauxAcceptWorst, double *weightParent,
                  int *maxseconds,
                  char* outputFile);


int main(int argc, char **argv) {

    /* Options, changed by command-line arguments. */
    char graphName[80] = "";
    int nbColor;
    long long nbLocalSearch = 30000;
    int nbGeneration = -1;
    int nbRun = 1;
    int randSeed = 0;
    int nbRandCross = 0;
    int swapIter = -1;
    double swapingRate = 99.0;
    bool quite = false;
    bool tabucol = false;
    double tauxAcceptWorst = 1.0;
    double weightParent = -1;
    char outputFile[255] = "";
    int maxseconds = -1;

    if (argc<=2)
        printhelp(argv[0]);

    read_options(argc,argv, graphName, &nbColor, &nbLocalSearch,
               &nbGeneration, &nbRun, &randSeed, &nbRandCross, &swapIter,
               &swapingRate, &quite, &tabucol, &tauxAcceptWorst,
               &weightParent, &maxseconds,
               outputFile);

  Graph g(graphName);
  double totalCpuTime=0;
  double totalHumanTime=0;
  unsigned long long totalIterations=0;
  unsigned long long totalIterationsCross=0;
  unsigned long long totalIterationsCrossWithWrongRun=0;
  int nbFound=0;
  Head* solver=new Head();

  // >> Solver Parameters
  solver->graph=&g;
  solver->nbColors=nbColor;
  solver->nbLocalSearch=nbLocalSearch;
  solver->nbGeneration=nbGeneration;
  solver->userRandSeed=randSeed;
  solver->nbRandCross=nbRandCross;
  solver->swapIter=swapIter;
  solver->swapingRate=swapingRate/100.0;
  solver->debug=!quite;
  solver->tauxAcceptWorst=tauxAcceptWorst;
  solver->weightParent=weightParent/100.0;
  solver->initSol = NULL;
  solver->maxsecondes=maxseconds;
  solver->tabucol = tabucol;
  // << solver parameters

  //// >>>>>>>>>  affichage de l'heure au debut
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  if(!quite)
      printf ( "Start at :  %s", asctime (timeinfo) );
  //// <<<<<<<<<  affichage de l'heure de debut

  int runId;
  for (runId=0; (runId<nbRun) || ((nbRun==-1) && (nbFound==0)) ; runId++) {
    clock_t startTime=clock();
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double humanTime = (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0;

    //// LANCEMENT DU CALCUL /////////////////
    solver->compute();

    double elapsedTime=(clock()-startTime) / (double)CLOCKS_PER_SEC/60.0;
    gettimeofday(&tv, NULL);
    humanTime = ((double) tv.tv_sec + (double) tv.tv_usec / 1000000.0 - humanTime)/60.0;
    
    printf("nb conflicted edges: %d\n", solver->bestSol.nbEdgesConflict);
    if(!quite){
      printf("best coloring: ");
      solver->bestSol.breakSymmetry();
      solver->bestSol.print();
    }
    
    // Save coloring(s) in outputfile
    if (strlen(outputFile) != 0)
      solver->saveBestColoring(outputFile);

    /////// Affichage de l'heure
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    if(!quite)
        printf ( "\tFinished :  %s  (cpu time: %fmin, humain time : %fmin)\n", asctime (timeinfo), elapsedTime, humanTime );
    fflush(stdout);
    ///////

    totalIterationsCrossWithWrongRun+=solver->nbIterationsCross;
    if (solver->bestSol.nbEdgesConflict==0) {
      //solver->save(elapsedTime);
      nbFound++;
      totalCpuTime+=elapsedTime;
      totalHumanTime+=humanTime;
      totalIterations+=solver->nbIterations;
      totalIterationsCross+=solver->nbIterationsCross;
    }

    fflush(stdout);
    
  }
  printf("\n#Success / #Runs : %d / %d\n", nbFound,runId);

  if (nbFound>0) {
    printf("Mean CPU time   : %0.2f min\n", totalCpuTime/nbFound);
    printf("Mean humain time: %0.2f min\n", totalHumanTime/nbFound);
    printf("Mean iterations : %0.3f (x10.6)\n", totalIterations/nbFound/1000000.0);
    printf("Mean crossover  : %llu \n", totalIterationsCross/nbFound);
  }

  printf("End\n");
  fflush(stdout);
  return 0;
}



/*
 * Prints the help message to stdout.  prog is the program name
 * (from argv[0]).
 */
void printhelp(char *prog) {
  printf("Usage:  %s [options] <dimacs-file> <number-color>\n"
         "\n"
         " <dimacs-file>   is the ASCII DIMACS-format file from which to read the graph.\n"
         " <number-colors> is the number of colors used.\n Options are the following (I:Integer; F:Float; S:String):\n"
         "\n"
         " -h    --help           This help text.\n"
         " -t    --tabucol        Run TabouCol (with 10 000 000 iterations by default)\n"
	 "                         instead of HEAD (default: HEAD).\n"
         " -i I  --iter I         Number of iterations for the local search\n"
	 "                         (default =30000).\n"
         " -g I  --gen I          Number of generations of memetic algorithm\n"
	 "                         (-1: until finding a legal coloring; default =-1).\n"
         " -r I  --run I          Number of runs (-1: run until finding a\n"
	 "                         legal coloring; default =1)\n"
         " -d I  --seed I         Seed to run again a specific run\n"
	 "                         (0: random seed, default)\n"
         " -R I  --RandCross I    Number of random color classes transmited before GPX\n"
	 "                         crossover (default=0: standard GPX crossover).\n"
         " -w I  --swap I         Number of runs before swapping \n"
	 "                         (-1 : no swapping; default =-1).\n"
         " -p F  --swapingRate F  Proximity that generates a swap (defaut =99).\n"
         " -a F  --acceptRate F   Acceptance rate [0.0, 1.0] of a chirld worst than \n"
	 "                         parents (default=1.0).\n"
         " -U F  --weightParent F Imbalanced crossover. Weight [0, 100] of first parent\n"
	 "                         in crossover (defaut =-1 : no weight = standard GPX).\n"
         " -s I  --maxseconds I   Maximal duration of one run (defalut =-1 : no limit).\n"
	 "                         Another stopping criterion is when the 4 colorings\n"
	 "                         (2 solutions + 2 elites) are too similar.\n"
         " -o S  --output S       Save solution(s) found in a file.\n"
         " -q    --quiet          Suppresses progress output.\n"
         "\n",
         prog);
  exit(1);
}


/*
 * Reads the command-line options and sets the global options accordingly.
 */
void read_options(int argc, char **argv, char* graphName, int *nbColor, \
                  long long *nbLocalSearch, int *nbGeneration, int *nbRun,
                  int *randSeed, int *nbRandCross, int *swapIter,
                  double *swapingRate, bool *quite, bool *tabucol,
                  double *tauxAcceptWorst, double *weightParent,
                  int *maxseconds,
                  char* outputFile){
  int c;
  char *ptr;

  while (true) {
    int option_index = 0;
    static struct option long_options[] = {
      { "tabucol", no_argument, NULL, 't' },
      { "nbIterLocal", required_argument, NULL, 'i' },
      { "nbGen", required_argument, NULL, 'g' },
      { "nbRun", required_argument, NULL, 'r' },
      { "seed", required_argument, NULL, 'd' },
      { "RandCross", required_argument, NULL, 'R' },
      { "swap", required_argument, NULL, 'w' },
      { "swapingRate", required_argument, NULL, 'p' },
      { "acceptRate", required_argument, NULL, 'a' },
      { "weightParent", required_argument, NULL, 'U' },
      { "maxseconds", required_argument, NULL, 's' },
      { "output", required_argument, NULL, 'o' },
      { "quiet", no_argument, NULL, 'q' },
      { "help", no_argument, NULL, 'h' },
      { 0,0,0,0 }
    };
    c=getopt_long(argc,argv,"i:g:r:d:R:w:p:a:U:s:f:o:qth",
                  long_options,&option_index);
    if (c==-1)
      break;
    switch (c) {
    case 0:
      break;
    case 't':
      *tabucol=true;
      if (*nbLocalSearch == 30000)
	*nbLocalSearch=10000000;
      break;
    case 'q':
      *quite=true;
      break;
    case 'h':
      printhelp(argv[0]);
      break;
    case 'i' :
      *nbLocalSearch=strtoll(optarg,&ptr,10);
      if ((*optarg == 0) || (*ptr != 0) || (*nbLocalSearch<=0)) {
        fprintf(stderr,"Bad argument: %s\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 'g' :
      *nbGeneration=strtol(optarg,&ptr,10);
      if ((*optarg == 0) || (*ptr != 0) || (*nbGeneration<-1)) {
        fprintf(stderr,"Bad argument: %s\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 'r' :
      *nbRun=strtol(optarg,&ptr,10);
      if ((*optarg == 0) || (*ptr != 0) || (*nbRun<-1)) {
        fprintf(stderr,"Bad argument: %s\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 'd' :
      *randSeed=strtol(optarg,&ptr,10);
      if ((*optarg == 0) || (*ptr != 0) || (*randSeed<0)) {
        fprintf(stderr,"Bad argument: %s\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 'R' :
      *nbRandCross=strtol(optarg,&ptr,10);
      if ((*optarg == 0) || (*ptr != 0) || (*nbRandCross<0)) {
        fprintf(stderr,"Bad argument: %s\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 'w' :
      *swapIter=strtol(optarg,&ptr,10);
      if ((*optarg == 0) || (*ptr != 0) || (*swapIter<-1)) {
        fprintf(stderr,"Bad argument: %s\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 's' :
      *maxseconds=strtol(optarg,&ptr,10);
      if ((*optarg == 0) || (*ptr != 0) || (*maxseconds<-1)) {
        fprintf(stderr,"Bad argument: %s\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 'p' :
      *swapingRate=strtod(optarg,&ptr);
      if ((*optarg == 0) || (*ptr != 0) || (*swapingRate<=0) || (*swapingRate>100)) {
        fprintf(stderr,"Bad argument: %s in ]0, 100]\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 'a' :
      *tauxAcceptWorst=strtod(optarg,&ptr);
      if ((*optarg == 0) || (*ptr != 0) || (*tauxAcceptWorst<0) || (*tauxAcceptWorst>1)) {
        fprintf(stderr,"Bad argument: %s in [0.0, 1.0]\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 'u' :
      *weightParent=strtod(optarg,&ptr);
      if ((*optarg == 0) || (*ptr != 0) || (*weightParent<-1) || (*weightParent>100)) {
        fprintf(stderr,"Bad argument: %s in [0, 100] or =-1\n",optarg);
        fprintf(stderr,TRYFORHELP);
        exit(1);
      }
      break;
    case 'o' :
      strcpy(outputFile,optarg);
      //strcat(outputFile,optarg);
      break;
    case '?':
      fprintf(stderr,TRYFORHELP);
      exit(1);
    default:
      fprintf(stderr,"Internal bug: line %d should never be reached!",__LINE__);
      exit(-1);
    }
  }

  if (optind+1 >= argc) {
    fprintf(stderr,"Missing filename or number of color used.\n");
    fprintf(stderr,TRYFORHELP);
    exit(1);
  }
  if (optind < argc-2) {
    fprintf(stderr,"Bad command line option: %s\n",argv[optind]);
    fprintf(stderr,TRYFORHELP);
    exit(1);
  }
  strcpy(graphName,argv[optind]);
  *nbColor=atoi(argv[optind+1]);
  if (*nbColor<=0) {
    fprintf(stderr,"Bad number of colors: %s\n",argv[optind-1]);
    fprintf(stderr,TRYFORHELP);
    exit(1);
  }
  return;
}

