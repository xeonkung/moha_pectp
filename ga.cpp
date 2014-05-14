#include <stdlib.h>

#include "Control.h"
#include "Problem.h"
#include "Solution.h"

#include <fstream>

#include <list>
#include <vector>
using namespace std;
Random* rnd;
extern ostream *outs;

/*class compareSolution{
public:
  bool operator()(Solution* sol1, Solution* sol2) const
  { return sol1->penalty < sol2->penalty;}
  }*/

Solution* selection(Solution** pop, int popSize ){

  // tournament selection with tornament size 2
  int first, second;
  first = (int)(rnd->next()*popSize);
  second = (int)(rnd->next()*popSize);
  
  if(pop[first]->penalty < pop[second]->penalty)
      return(pop[first]);
  else
      return(pop[second]);
 
}

Solution* selection5(Solution** pop, int popSize ){

  // tournament selection with tornament size 5
  int tournament[5];
  int best;

  tournament[0] = (int)(rnd->next()*popSize);
  
  best =  tournament[0];
  for(int i= 1; i<5; i++){
    tournament[i] = (int)(rnd->next()*popSize);
    if(pop[tournament[i]]->penalty < pop[best]->penalty)
      best = tournament[i];
  }

  return(pop[best]);
 
}

bool compareSolution(Solution * sol1, Solution * sol2)
{
 return sol1->penalty < sol2->penalty;
}

int main( int argc, char** argv) {

  Control control(argc, argv);
  outs = control.os;
  int popSize = 50;
//  int maxSteps = control.getMaxSteps();
  
  Problem *problem = new Problem(control.getInputStream());

  rnd = new Random((unsigned) control.getSeed());

  while( control.triesLeft()){
    control.beginTry();

    int generation = 0;

    Solution* pop[popSize];

    for(int i=0; i < popSize; i++){
      pop[i] = new Solution(problem, rnd);
      pop[i]->RandomInitialSolution();
      pop[i]->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());
      //pop[i]->LS2(maxSteps, control.getTimeLimit());
      //pop[i]->tabuSearch(10, control.alfa);
      pop[i]->computePenalty();
      //cout<< pop[i]->penalty<<endl;
    }
   
    // sort the population by penalty
    sort(pop, pop + popSize, compareSolution);

    /*for(int i=0; i < popSize; i++){
      cout<< pop[i]->penalty<<endl;
      }*/ 

    control.setCurrentCost(pop[0]);

    while(control.timeLeft()){

      // start reproduction (steady-state GA)
      Solution* child= new Solution(problem,rnd);

      // select parents
      Solution* parent1 = selection(pop, popSize);
      Solution* parent2 = selection(pop, popSize);

      // generate child
      if(rnd->next() < 0.8)
	child->crossover(parent1,parent2);
      else{
	child->copy(parent1);
      }

      // do some mutation
      if(rnd->next() < 0.5){
	  child->mutation();
      }
     
      //apply local search to offspring
      
      child->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());      
      //child->LS2(maxSteps, control.getTimeLimit());
      //child->tabuSearch(10, control.alfa);
      //evaluate the offspring
      child->computePenalty();
      //cout << "Child " << child->penalty << endl; 
      //cout<< "Parent1 "<< parent1->penalty<< " Parent2 " << parent2->penalty<<endl;
      generation ++;
      // replace worst member of the population with offspring  
      //if(child->penalty < pop[popSize - 1]->penalty){
	pop[popSize - 1]->copy(child);
	sort(pop, pop + popSize, compareSolution);
	//cout<< "generation " << generation << endl;
	control.setCurrentCost(pop[0]);
	//}
      // replace if better
	// if(parent1->penalty < parent2->penalty){
	//if(child->penalty < parent2->penalty ){
	 // parent2->copy(child);
	//}
      //}
      //else{
	//if(child->penalty < parent1->penalty ){
	  //parent1->copy(child);
	//}
	//}
      //cout<<"New elements in the pop " << parent1->penalty<< " " << parent2->penalty<< endl;

      // sort the new pop
	
      delete child; 
    }
    control.endTry(pop[0]);
        if (control.getTimeLimit2() > 0) {
            (*outs) << "[--[ Start Tabu Search ]--]" << endl;
            pop[0]->tabuSearch(control.getTimeLimit2(), control.alfa, 0.1, 0.1);
            pop[0]->computePenalty();
            control.endTry2(pop[0]);
            // remember to delete the population
            (*outs) << "[--[ End ]--] Total Time:" << control.getTime() << endl;
        }
    for(int i=0; i < popSize; i++){
      delete pop[i];
    }
  }

  delete problem;
  delete rnd;
  
}


