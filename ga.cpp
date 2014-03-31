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

Solution* selection(VectorSolution pop) {
    int popSize = pop.size();
    // tournament selection with tornament size 2
    int first, second;
    first = (int) (rnd->next() * popSize);
    second = (int) (rnd->next() * popSize);

    if (pop[first]->rank < pop[second]->rank)
        return pop[first];
    else if (pop[first]->rank > pop[second]->rank)
        return pop[second];
    else {
        if (pop[first]->distance < pop[second]->distance)
            return pop[first];
        else
            return pop[second];
    }
}

Solution* selection5(VectorSolution pop) {
    int popSize = pop.size();
    // tournament selection with tornament size 5
    int tournament[5];
    int best;

    tournament[0] = (int) (rnd->next() * popSize);

    best = tournament[0];
    for (int i = 1; i < 5; i++) {
        tournament[i] = (int) (rnd->next() * popSize);
        if (pop[tournament[i]]->penalty < pop[best]->penalty)
            best = tournament[i];
    }

    return (pop[best]);

}

bool compareSolution(Solution * sol1, Solution * sol2) {
    return sol1->penalty < sol2->penalty;
}

bool compareCrowding(Solution * a, Solution * b) {
    return a->distance < b->distance;
}

void printPOP(VectorSolution &pop, ostream &os) {
    os << "#print pop" << endl;
    for (int i = 0; i < (int) pop.size(); i++) {
        os << "pop[" << (i+1) << "](" << pop[i] << ")";
        os << "<";
        os << pop[i]->hcv << " //";
        for (int j = 0; j < Solution::obj_N; j++) {
            os << "," << pop[i]->objective[j];
        }
        os << ">" << "<r=" << pop[i]->rank << ">" << endl;
    }
    os << "#end pop" << endl;
}

void printSolutions(VectorSolution &pop, ostream &os) {
    os << "#print solutions" << endl;
    for (int i = 0; i < (int) pop.size(); i++) {
        os << "pop[" << (i+1) << "](" << pop[i] << ")";
        os << "<";
        os << pop[i]->hcv << " //";
        for (int j = 0; j < Solution::obj_N; j++) {
            os << "," << pop[i]->objective[j];
        }
        os << ">" << "<r=" << pop[i]->rank << ">" << endl;
        for (int j = 0; j < pop[i]->data->n_of_events; j++)
            os<< pop[i]->sln[j].first << " ";
        os << endl;
        for (int j = 0; j < pop[i]->data->n_of_events; j++)
            os << pop[i]->sln[j].second << " ";
        os << endl;
    }
    os << "#end solution" << endl;
}

VectorSolution rankSolution(VectorSolution &pop, Problem* problem, Solution* child = 0) {
    int popSize = pop.size();
    VectorSolution new_gen, front0;
    for (int i = 0; i < popSize; i++) {
        new_gen.push_back(new Solution(problem, rnd));
        new_gen[i]->copy(pop[i]);
    }
    if (child != 0) new_gen.push_back(child);
    Ranking rank(new_gen);
    Distance distance;
    int remain = popSize;
    int s = 0, k, index = 0;
    VectorSolution front = rank.Front[index];
    while ((remain > 0) && (remain >= (int) front.size())) {
        distance.crowdingDistanceAssignment(front);
        for (k = 0; k < (int) front.size(); k++) {
            pop[s]->copy(front[k]);
            if (index == 0) {
                front0.push_back(pop[s]);
            }
            s++;
        }
        remain -= front.size();
        index++;
        if (remain > 0) {
            front = rank.Front[index];
        }
    }
    if (remain > 0) {
        distance.crowdingDistanceAssignment(front);
        sort(front.begin(), front.end(), compareCrowding);
        for (k = 0; k < remain; k++) {
            pop[s]->copy(front[k]);
            if (index == 0) {
                front0.push_back(pop[s]);
            }
            s++;
        }
        remain = 0;
    }
    for (int i = 0; i < (int) new_gen.size(); i++) {
        delete new_gen[i];
    }
    return front0;
}

void pushToArchive(Solution* a, VectorSolution &archive, int size, Problem* pb) {
    vector<int> dominate;
    for (int i = 0; i < (int) archive.size(); i++) {
        int flag = Ranking::compareOverall(a, archive[i]);
        if (flag == 0) {
            flag = Ranking::compareDominateSolution(a, archive[i]);
        }
        if (flag == -1) {
            dominate.push_back(i);
        } else if (flag == 0) {
            if (a->equ(archive[i])) return;
        } else {
            return;
        }
    }
    // discard Dominated solution
    for (int i = 0; i < (int) dominate.size(); i++) {
        int j = dominate[i] - i;
        delete archive[j];
        archive.erase(archive.begin() + j);
    }
    Solution* temp = new Solution(pb, rnd);
    temp->copy(a);
    temp->rank = -1;
    archive.push_back(temp);

    // adjust size
    if ((int) archive.size() > size) {
        Distance distance;
        distance.crowdingDistanceAssignment(archive);
        sort(archive.begin(), archive.end(), compareCrowding);
        while ((int) archive.size() > size) {
            VectorSolution::reverse_iterator it = archive.rbegin();
            delete *it;
            archive.pop_back();
        }
    }
}

int main(int argc, char** argv) {

    Control control(argc, argv);
    
    //int problemType = control.getProblemType(); 
    int popSize = 50;
    int archSize = 10;
    Problem *problem = new Problem(control.getInputStream());

    rnd = new Random((unsigned) control.getSeed());
    while (control.triesLeft()) {
        control.beginTry();
        int generation = 0;        
        VectorSolution pop, front0, archiveSet;
        // Random generate solution
        for (int i = 0; i < popSize; i++) {
            pop.push_back(new Solution(problem, rnd));
            pop[i]->RandomInitialSolution();
            pop[i]->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());
            pop[i]->computePenalty();
        }
        front0 = rankSolution(pop, problem);
        for (int i = 0; i < (int) front0.size() && i < archSize; i++) {
            archiveSet.push_back(new Solution(problem, rnd));
            archiveSet[i]->copy(front0[i]);
        }
        control.setCurrentCost(pop[0]);
        while (control.timeLeft()) {
            
            // start reproduction (steady-state GA)
            Solution* child = new Solution(problem, rnd);

            // select parents
            Solution* parent1 = selection5(pop);
            Solution* parent2 = selection5(pop);

            // generate child
            if (rnd->next() < 0.9)
                child->crossover(parent1, parent2);
            else {
                child->copy(parent1);
            }

            // do some mutation
            if (rnd->next() < 0.6) {
                child->mutation();
            }

            //apply local search to offspring

            child->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());
            //child->LS2(maxSteps, control.getTimeLimit());
            //child->tabuSearch(10, control.alfa);
            //evaluate the offspring
            child->computePenalty();
            generation++;
            //new_gen
            pushToArchive(child, archiveSet, archSize, problem);
            front0 = rankSolution(pop, problem, child);
            control.setCurrentCost(pop[0]);
        }// while

        control.endTry(front0);
        printSolutions(archiveSet, control.getOutputStream());
        // remember to delete the population
        for (int i = 0; i < popSize; i++) {
            delete pop[i];
        }
        for (int i = 0; i < (int)archiveSet.size(); i++) {
            delete archiveSet[i];
        }
    }

    delete problem;
    delete rnd;

}


