// Written by Kia Bazargan, University of Minnesota
// SimAnneal.h: interface for the CSimAnneal class.
//
//////////////////////////////////////////////////////////////////////
#include <vector>
#include <iomanip>
#ifndef __SIMANNEAL_H__
#define __SIMANNEAL_H__




#define HIST_LEN 20
class ShortTermAverage
{
 private:
  int curIdx_;
  int numSamples_;
  double sum_;
  double history_[HIST_LEN];
 public:
  ShortTermAverage() {curIdx_=0; numSamples_=0; sum_=0;}
  double avgWithThis(double newValue);
};

extern std::vector<int> elements;
extern std::vector<int> currentSolution;
extern std::vector<int> bestSolution;
extern int numberOfElements;
extern int targetSum;
extern double curCost;
extern std::vector<int> buckupSolution;
extern std::vector<int> buckupBMyVector;
extern std::vector<int> buckupMyVector;
extern int finalCost;
extern int subSize;
extern std::vector<int> myVector;


double currentCost();   // defined by specific prob to be annealed
void perturbSolution(); // defined by specific prob to be annealed
void buildRandomInitSolution();
void backupCurrentSolution();
void restoreBackedUpSolution();
void saveCurrentSolutionAsBestSolution();
void restoreBestSolution();

class CSimAnneal  
{
 public:
  CSimAnneal(int numNodes);
  virtual ~CSimAnneal();
  
  // ----- data access member functions
  int				tempStep()	{return temperatureStep_;}
  double			temperature()	{return T_;}			
  
  // ----- Interface methods
  void			doAnnealing();	// the actual annealing loop. It goes nonstop.

  void			initializeAnnealing();
  bool			goOneTempStep();// If you want to do the annealing core loop
   // step by step, you can call
  // this, but make sure that you call like this:
  //
  //        initializeAnnealing();
  //        while (goOneTempStep() == TRUE) ;


  void                  writeCostsInFile(FILE *outFile);
 protected:
  // ---------- protected member functions (if any)
  double			doAMove();	// Selects one of the nodes   // asks it to do a move. Should be called "after"
  // initializeAnnealing.
  bool			acceptMove(double deltaCost);	// Based on the delta cost and current temperature,
  // it decides whether to accept a move or not.
  
  void			coolDownTemperature();		// Decreases the temperature and also updates
  // scaledBoltzTimesT_. (Does not update 
  // boltzmannConst_).
  
 private:
  // ---------- data members
  
  // ---- annealing related data members
  long			numMovesEachTemp_;
  double			T_;				// The annealing temperature
  
  int				temperatureStep_;		// ith temperature value from the beginning
  
  int				expectedNumTempSteps_;		// maximum number for temperatureStep_ (Maybe 1 less,

  double			globallyBestCost_;		// The best seen (wireLen + area penal).
  
  double			boltzmannConst_;		// Used in deciding whether to accept a bad move
  double			scaledBoltzTimesT_;		// scaled boltzmann const * T_
  double			soFarAvgDeltaCost_;
  
  ShortTermAverage shortTermAvgDeltaCost_;
  int				numConsecStepsNoImprovement_;
};


#endif // #ifndef __SIMANNEAL_H__
