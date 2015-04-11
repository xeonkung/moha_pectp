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
/**
 * Tournament selection with tournament size 2 with multi-objective
 * @param pop is population that is a set of solution
 * @return pointer to solution in population
 */
Solution* selectionMO(VectorSolution pop) {
    // a population size
    int popSize = pop.size();    
    int first, second;
    // random 2 number between 0 to population size
    first = (int) (rnd->next() * popSize);
    second = (int) (rnd->next() * popSize);
    // selection by ranking value of solution
    if (pop[first]->rank < pop[second]->rank)
        return pop[first];
    else if (pop[first]->rank > pop[second]->rank)
        return pop[second];
    else {
        // this case is pop[first]->rank == pop[second]->rank
        // then we use solution's distance to comparison
        if (pop[first]->distance < pop[second]->distance)
            return pop[first];
        else
            return pop[second];
    }
}
/**
 * Tournament selection with tournament size 2
 * @param pop is population that is a set of solution
 * @return pointer to solution in population
 */
Solution* selection(VectorSolution pop) {
    // a population size
    int popSize = pop.size();
    int first, second;
    // random 2 number between 0 to population size
    first = (int) (rnd->next() * popSize);
    second = (int) (rnd->next() * popSize);
    // selection by penalty value of solution
    if (pop[first]->penalty < pop[second]->penalty)
        return (pop[first]);
    else
        return (pop[second]);
}
/**
 * Tournament selection with tournament size 5 with multi-objective
 * @param pop is population that is a set of solution
 * @return pointer to solution in population
 */
Solution* selection5MO(VectorSolution pop) {
    // a population size
    int popSize = pop.size();
    int tournament[5];
    int best;
    // get random index between 0 to population size
    tournament[0] = (int) (rnd->next() * popSize);
    // set tournament[0] to the best index
    best = tournament[0];
    for (int i = 1; i < 5; i++) {
        // get random index between 0 to population size
        tournament[i] = (int) (rnd->next() * popSize);
        // compare solution by ranking value
        if (pop[tournament[i]]->rank < pop[best]->rank) 
            best = tournament[i];
        // else if pop[first]->rank == pop[second]->rank
        // then we use solution's distance to comparison
        else if (
                    pop[tournament[i]]->rank == pop[best]->rank && 
                    pop[tournament[i]]->distance < pop[best]->distance
                )
            best = tournament[i];
    }
    return pop[best];
}
/**
 * Tournament selection with tournament size 5
 * @param pop is population that is a set of solution
 * @return pointer to solution in population
 */
Solution* selection5(VectorSolution pop) {
    // a population size
    int popSize = pop.size();
    int tournament[5];
    int best;
    // get random index between 0 to population size
    tournament[0] = (int) (rnd->next() * popSize);
    // set tournament[0] to the best index
    best = tournament[0];
    for (int i = 1; i < 5; i++) {
        // get random index between 0 to population size
        tournament[i] = (int) (rnd->next() * popSize);
        // compare solution by penalty value
        if (pop[tournament[i]]->penalty < pop[best]->penalty)
            best = tournament[i];
    }

    return (pop[best]);

}
/**
 * Comparative function between 2 solution
 * by penalty value
 * @param sol1 is pointer to solution
 * @param sol2  is pointer to solution
 * @return boolean sol1 < sol2
 */
bool compareSolution(Solution * sol1, Solution * sol2) {
    return sol1->penalty < sol2->penalty;
}
/**
 * Comparative function between 2 solution
 * by crowding distance
 * @param sol1 is pointer to solution
 * @param sol2  is pointer to solution
 * @return boolean sol1 < sol2
 */
bool compareCrowding(Solution * a, Solution * b) {
    return a->distance > b->distance;
}
/**
 * Show penalty value in each solution
 * @param pop is population that is a set of solution
 * @param os is output-stream to print out
 */
void printPOP(VectorSolution &pop, ostream &os) {
    os << "#print pop" << endl;
    // for each solution in population
    for (int i = 0; i < (int) pop.size(); i++) {
        os << "pop[" << (i + 1) << "](" << pop[i] << ")";
        os << "<";
        // show solution's penalty
        os << pop[i]->hcv << " //";
        for (int j = 0; j < Solution::obj_N; j++) {
            os << "," << pop[i]->objective[j];
        }
        // show solution's rank value
        os << ">" << "<r=" << pop[i]->rank << ">" << endl;
    }
    os << "#end pop" << endl;
}
/**
 * Show detail in each solution
 * @param pop is population that is a set of solution
 * @param os is output-stream to print out
 */
