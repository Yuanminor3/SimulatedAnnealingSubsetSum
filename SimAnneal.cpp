// Written by Kia Bazargan, University of Minnesota
// SimAnneal.cpp: implementation of the CSimAnneal class.
//
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "SimAnneal.h"
#include <vector>

using namespace std;


//#define _VPR_
					// if defined, follows the parameters using the guidelines given in VPR


#define KIA_SHOW_ANNEAL_STATUS		// writes anneal temp, current cost, etc.


extern FILE *_kiaDebugFile;


const double BOLTZ_INIT_ACC_CONST = 32.83; // init acceptance=97%.refer to Kia's notebook, page 7-31-98
const double FULL_ANNEALING_INIT_TEMP = 40000.0; // independent of user input. There is a subtle difference between
double ANNEAL_INIT_TEMP = FULL_ANNEALING_INIT_TEMP; // might be read in main.cpp
// There is a subtle difference between FULL_ANNEALING_INIT_TEMP and
// ANNEAL_INIT_TEMP. The former is a constant used when a full annealing is running.
// The latter could be less than the former for a low-tempoerature annealing process and
// can be changed with user input.
double ANNEAL_COOLING_RATE = 0.95; // 0.95 might be changed in main.cpp
double ANNEAL_FREEZ_TEMP = 100; //0.01; // might be read in main.cpp
double NUM_MOVES_EACH_MODULE_EACH_TEMP_STEP = 1;// might be read in main.cpp

vector<int> elements;
vector<int> currentSolution;
vector<int> bestSolution;
int numberOfElements;
int targetSum;
double curCost;
vector<int> buckupSolution;
int finalCost;
vector<int> buckupBMyVector;
vector<int> buckupMyVector;
vector<int> myVector; // for no repeat index add
bool isRandom = false;


void buildRandomInitSolution() {
  // Generate a random initial solution
  myVector.resize(numberOfElements, 0);     
  int sum = 0;
  for (size_t i = 0; i < elements.size() && sum < targetSum; i++) {
      myVector[i] = 1;
      currentSolution.push_back(i);
      sum += elements[i];

      if (sum == targetSum) { // if random Initial solution is just solution
            isRandom = true;
            return;
      }
  }
}

double currentCost() {
  double sum = 0.0;
  for (int index : currentSolution) {
      sum += elements[index];
  }
  return abs(targetSum - sum);
}

void perturbSolution() {
  if (rand() % 2 == 0 && !currentSolution.empty()) {
      // Remove a random element
      int removeIndex = rand() % currentSolution.size();
      int indexToRemove = currentSolution[removeIndex];
      myVector[indexToRemove] = 0; // Mark as not added
      currentSolution.erase(currentSolution.begin() + removeIndex);
  } else {
    // Add a random element if possible
    vector<int> availableIndices;
    for (int i = 0; i < myVector.size(); ++i) {
        if (myVector[i] == 0) {
            availableIndices.push_back(i);
        }
    }
    if (!availableIndices.empty()) {
        int randomIndex = rand() % availableIndices.size();
        int addIndex = availableIndices[randomIndex];
        myVector[addIndex] = 1; // Mark as added
        currentSolution.push_back(addIndex);
    }
    }
}

void backupCurrentSolution() {
  buckupSolution = currentSolution;
  buckupMyVector = myVector;
}

void restoreBackedUpSolution() {
  currentSolution = buckupSolution;
  myVector = buckupMyVector;
}

void saveCurrentSolutionAsBestSolution() {
  bestSolution = currentSolution;
  buckupBMyVector = myVector;
}

