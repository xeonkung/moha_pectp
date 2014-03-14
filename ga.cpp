#include <stdlib.h>

#include "Control.h"
#include "Problem.h"
#include "Solution.h"
#include "Ranking.h"
#include "Distance.h"
#include <fstream>

#include <list>
#include <vector>
using namespace std;
Random* rnd;

/*class compareSolution{
public:
  bool operator()(Solution* sol1, Solution* sol2) const
  { return sol1->penalty < sol2->penalty;}
  }*/

Solution* selection(VectorSolution pop){
    int popSize = pop.size();
  // tournament selection with tornament size 2
  int first, second;
  first = (int)(rnd->next()*popSize);
  second = (int)(rnd->next()*popSize);
  
  if(pop[first]->penalty < pop[second]->penalty)
      return(pop[first]);
  else
      return(pop[second]);
 
}

Solution* selection5(VectorSolution pop){
     int popSize = pop.size();
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

bool compareCrowding(Solution * a, Solution * b){
    return a->distance < b->distance;
}

int main( int argc, char** argv) {

  Control control(argc, argv);

  int problemType = control.getProblemType(); 
  int popSize = 10;
  
  Problem *problem = new Problem(control.getInputStream());

  rnd = new Random((unsigned) control.getSeed());

  while( control.triesLeft()){
    control.beginTry();

    int generation = 0;

    VectorSolution pop;

    for(int i=0; i < popSize; i++){       
      pop.push_back(new Solution(problem, rnd));
      pop[i]->RandomInitialSolution();
      //pop[i]->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());
      pop[i]->computePenalty();
    }   
    control.setCurrentCost(pop[0]);

    while(control.timeLeft()){

      // start reproduction (steady-state GA)
      Solution* child= new Solution(problem,rnd);

      // select parents
      Solution* parent1 = selection5(pop, popSize);
      Solution* parent2 = selection5(pop, popSize);

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
      
      //child->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());      
      //child->LS2(maxSteps, control.getTimeLimit());
      //child->tabuSearch(10, control.alfa);
      //evaluate the offspring
      child->computePenalty();
      //cout << "Child " << child->penalty << endl; 
      //cout<< "Parent1 "<< parent1->penalty<< " Parent2 " << parent2->penalty<<endl;
      generation++;
      pop.push_back(child);
      Ranking rank (pop);
      Distance distance;
      pop.clear();
      int remain = popSize;
      int k,index = 0;
      VectorSolution front = rank.Front[index];
      while((remain > 0) && (remain >= front.size())){
          distance.crowdingDistanceAssignment(front);
          for(k = 0; k < front.size(); k ++){
              pop.push_back(front[k]);
          }
          remain -= front.size();
          index++;
          if (remain > 0){
              front = rank.Front[index];
          }
      }
      if(remain > 0){
          distance.crowdingDistanceAssignment(front);
          sort(front.begin(), front.end(), compareCrowding);
          for(k = 0; k < remain; k ++){
              pop.push_back(front[k]);
          }
          remain = 0;
          while(k < front.size()){
              delete front[k];
              k++;
          }
          index++;
      }
      while(index < rank.Front.size()){
          for(k = 0; k < remain; k ++){
              delete rank.Front[index][k];
          }
          index++;
      }
	//cout<< "generation " << generation << endl;
	control.setCurrentCost(pop[0]);
      //delete child; 
    }// while
    
    control.endTry(pop[0]);

    // remember to delete the population
    for(int i=0; i < popSize; i++){
      delete pop[i];
    }
  }

  delete problem;
  delete rnd;
  
}