void printSolutions(VectorSolution &pop, ostream &os) {
    os << "#print solutions" << endl;
    // for each solution in population
    for (int i = 0; i < (int) pop.size(); i++) {
        os << "pop[" << (i + 1) << "](" << pop[i] << ")";
        os << "<";
        // show solution's penalty
        os << pop[i]->hcv << " //";
        for (int j = 0; j < Solution::obj_N; j++) {
            os << "," << pop[i]->objective[j];
        }
        // show solution's rank value
        os << ">" << "<r=" << pop[i]->rank << ">" << endl;
        // show room
        for (int j = 0; j < pop[i]->data->n_of_events; j++)
            os << pop[i]->sln[j].first << " ";
        os << endl;
        // show time-slot
        for (int j = 0; j < pop[i]->data->n_of_events; j++)
            os << pop[i]->sln[j].second << " ";
        os << endl;
    }
    os << "#end solution" << endl;
}
/**
 * Calculate solution's ranking value & arrange to front
 * @param pop is population
 * @param problem is pointer to problem instance
 * @param child is optional pointer to new offspring solution
 * @return Front 0 of population
 */
VectorSolution rankSolution(VectorSolution &pop, Problem* problem, Solution* child = 0) {
    // a population size
    int popSize = pop.size();
    VectorSolution new_gen, front0;
    // for each solution in population
    // and cloning
    for (int i = 0; i < popSize; i++) {
        new_gen.push_back(new Solution(problem, rnd));
        new_gen[i]->copy(pop[i]);
    }
    if (child != 0) new_gen.push_back(child);
    // rank new_gen
    Ranking rank(new_gen);
    Distance distance;
    // remaining member of population
    int remain = popSize;
    int s = 0, k, index = 0;
    // get front from rank class
    VectorSolution front = rank.Front[index];
    while ((remain > 0) && (remain >= (int) front.size())) {
        // compute solution distances in current front
        distance.crowdingDistanceAssignment(front);
        // push member of current front to population
        for (k = 0; k < (int) front.size(); k++) {
            pop[s]->copy(front[k]);
            // keep front 0
            if (index == 0) {
                front0.push_back(pop[s]);
            }
            s++;
        }
        remain -= front.size();
        index++;
        // set index to next front
        if (remain > 0) {
            front = rank.Front[index];
        }
    }
    // we have remain non-assign population
    if (remain > 0) {
        // compute solution distances in current front
        distance.crowdingDistanceAssignment(front);
        // sort front's member by distance
        sort(front.begin(), front.end(), compareCrowding);
        // push member of current front to population limited by remain
        for (k = 0; k < remain; k++) {
            pop[s]->copy(front[k]);
            if (index == 0) {
                front0.push_back(pop[s]);
            }
            s++;
        }
        remain = 0;
    }
    // free variable
    for (int i = 0; i < (int) new_gen.size(); i++) {
        delete new_gen[i];
    }
    return front0;
}
/**
 * Push a solution to archive set
 * @param a is pointer to solution
 * @param archive is target archive set
 * @param size is size of archive
 * @param pb is pointer to problem instance
 */
void pushToArchive(Solution* a, VectorSolution &archive, int size, Problem* pb) {
    vector<int> dominate;
    // find dominated solution
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
        // compute solution distances in archive set
        distance.crowdingDistanceAssignment(archive);
        // sort archive member by distance
        sort(archive.begin(), archive.end(), compareCrowding);
        // remove over size archive's member
        while ((int) archive.size() > size) {
            VectorSolution::reverse_iterator it = archive.rbegin();
            delete *it;
            archive.pop_back();
        }
    }
}
/**
 * Tabu-search 
 * @param c is pointer to soluation
 * @param control is Control instance
 * @param pb is pointer to problem instance
 * @param showcost is show cost flag
 */