void restoreBestSolution() {
  currentSolution = bestSolution;
  myVector = buckupBMyVector;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSimAnneal::CSimAnneal(int inputSize)
{

  numMovesEachTemp_ = (long)(.5+ inputSize* NUM_MOVES_EACH_MODULE_EACH_TEMP_STEP);
  if (numMovesEachTemp_<1)
	  numMovesEachTemp_ = 1;

  temperatureStep_=1;
  T_ = ANNEAL_INIT_TEMP;
}

CSimAnneal::~CSimAnneal()
{

}


double CSimAnneal::doAMove()
{
  double oldCost = currentCost();

  perturbSolution();

  double newCost = currentCost();
  return newCost - oldCost;
}

void CSimAnneal::doAnnealing()
{
  initializeAnnealing();
  if (isRandom){
      return;
  }

  bool annealingNotDoneYet;
  do {
    annealingNotDoneYet = goOneTempStep();
  } while (annealingNotDoneYet == true);

  cout << "Final annealing cost =" << currentCost() << endl;
  finalCost = currentCost();
}



void CSimAnneal::writeCostsInFile(FILE *outFile)
{
  fprintf(outFile, "Cost after restoring the best= %10.3f\n",
	  currentCost()  );
}


void CSimAnneal::initializeAnnealing()
{
   buildRandomInitSolution();
   if(isRandom){
      return;
   }

   backupCurrentSolution();

	temperatureStep_=1; // a serial counter for different temperatures

	if (numMovesEachTemp_==0)
		numMovesEachTemp_ = 1;

	cout << "Initial cost = " << currentCost() << endl;

	globallyBestCost_ = currentCost();

	// First perturb the tree and see how the delta_cost function behaves.
	// Then set the boltzmannConst_ so that 99% of the moves are
	// accepted when T_==ANNEAL_T0.
	// See Kia's notebook page 7-31-98
	int i;
	soFarAvgDeltaCost_ = 1e-20;
	for (i=0 ; i<20 ; i++) {
		double deltaCost = doAMove();
		soFarAvgDeltaCost_ += fabs(deltaCost);
	}
	soFarAvgDeltaCost_ /= 20;

//#pragma WARNING("artificially put in here. remove after you optimize")
//	soFarAvgDeltaCost_ /= 100.0;

	shortTermAvgDeltaCost_.avgWithThis(soFarAvgDeltaCost_);

	if (currentCost() > globallyBestCost_) {
	   restoreBackedUpSolution();
	}
	else {
		globallyBestCost_ = currentCost();
		backupCurrentSolution();
	}


	cout << "avg DC init = " << soFarAvgDeltaCost_ << endl;

#ifdef _VPR_
	T_ = 20 * soFarAvgDeltaCost_;
#else
	T_ = ANNEAL_INIT_TEMP;
#endif


	expectedNumTempSteps_ = (int)(.5 + log ( ANNEAL_FREEZ_TEMP / FULL_ANNEALING_INIT_TEMP) / log ( ANNEAL_COOLING_RATE ));

	assert ( expectedNumTempSteps_ > 0 );

#ifdef _VPR_
	boltzmannConst_ = 1.0;
#else
	boltzmannConst_ = BOLTZ_INIT_ACC_CONST * soFarAvgDeltaCost_ / ANNEAL_INIT_TEMP;
	cout << "boltzCons=" << boltzmannConst_ << endl;
#endif // _VPR_

	assert ( boltzmannConst_ > 0 );

	scaledBoltzTimesT_ = boltzmannConst_ * T_;


#ifdef KIA_SHOW_ANNEAL_STATUS
	fprintf(_kiaDebugFile, "tmpStp,T,cost,dcHis,ac,rj,avgDCg,avgDCb,stpDC\n");
	fflush(_kiaDebugFile);
	printf("%3s %8s %15s\n", "stp", "temp  ", "cost    ");
	fflush(stdout);
#endif

}



// debugging variables. Only thread 0 will access them.
double deltaCostGoodMoves=0;
double deltaCostBadMoves=0;
long numGoodMoves=0;
long numBadMoves=0;
double prevStepCost=0;

bool CSimAnneal::acceptMove ( double deltaCost )
{
  if (deltaCost < -1e-6) {
    ++numGoodMoves;
    deltaCostGoodMoves += deltaCost;
    return true;
  }

  ++numBadMoves;
  deltaCostBadMoves += deltaCost;

  double expDCOverT = exp(-deltaCost / scaledBoltzTimesT_);
  double r = ((double)rand()) / RAND_MAX;
  if (r < expDCOverT)
    return true;
  return false;
}

// Decreases T_ with a rate depending on T_'s value.
void CSimAnneal::coolDownTemperature()
{

	++temperatureStep_;

#ifdef _VPR_
	double Racc = 1.0 * numGoodMoves / (numGoodMoves+numBadMoves);
	if (Racc > 0.96)
		T_ *= 0.5;
	else if (Racc > 0.8)
		T_ *= 0.9;
	else if (Racc > 0.15)
		T_ *= 0.95;
	else T_ *= 0.8;
	T_ *= ANNEAL_COOLING_RATE / 0.95;   // so that if the user wants faster/slower annealing, s/he can get it
	// this assumes that 0.95 is the "natural" cooling rate
#else // _VPR_
	T_ *= ANNEAL_COOLING_RATE;
#endif // _VPR_


  scaledBoltzTimesT_ = boltzmannConst_ * T_;

  deltaCostGoodMoves=0;
  deltaCostBadMoves=0;
  numGoodMoves=0;
  numBadMoves=0;
  prevStepCost = currentCost();
}


// One step of the annealing core.
// If the return value is true, the annealing is ongoing, otherwise, it is done.
bool CSimAnneal::goOneTempStep()
{
  if (T_ < ANNEAL_FREEZ_TEMP) {// annealing finished
    restoreBestSolution(); // restore the best seen

#ifdef KIA_SHOW_ANNEAL_STATUS
    fprintf(_kiaDebugFile, "\nCost after restoring the best= %10.3f \n",
	    currentCost());
#endif
    return false;
  }

  long moveCount = 0;
  int tempNumAccMoves=0; // number of accepted moves at this temperature
  int tempNumRejMoves = 0;
  double avgTempStepDeltaCost = 0;
  double costBeforeTempStep = currentCost();

  int numMoves = numMovesEachTemp_;

/*
#ifndef _VPR_
  if (temperatureStep_<0.25*expectedNumTempSteps_)
    numMoves /= 2;
  else if (temperatureStep_>0.5*expectedNumTempSteps_)
    numMoves /= 4;
  if (numMoves==0)
    ++numMoves;
#endif // _VPR_
*/

  for (moveCount=0 ; moveCount<numMoves ; moveCount++) {
    backupCurrentSolution();
    double deltaCost = doAMove();

    avgTempStepDeltaCost += fabs (deltaCost);

    double curCost = currentCost();
    if (acceptMove(deltaCost)) {
      if (deltaCost<0 && curCost < globallyBestCost_) {
        globallyBestCost_ = curCost;
        saveCurrentSolutionAsBestSolution(); // as the best seen so far
      }

      ++tempNumAccMoves;
    } else {
      restoreBackedUpSolution();
      ++tempNumRejMoves;
    }
  } // for(moveCount=0... move per temperature

  avgTempStepDeltaCost /= numMoves;

  // See if the cost is not improving over the last temp steps...
  double curCost = currentCost();

#ifdef KIA_SHOW_ANNEAL_STATUS
  double avgDCg = 0.0;
  double avgDCb = 0.0;
  char areaOverflow=0;

  if (numGoodMoves)
    avgDCg = deltaCostGoodMoves / numGoodMoves;
  if (numBadMoves)
    avgDCb = deltaCostBadMoves / numBadMoves;
  fprintf(_kiaDebugFile, "%d,%g,%g,%g,%d,%d,%g,%g,%g,",
	  temperatureStep_, T_, currentCost(), soFarAvgDeltaCost_, tempNumAccMoves, tempNumRejMoves,
	  avgDCg, avgDCb, currentCost()-prevStepCost );
  fprintf(_kiaDebugFile, "\n");

  printf("%3d %8.2f %15.4f\n",
	  temperatureStep_, T_, currentCost() );
  fflush(_kiaDebugFile);
  fflush(stdout);
#endif


#ifndef _VPR_
  // Update the avg delta cost and/or boltzmann constant
  if (temperatureStep_<40) {
    soFarAvgDeltaCost_  = shortTermAvgDeltaCost_.avgWithThis ( avgTempStepDeltaCost );
    assert ( soFarAvgDeltaCost_ > 0 );
    boltzmannConst_ = BOLTZ_INIT_ACC_CONST * soFarAvgDeltaCost_ / FULL_ANNEALING_INIT_TEMP;
  }
#endif // _VPR_


  coolDownTemperature();

  return true;
}




//////////////////////////////////////////////////////////////////////
// ShortTermAverage Class
double ShortTermAverage::avgWithThis(double newValue)
{
  if (numSamples_ < HIST_LEN) {
    ++numSamples_;
    history_[curIdx_++] = newValue;
    sum_ += newValue;
    if (curIdx_ == HIST_LEN)
      curIdx_ = 0;
    return sum_ / numSamples_;
  }

  sum_ -= history_[curIdx_];
  history_[curIdx_++] = newValue;
  sum_ += newValue;

  if (curIdx_ == HIST_LEN)
    curIdx_ = 0;

  return sum_ / HIST_LEN;
}