void tabuSearch(Solution* current, Control &control, Problem* pb, bool showcost = false) {
    Timer timer;
    Solution *nbh_sol = new Solution(pb, rnd),      // pointer to neighbour solution
            *best_sol = new Solution(pb, rnd),      // pointer to the best solution
            *best_nbh_sol = new Solution(pb, rnd);  // pointer to the best neighbour solution
    int eventList[pb->n_of_events];                 // keep a list of events to go through
    int *tabuList = new int [pb->n_of_events];      // tabu list of events
    int ts_iter = 0;                                // tabu-search iteration
    int max_step = control.getTS_maxSteps();
    double timeLimit = control.getTimeLimit2();    
    const int ts_size = (int) (control.alfa * (double) pb->n_of_events);    // tabu length
    for (int i = 0; i < pb->n_of_events; i++) {
        tabuList[i] = -ts_size; //initialize tabu list
        eventList[i] = i;
    }
    // scramble the list of events to obtain a random order
    for (int i = 0; i < pb->n_of_events; i++) {
        int j = (int) (rnd->next() * pb->n_of_events);
        int h = eventList[i];
        eventList[i] = eventList[j];
        eventList[j] = h;
    }
    int bestMove[3];
    // flag to break a loop
    bool foundbetter = false;
    int i = -1; // current event pointer
    // copy from current solution
    best_sol->copy(current);
    //set equal to the hcv of the first found solution
    int bestHcv = current->hcv; 
    int bestScv = bestHcv == 0 ? current->scv : 99999;
    while (timer.elapsedTime(Timer::VIRTUAL) < timeLimit && ts_iter < max_step) {
        int evCount = 0; // counter of events considered
        // reset
        bestMove[0] = -1;   // event index
        bestMove[1] = 99999; // hcv
        bestMove[2] = 99999; // scv
        ts_iter++;
        while (evCount < pb->n_of_events && timer.elapsedTime(Timer::VIRTUAL) < timeLimit && ts_iter < max_step) {
            i = (i + 1) % pb->n_of_events; //next event
            int currentHcv = current->eventHcv(eventList[i]);
            int currentScv = current->eventScv(eventList[i]);
            if ((currentHcv == 0 && current->hcv != 0) || (current->hcv == 0 && currentScv == 0)) { // if the event on the list does not cause any hcv
                evCount++; // increase the counter
                continue; // go to the next event
            }//end if

            // otherwise if the event in consideration caused hcv or scv
            int t_start = (int) (rnd->next()*45); // try moves of type 1
            // get current time-slot
            int t_orig = current->sln[eventList[i]].first;
            // Search with move1 -- change new time-slot
            for (int h = 0, t = t_start; h < 45; t = (t + 1) % 45, h++) {
                if (timer.elapsedTime(Timer::VIRTUAL) > timeLimit || ts_iter > max_step) break;
                if (rnd->next() < control.getTS_N1Prob()) {
                    // copy from current solution
                    nbh_sol->copy(current);                    
                    // move1 -- change new time-slot
                    nbh_sol->Move1(eventList[i], t);
                    // find delta HCV
                    int neighbourAffectedHcv = nbh_sol->eventAffectedHcv(eventList[i]) + nbh_sol->affectedRoomInTimeslotHcv(t_orig);
                    int currentAffectedHcv = current->eventAffectedHcv(eventList[i]) + current->affectedRoomInTimeslotHcv(t);
                    int delta1Hcv = neighbourAffectedHcv - currentAffectedHcv;
                    int newHcv = current->hcv + delta1Hcv;                    
                    if (newHcv < bestHcv) {
                        // if get better HCV
                        current->copy(nbh_sol);
                        current->hcv = newHcv;
                        // set tabu-list with Tabu-search iteration number
                        tabuList[i] = ts_iter;
                        bestHcv = newHcv;
                        if (current->hcv == 0) current->computeScv();
                        evCount = 0;
                        ts_iter++;
                        foundbetter = true;
                        // reset
                        bestMove[0] = -1;
                        bestMove[1] = 99999;
                        bestMove[2] = 99999;
                        if (showcost) control.setTSCurrentCost(current, ts_iter);
                        break;
                    } else if (newHcv == 0) {
                        // if neighbour's HCV is 0
                        int newScv;
                        if (current->hcv == 0) {
                            // find delta SCV
                            int neighbourScv = nbh_sol->eventScv(eventList[i]) + // respectively Scv involving event e
                                    current->singleClassesScv(eventList[i]) - // + single classes introduced in day of original timeslot
                                    nbh_sol->singleClassesScv(eventList[i]); // - single classes "solved" in new day
                            int delta1Scv = neighbourScv - current->eventScv(eventList[i]);
                            newScv = current->scv + delta1Scv;
                        } else {
                            nbh_sol->computeScv();
                            newScv = nbh_sol->scv;
                        }
                        if (newScv < bestScv) {
                            // if get better SCV
                            current->copy(nbh_sol);
                            current->hcv = newHcv;
                            current->scv = newScv;
                            // set tabu-list with Tabu-search iteration number
                            tabuList[i] = ts_iter;
                            bestScv = newScv;
                            evCount = 0;
                            ts_iter++;
                            foundbetter = true;
                            // reset
                            bestMove[0] = -1;
                            bestMove[1] = 99999;
                            bestMove[2] = 99999;
                            if (showcost) control.setTSCurrentCost(current, ts_iter);
                            break;
                        } else if ((tabuList[i] + ts_size) <= ts_iter && newScv < bestMove[2]) {
                            // memorize the best found non improving neighbouring solution
                            best_nbh_sol->copy(nbh_sol);
                            bestMove[0] = i;
                            bestMove[1] = 0;
                            bestMove[2] = newScv;
                        }
                    } else if ((tabuList[i] + ts_size) <= ts_iter && newHcv < bestMove[1]) {
                        // memorize the best found non improving neighbouring solution
                        best_nbh_sol->copy(nbh_sol);
                        bestMove[0] = i;
                        bestMove[1] = newHcv;
                        bestMove[2] = 99999;
                    }
                }
            }
            if (foundbetter) {
                best_sol->copy(current);
                foundbetter = false;
                continue;
            }
            // Search with move2 -- swap evets time-slot
            for (int j = (i + 1) % pb->n_of_events; j != i; j = (j + 1) % pb->n_of_events) { // try moves of type 2
                if (timer.elapsedTime(Timer::VIRTUAL) > timeLimit || ts_iter > max_step) break;
                if (rnd->next() < control.getTS_N2Prob()) {
                    // copy from current solution
                    nbh_sol->copy(current);
                    //  move2 -- swap evets time-slot
                    nbh_sol->Move2(eventList[i], eventList[j]);                    
                    int newHcv;
                    if (current->hcv == 0) {
                        newHcv = nbh_sol->eventAffectedHcv(eventList[i]) +
                                nbh_sol->eventAffectedHcv(eventList[j]);
                    } else {
                        // find delta HCV
                        int neighbourAffectedHcv = nbh_sol->eventAffectedHcv(eventList[i]) +
                                nbh_sol->eventAffectedHcv(eventList[j]);
                        int currentAffectedHcv = current->eventAffectedHcv(eventList[i]) + current->eventAffectedHcv(eventList[j]);
                        int delta2Hcv = neighbourAffectedHcv - currentAffectedHcv;
                        newHcv = current->hcv + delta2Hcv;
                    }
                    if (newHcv < bestHcv) {
                        // if get better HCV
                        current->copy(nbh_sol);
                        current->hcv = newHcv;
                        // set tabu-list with Tabu-search iteration number
                        tabuList[i] = ts_iter;
                        bestHcv = newHcv;
                        if (current->hcv == 0) current->computeScv();
                        evCount = 0;
                        ts_iter++;
                        foundbetter = true;
                        // reset
                        bestMove[0] = -1;
                        bestMove[1] = 99999;
                        bestMove[2] = 99999;
                        if (showcost) control.setTSCurrentCost(current, ts_iter);
                        break;
                    } else if (newHcv == 0) {
                        // only if no hcv are introduced by the move
                        int newScv;
                        if (current->hcv == 0) {
                            // compute alterations on scv for neighbour solution
                            int neighbourScv = nbh_sol->eventScv(eventList[i]) +
                                    current->singleClassesScv(eventList[i]) -
                                    nbh_sol->singleClassesScv(eventList[i]) +
                                    nbh_sol->eventScv(eventList[j]) +
                                    current->singleClassesScv(eventList[j]) -
                                    nbh_sol->singleClassesScv(eventList[j]);

                            int delta2Scv = neighbourScv - (current->eventScv(eventList[i]) + current->eventScv(eventList[j]));
                            newScv = current->scv + delta2Scv;
                        } else {
                            nbh_sol->computeScv();
                            newScv = nbh_sol->scv;
                        }
                        if (newScv < bestScv) {
                            // if get better SCV
                            current->copy(nbh_sol);
                            current->hcv = newHcv;
                            current->scv = newScv;
                            // set tabu-list with Tabu-search iteration number
                            tabuList[i] = ts_iter;
                            bestScv = newScv;
                            evCount = 0;
                            ts_iter++;
                            foundbetter = true;
                            // reset
                            bestMove[0] = -1;
                            bestMove[1] = 99999;
                            bestMove[2] = 99999;
                            if (showcost) control.setTSCurrentCost(current, ts_iter);
                            break;
                        } else if ((tabuList[i] + ts_size) <= ts_iter && newScv < bestMove[2]) {
                            //memorize the best found non improving neighbouring solution
                            best_nbh_sol->copy(nbh_sol);
                            bestMove[0] = i;
                            bestMove[1] = 0;
                            bestMove[2] = newScv;
                        }
                    } else if ((tabuList[i] + ts_size) <= ts_iter && newHcv < bestMove[1]) {
                        //memorize the best found non improving neighbouring solution
                        best_nbh_sol->copy(nbh_sol);
                        bestMove[0] = i;
                        bestMove[1] = newHcv;
                        bestMove[2] = 99999;
                    }
                }
            }
            if (foundbetter) {
                best_sol->copy(current);
                foundbetter = false;
                continue;
            }//end if
            evCount++;
        }
        // if bestmove not empty
        if (bestMove[0] != -1) {
            // apply a current-solution with best neighbour solution
            current->copy(best_nbh_sol);
            current->hcv = bestMove[1];
            current->scv = bestMove[2];
            bestMove[0] = ts_iter;
        } else if (timer.elapsedTime(Timer::VIRTUAL) < timeLimit && ts_iter < max_step) {
            // if not the end!! -- make the different with random Move
            current->randomMove();
            current->computeHcv();
            if (current->hcv == 0) {
                current->computeScv();
                if (showcost) control.setTSCurrentCost(current, ts_iter);
            }
        }
    }
    current->copy(best_sol);
    if (showcost) control.getOutputStream() << "TS total Iteration:" << ts_iter << endl;
    delete nbh_sol;
    delete best_nbh_sol;
    delete best_sol;
    delete []tabuList;
}
/**
 * Tabu-search with Multi-Objective
 * @param archiveSet is Archive set
 * @param archSize is Archive set's size
 * @param control is control instance
 * @param pb is pointer to problem instance
 */
void tabuSearchMO(VectorSolution &archiveSet, int archSize, Control &control, Problem* pb) {
    Timer timer;
    VectorSolution temp;
    Solution *nbh_sol = new Solution(pb, rnd),      // pointer to neighbour solution
            *best_sol = new Solution(pb, rnd),      // pointer to the best solution
            *best_nbh_sol = new Solution(pb, rnd);  // pointer to the best neighbour solution
    int eventList[pb->n_of_events];                 // keep a list of events to go through
    int tabuList[archSize][pb->n_of_events];        // tabu list of events
    int ts_iter = 0;                                // tabu-search iteration
    int max_step = control.getTS_maxSteps();
    double limitTime = control.getTimeLimit2();
    const int ts_size = (int) (control.alfa * (double) pb->n_of_events);    // tabu length
    for (int i = 0; i < pb->n_of_events; i++) {
        for (int k = 0; k < archSize; k++) {
            tabuList[k][i] = -ts_size; //initialize tabu list
        }
        eventList[i] = i;
    }
    // scramble the list of events to obtain a random order
    for (int i = 0; i < pb->n_of_events; i++) { 
        int j = (int) (rnd->next() * pb->n_of_events);
        int h = eventList[i];
        eventList[i] = eventList[j];
        eventList[j] = h;
    }
    int bestMove[3];
    // flag to break a loop
    int foundbetter = false;
    int i = -1; // current event pointer
    while (timer.elapsedTime(Timer::VIRTUAL) < limitTime && ts_iter < max_step) {
        int evCount = 0; // counter of events considered
        // reset
        bestMove[0] = -1;   // event index
        bestMove[1] = 9999; // hcv
        bestMove[2] = 9999; // scv
        int bestHcv = 9999; // set equal to the hcv of the first found solution
        int bestScv = 9999;
        ts_iter++;
        for (int k = 0; k < (int) archiveSet.size(); ++k) {
            temp.push_back(new Solution(pb, rnd));
            temp[k]->copy(archiveSet[k]);
            while (evCount < pb->n_of_events && timer.elapsedTime(Timer::VIRTUAL) < limitTime && ts_iter < max_step) {
                i = (i + 1) % pb->n_of_events; //next event
                int currentHcv = temp[k]->eventHcv(eventList[i]);
                int currentScv = temp[k]->eventScv(eventList[i]);
                if ((currentHcv == 0 && temp[k]->hcv != 0) || (temp[k]->hcv == 0 && currentScv == 0)) { // if the event on the list does not cause any hcv
                    evCount++; // increase the counter
                    continue; // go to the next event
                }//end if

                // otherwise if the event in consideration caused hcv or scv
                int t_start = (int) (rnd->next()*45); // try moves of type 1
                // get current time-slot
                int t_orig = temp[k]->sln[eventList[i]].first;
                // Search with move1 -- change new time-slot
                for (int h = 0, t = t_start; h < 45; t = (t + 1) % 45, h++) {
                    if (timer.elapsedTime(Timer::VIRTUAL) > limitTime) break;
                    if (rnd->next() < control.getTS_N1Prob()) {
                        // copy from current solution
                        nbh_sol->copy(temp[k]);
                        // move1 -- change new time-slot
                        nbh_sol->Move1(eventList[i], t);
                        // find delta HCV
                        int neighbourAffectedHcv = nbh_sol->eventAffectedHcv(eventList[i]) + nbh_sol->affectedRoomInTimeslotHcv(t_orig);
                        int currentAffectedHcv = temp[k]->eventAffectedHcv(eventList[i]) + temp[k]->affectedRoomInTimeslotHcv(t);
                        int delta1Hcv = neighbourAffectedHcv - currentAffectedHcv;
                        int newHcv = temp[k]->hcv + delta1Hcv;
                        if (newHcv < bestHcv) {
                            // if get better HCV
                            temp[k]->copy(nbh_sol);
                            temp[k]->hcv = newHcv;
                            // set tabu-list with Tabu-search iteration number
                            tabuList[k][i] = ts_iter;
                            bestHcv = newHcv;
                            if (temp[k]->hcv == 0) temp[k]->computeScv();
                            evCount = 0;
                            //ts_iter++;
                            foundbetter = true;
                            // reset
                            bestMove[0] = -1;
                            bestMove[1] = 9999;
                            bestMove[2] = 9999;
                            break;
                        } else if (newHcv == 0) {
                            // if neighbour's HCV is 0
                            int newScv;
                            if (temp[k]->hcv == 0) {
                                // find delta SCV
                                int neighbourScv = nbh_sol->eventScv(eventList[i]) + // respectively Scv involving event e
                                        temp[k]->singleClassesScv(eventList[i]) - // + single classes introduced in day of original timeslot
                                        nbh_sol->singleClassesScv(eventList[i]); // - single classes "solved" in new day
                                int delta1Scv = neighbourScv - temp[k]->eventScv(eventList[i]);
                                newScv = temp[k]->scv + delta1Scv;
                            } else {
                                nbh_sol->computeScv();
                                newScv = nbh_sol->scv;
                            }
                            if (newScv < bestScv) {
                                // if get better SCV
                                temp[k]->copy(nbh_sol);
                                temp[k]->hcv = newHcv;
                                temp[k]->scv = newScv;
                                // set tabu-list with Tabu-search iteration number
                                tabuList[k][i] = ts_iter;
                                bestScv = newScv;
                                evCount = 0;
                                //ts_iter++;
                                foundbetter = true;
                                // reset
                                bestMove[0] = -1;
                                bestMove[1] = 9999;
                                bestMove[2] = 9999;
                                break;
                            } else if ((tabuList[k][i] + ts_size) <= ts_iter && newScv < bestMove[2]) {
                                //memorize the best found non improving neighbouring solution
                                best_nbh_sol->copy(nbh_sol);
                                bestMove[0] = i;
                                bestMove[1] = 0;
                                bestMove[2] = newScv;
                            }
                        } else if ((tabuList[k][i] + ts_size) <= ts_iter && newHcv < bestMove[1]) {
                            // memorize the best found non improving neighbouring solution
                            best_nbh_sol->copy(nbh_sol);
                            bestMove[0] = i;
                            bestMove[1] = newHcv;
                            bestMove[2] = 9999;
                        }
                    }
                }
                if (foundbetter) {
                    best_sol->copy(temp[k]);
                    foundbetter = false;
                    //continue;
                    break;
                }
                // Search with move2 -- swap evets time-slot
                for (int j = (i + 1) % pb->n_of_events; j != i; j = (j + 1) % pb->n_of_events) { // try moves of type 2
                    if (timer.elapsedTime(Timer::VIRTUAL) > limitTime) break;
                    if (rnd->next() < control.getTS_N2Prob()) {
                        // copy from current solution
                        nbh_sol->copy(temp[k]);
                        //  move2 -- swap evets time-slot
                        nbh_sol->Move2(eventList[i], eventList[j]);
                        int newHcv;
                        if (temp[k]->hcv == 0) {
                            newHcv = nbh_sol->eventAffectedHcv(eventList[i]) +
                                    nbh_sol->eventAffectedHcv(eventList[j]);
                        } else {
                            // find delta HCV
                            int neighbourAffectedHcv = nbh_sol->eventAffectedHcv(eventList[i]) +
                                    nbh_sol->eventAffectedHcv(eventList[j]);
                            int currentAffectedHcv = temp[k]->eventAffectedHcv(eventList[i]) + temp[k]->eventAffectedHcv(eventList[j]);
                            int delta2Hcv = neighbourAffectedHcv - currentAffectedHcv;
                            newHcv = temp[k]->hcv + delta2Hcv;
                        }
                        if (newHcv < bestHcv) {
                            // if get better HCV
                            temp[k]->copy(nbh_sol);
                            temp[k]->hcv = newHcv;
                            // set tabu-list with Tabu-search iteration number
                            tabuList[k][i] = ts_iter;
                            bestHcv = newHcv;
                            if (temp[k]->hcv == 0) temp[k]->computeScv();
                            evCount = 0;
                            //ts_iter++;
                            foundbetter = true;
                            // reset
                            bestMove[0] = -1;
                            bestMove[1] = 9999;
                            bestMove[2] = 9999;
                            break;
                        } else if (newHcv == 0) {// only if no hcv are introduced by the move
                            int newScv;
                            if (temp[k]->hcv == 0) {
                                // compute alterations on scv for neighbour solution
                                int neighbourScv = nbh_sol->eventScv(eventList[i]) +
                                        temp[k]->singleClassesScv(eventList[i]) -
                                        nbh_sol->singleClassesScv(eventList[i]) +
                                        nbh_sol->eventScv(eventList[j]) +
                                        temp[k]->singleClassesScv(eventList[j]) -
                                        nbh_sol->singleClassesScv(eventList[j]);

                                int delta2Scv = neighbourScv - (temp[k]->eventScv(eventList[i]) + temp[k]->eventScv(eventList[j]));
                                newScv = temp[k]->scv + delta2Scv;
                            } else {
                                nbh_sol->computeScv();
                                newScv = nbh_sol->scv;
                            }
                            if (newScv < bestScv) {
                                // if get better SCV
                                temp[k]->copy(nbh_sol);
                                temp[k]->hcv = newHcv;
                                temp[k]->scv = newScv;
                                // set tabu-list with Tabu-search iteration number
                                tabuList[k][i] = ts_iter;
                                bestScv = newScv;
                                evCount = 0;
                                //ts_iter++;
                                foundbetter = true;
                                // reset
                                bestMove[0] = -1;
                                bestMove[1] = 9999;
                                bestMove[2] = 9999;
                                break;
                            } else if ((tabuList[k][i] + ts_size) <= ts_iter && newScv < bestMove[2]) {
                                //memorize the best found non improving neighbouring solution
                                best_nbh_sol->copy(nbh_sol);
                                bestMove[0] = i;
                                bestMove[1] = 0;
                                bestMove[2] = newScv;
                            }
                        } else if ((tabuList[k][i] + ts_size) <= ts_iter && newHcv < bestMove[1]) {
                            //memorize the best found non improving neighbouring solution
                            best_nbh_sol->copy(nbh_sol);
                            bestMove[0] = i;
                            bestMove[1] = newHcv;
                            bestMove[2] = 9999;
                        }
                    }
                }//end if
                if (foundbetter) {
                    best_sol->copy(temp[k]);
                    foundbetter = false;
                    //continue;
                    break;
                }//end if
                evCount++;
            }
            // apply a current-solution with best solution
            temp[k]->copy(best_sol);
        }
        // apply to archiveSet
        for (int k = 0; k < (int) temp.size(); ++k) {
            pushToArchive(temp[k], archiveSet, archSize, pb);
            delete temp[k];
        }
        temp.clear();
        // print solution
        control.setCurrentCost(archiveSet[0]);
    }
    delete nbh_sol;
    delete best_nbh_sol;
    delete best_sol;
}
/**
 * Multi-Objective GA
 * @param control is Control instamce
 */
void MOGA(Control &control) {
//    cout << "Start MOGA" << endl;
    // get output stream from control instance
    ostream& os = control.getOutputStream();
    // get population size
    int popSize = control.getPOPSize();
    // set archive-set size
    int archSize = 20;
    // get problem instance
    Problem *problem = new Problem(control.getInputStream());
    // set random instance
    rnd = new Random((unsigned) control.getSeed());
    while (control.triesLeft()) {
        // set control beginning point
        control.beginTry();
        int generation = 0;
        VectorSolution popu, front0, archiveSet;
        // Random generate solution
        for (int i = 0; i < popSize; i++) {
            popu.push_back(new Solution(problem, rnd));
            popu[i]->RandomInitialSolution();
            if (control.flag["LS1"] || control.flag["LS1E"])
                popu[i]->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3(), control.flag["LS1E"]);
            if (control.flag["LS2"])
                popu[i]->LS2(control.getMaxSteps(), control.getTimeLimit());
            // calculate penalty value
            popu[i]->computePenalty();
        }
        // find front0
        front0 = rankSolution(popu, problem);
        // create archive-set
        for (int i = 0; i < (int) front0.size() && i < archSize; i++) {
            archiveSet.push_back(new Solution(problem, rnd));
            archiveSet[i]->copy(front0[i]);
        }
        // set & show best solution
        control.setCurrentCost(popu[0]);
        if (control.getMethod() == Control::METHOD_NSGA || control.getMethod() == Control::METHOD_MOHA || control.getMethod() == Control::METHOD_MOHA2) {
            while (control.timeLeft()) {
                // start reproduction (steady-state GA)
                Solution* child = new Solution(problem, rnd);
                // select parents
                Solution* parent1 = selection5MO(popu);
                Solution* parent2 = selection5MO(popu);
                // generate child
                if (rnd->next() < control.getPC())
                    child->crossover(parent1, parent2);
                else {
                    child->copy(parent1);
                }
                // do some mutation
                if (rnd->next() < control.getPM()) {
                    child->mutation();
                }
                //apply local search to offspring
                if (control.flag["LS1"] || control.flag["LS1E"])
                    child->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3(), control.flag["LS1E"]);
                if (control.flag["LS2"])
                    child->LS2(control.getMaxSteps(), control.getTimeLimit());
                if (control.getMethod() == Control::METHOD_MOHA2)
                    tabuSearch(child, control, problem);
                //child->tabuSearch(10, control.alfa);
                //evaluate the offspring
                child->computePenalty();
                generation++;
                //new_gen
                pushToArchive(child, archiveSet, archSize, problem);
                front0 = rankSolution(popu, problem, child);
                control.setCurrentCost(popu[0]);
            }// end while            
            //control.endTry(front0);
            //printSolutions(archiveSet, control.getOutputStream());
        }
        if (control.getMethod() == Control::METHOD_MOHA || control.getMethod() == Control::METHOD_MOTS) {
            os << "MOTS started!!" << endl;
            tabuSearchMO(archiveSet, archSize, control, problem);
        }
        control.endTry(archiveSet);
        // remember to delete the population
        for (int i = 0; i < popSize; i++) {
            delete popu[i];
        }
        for (int i = 0; i < (int) archiveSet.size(); i++) {
            delete archiveSet[i];
        }
    }
    delete problem;
    delete rnd;
}
/**
 * GA method
 * @param control is Control instance
 */
void GA(Control &control) {
//    cout << "Start GA" << endl;
    // get population size
    int popSize = control.getPOPSize();
    // get problem instance
    Problem *problem = new Problem(control.getInputStream());
    // set random instance
    rnd = new Random((unsigned) control.getSeed());
    while (control.triesLeft()) {
        // set control beginning point
        control.beginTry();
        int generation = 0;
        VectorSolution pop;
        // Random generate solution
        for (int i = 0; i < popSize; i++) {
            pop.push_back(new Solution(problem, rnd));
            pop[i]->RandomInitialSolution();
            if (control.flag["LS1"] || control.flag["LS1E"])
                pop[i]->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3(), control.flag["LS1E"]);
            if (control.flag["LS2"])
                pop[i]->LS2(control.getMaxSteps(), control.getTimeLimit());
            // calculate penalty value
            pop[i]->computePenalty();
        }
        // sort population
        sort(pop.begin(), pop.end(), compareSolution);
        // set & show best solution
        control.setCurrentCost(pop[0]);
        while (control.timeLeft()) {
            // start reproduction (steady-state GA)
            Solution* child = new Solution(problem, rnd);
            // select parents
            Solution* parent1 = selection5(pop);
            Solution* parent2 = selection5(pop);
            // generate child
            if (rnd->next() < control.getPC())
                child->crossover(parent1, parent2);
            else {
                child->copy(parent1);
            }
            // do some mutation
            if (rnd->next() < control.getPM()) {
                child->mutation();
            }
            //apply local search to offspring
            //if (control.flag["LS1"] || control.flag["LS1E"])
            //    child->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3(), control.flag["LS1E"]);
            //if (control.flag["LS2"])
            //    child->LS2(control.getMaxSteps(), control.getTimeLimit());
            //tabu
            if (control.getMethod() == Control::METHOD_SSGATS)
                tabuSearch(child, control, problem);
            //evaluate the offspring
            child->computePenalty();
            generation++;
            // replace worst member of the population with offspring
            pop[popSize - 1]->copy(child);
            sort(pop.begin(), pop.end(), compareSolution);
            control.setCurrentCost(pop[0]);
            delete child;
        }
        if (control.getMethod() == Control::METHOD_GA_TS) {
            control.getOutputStream() << "Start TS" << endl;
            tabuSearch(pop[0], control, problem, true);
        }
        control.endTry(pop[0]);
        for (int i = 0; i < popSize; i++) {
            delete pop[i];
        }
    }
    delete problem;
    delete rnd;
}
/**
 * Pure Tabu-search
 * @param control is control instance
 */
void TS(Control &control) {
//    cout << "Start TS" << endl;
    // get problem instance
    Problem *problem = new Problem(control.getInputStream());
    // set random instance
    rnd = new Random((unsigned) control.getSeed());
    while (control.triesLeft()) {
        // set control beginning point
        control.beginTry();
        // initial solution
        Solution *sol = new Solution(problem, rnd);
        sol->RandomInitialSolution();
        sol->computePenalty();
        // set & show best solution
        control.setCurrentCost(sol);
        // tabuSearch 
        tabuSearch(sol, control, problem, true);
        control.endTry(sol);
        delete sol;
    }

    delete problem;
    delete rnd;
}

int main(int argc, char** argv) {    
    Control control(argc, argv);
    if (control.getMethod() / 300 == 1) TS(control);
    else if (control.getMethod() / 200 == 1) GA(control);
    else if (control.getMethod() / 100 == 1) MOGA(control);
}


