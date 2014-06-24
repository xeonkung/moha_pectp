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

Solution* selectionMO(VectorSolution pop) {
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

Solution* selection(VectorSolution pop) {
    int popSize = pop.size();
    // tournament selection with tornament size 2
    int first, second;
    first = (int) (rnd->next() * popSize);
    second = (int) (rnd->next() * popSize);
    if (pop[first]->penalty < pop[second]->penalty)
        return (pop[first]);
    else
        return (pop[second]);
}

Solution* selection5MO(VectorSolution pop) {
    int popSize = pop.size();
    // tournament selection with tornament size 5
    int tournament[5];
    int best;

    tournament[0] = (int) (rnd->next() * popSize);

    best = tournament[0];
    for (int i = 1; i < 5; i++) {
        tournament[i] = (int) (rnd->next() * popSize);
        if (pop[tournament[i]]->rank < pop[best]->rank) best = tournament[i];
        else if (pop[tournament[i]]->rank == pop[best]->rank)
            if (pop[tournament[i]]->distance < pop[best]->distance) best = tournament[i];
    }
    return (pop[best]);
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
        os << "pop[" << (i + 1) << "](" << pop[i] << ")";
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
        os << "pop[" << (i + 1) << "](" << pop[i] << ")";
        os << "<";
        os << pop[i]->hcv << " //";
        for (int j = 0; j < Solution::obj_N; j++) {
            os << "," << pop[i]->objective[j];
        }
        os << ">" << "<r=" << pop[i]->rank << ">" << endl;
        for (int j = 0; j < pop[i]->data->n_of_events; j++)
            os << pop[i]->sln[j].first << " ";
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

void tabuSearch(Solution* c, Control &control, Problem* pb) {
    Timer timer;
    Solution *nbh_sol = new Solution(pb, rnd),
            *best_sol = new Solution(pb, rnd),
            *best_nbh_sol = new Solution(pb, rnd);
    int eventList[pb->n_of_events]; // keep a list of events to go through
    int *tabuList = new int [pb->n_of_events]; // tabu list of events
    int ts_iter = 0;
    int max_step = control.getTS_maxSteps();
    double timeLimit = 9999.0;
    const int ts_size = (int) (control.alfa * (double) pb->n_of_events);
    for (int i = 0; i < pb->n_of_events; i++) {
        tabuList[i] = -ts_size; //initialize tabu list
        eventList[i] = i;
    }
    for (int i = 0; i < pb->n_of_events; i++) { // scramble the list of events to obtain a random order
        int j = (int) (rnd->next() * pb->n_of_events);
        int h = eventList[i];
        eventList[i] = eventList[j];
        eventList[j] = h;
    }
    int bestMove[3];
    int foundbetter = false;
    int i = -1; // current event pointer
    best_sol->copy(c);
    int bestHcv = c->hcv; //set equal to the hcv of the first found solution
    int bestScv;
    bestScv = bestHcv != 0 ? c->scv : 99999;
    while (timer.elapsedTime(Timer::VIRTUAL) < timeLimit && ts_iter < max_step) {
        int evCount = 0; // counter of events considered
        // reset
        bestMove[0] = -1;
        bestMove[1] = 9999;
        bestMove[2] = 9999;
        ts_iter++;
        while (evCount < pb->n_of_events && timer.elapsedTime(Timer::VIRTUAL) < timeLimit && ts_iter < max_step) {
            i = (i + 1) % pb->n_of_events; //next event
            int currentHcv = c->eventHcv(eventList[i]);
            int currentScv = c->eventScv(eventList[i]);
            if ((currentHcv == 0 && c->hcv != 0) || (c->hcv == 0 && currentScv == 0)) { // if the event on the list does not cause any hcv
                evCount++; // increase the counter
                continue; // go to the next event
            }//end if

            // otherwise if the event in consideration caused hcv or scv
            int t_start = (int) (rnd->next()*45); // try moves of type 1
            int t_orig = c->sln[eventList[i]].first;
            for (int h = 0, t = t_start; h < 45; t = (t + 1) % 45, h++) {
                if (timer.elapsedTime(Timer::VIRTUAL) > timeLimit || ts_iter > max_step) break;
                if (rnd->next() < control.getTS_N1Prob()) {
                    nbh_sol->copy(c);
                    nbh_sol->Move1(eventList[i], t);
                    int neighbourAffectedHcv = nbh_sol->eventAffectedHcv(eventList[i]) + nbh_sol->affectedRoomInTimeslotHcv(t_orig);
                    int currentAffectedHcv = c->eventAffectedHcv(eventList[i]) + c->affectedRoomInTimeslotHcv(t);
                    int delta1Hcv = neighbourAffectedHcv - currentAffectedHcv;
                    int newHcv = c->hcv + delta1Hcv;
                    if (newHcv < bestHcv) {
                        c->copy(nbh_sol);
                        c->hcv = newHcv;
                        // set tabu
                        tabuList[i] = ts_iter;
                        bestHcv = newHcv;
                        if (c->hcv == 0) c->computeScv();
                        evCount = 0;
                        ts_iter++;
                        foundbetter = true;
                        // reset
                        bestMove[0] = -1;
                        bestMove[1] = 9999;
                        bestMove[2] = 9999;
                        break;
                    } else if (newHcv == 0) {
                        int newScv;
                        if (c->hcv == 0) {
                            int neighbourScv = nbh_sol->eventScv(eventList[i]) + // respectively Scv involving event e
                                    c->singleClassesScv(eventList[i]) - // + single classes introduced in day of original timeslot
                                    nbh_sol->singleClassesScv(eventList[i]); // - single classes "solved" in new day
                            int delta1Scv = neighbourScv - c->eventScv(eventList[i]);
                            newScv = c->scv + delta1Scv;
                        } else {
                            nbh_sol->computeScv();
                            newScv = nbh_sol->scv;
                        }
                        if (newScv < bestScv) {
                            c->copy(nbh_sol);
                            c->hcv = newHcv;
                            c->scv = newScv;
                            // set tabu
                            tabuList[i] = ts_iter;
                            bestScv = newScv;
                            evCount = 0;
                            ts_iter++;
                            foundbetter = true;
                            // reset
                            bestMove[0] = -1;
                            bestMove[1] = 9999;
                            bestMove[2] = 9999;
                            break;
                        } else if ((tabuList[i] + ts_size) <= ts_iter && newScv < bestMove[2]) {//memorize the best found non improving neighbouring solution
                            best_nbh_sol->copy(nbh_sol);
                            bestMove[0] = i;
                            bestMove[1] = 0;
                            bestMove[2] = newScv;
                        }
                    } else if ((tabuList[i] + ts_size) <= ts_iter && newHcv < bestMove[1]) {
                        best_nbh_sol->copy(nbh_sol);
                        bestMove[0] = i;
                        bestMove[1] = newHcv;
                        bestMove[2] = 9999;
                    }
                }
            }
            if (foundbetter) {
                best_sol->copy(c);
                foundbetter = false;
                continue;
            }
            for (int j = (i + 1) % pb->n_of_events; j != i; j = (j + 1) % pb->n_of_events) { // try moves of type 2
                if (timer.elapsedTime(Timer::VIRTUAL) > timeLimit || ts_iter > max_step) break;
                if (rnd->next() < control.getTS_N2Prob()) {
                    nbh_sol->copy(c);
                    nbh_sol->Move2(eventList[i], eventList[j]);
                    int newHcv;
                    if (c->hcv == 0) {
                        newHcv = nbh_sol->eventAffectedHcv(eventList[i]) +
                                nbh_sol->eventAffectedHcv(eventList[j]);
                    } else {
                        int neighbourAffectedHcv = nbh_sol->eventAffectedHcv(eventList[i]) +
                                nbh_sol->eventAffectedHcv(eventList[j]);
                        int currentAffectedHcv = c->eventAffectedHcv(eventList[i]) + c->eventAffectedHcv(eventList[j]);
                        int delta2Hcv = neighbourAffectedHcv - currentAffectedHcv;
                        newHcv = c->hcv + delta2Hcv;
                    }


                    if (newHcv < bestHcv) {
                        c->copy(nbh_sol);
                        c->hcv = newHcv;
                        // set tabu
                        tabuList[i] = ts_iter;
                        bestHcv = newHcv;
                        if (c->hcv == 0) c->computeScv();
                        evCount = 0;
                        ts_iter++;
                        foundbetter = true;
                        // reset
                        bestMove[0] = -1;
                        bestMove[1] = 9999;
                        bestMove[2] = 9999;
                        break;
                    } else if (newHcv == 0) {// only if no hcv are introduced by the move
                        int newScv;
                        if (c->hcv == 0) {
                            // compute alterations on scv for neighbour solution
                            int neighbourScv = nbh_sol->eventScv(eventList[i]) +
                                    c->singleClassesScv(eventList[i]) -
                                    nbh_sol->singleClassesScv(eventList[i]) +
                                    nbh_sol->eventScv(eventList[j]) +
                                    c->singleClassesScv(eventList[j]) -
                                    nbh_sol->singleClassesScv(eventList[j]);

                            int delta2Scv = neighbourScv - (c->eventScv(eventList[i]) + c->eventScv(eventList[j]));
                            newScv = c->scv + delta2Scv;
                        } else {
                            nbh_sol->computeScv();
                            newScv = nbh_sol->scv;
                        }
                        if (newScv < bestScv) {
                            c->copy(nbh_sol);
                            c->hcv = newHcv;
                            c->scv = newScv;
                            // set tabu
                            tabuList[i] = ts_iter;
                            bestScv = newScv;
                            evCount = 0;
                            ts_iter++;
                            foundbetter = true;
                            // reset
                            bestMove[0] = -1;
                            bestMove[1] = 9999;
                            bestMove[2] = 9999;
                            break;
                        } else if ((tabuList[i] + ts_size) <= ts_iter && newScv < bestMove[2]) {//memorize the best found non improving neighbouring solution
                            best_nbh_sol->copy(nbh_sol);
                            bestMove[0] = i;
                            bestMove[1] = 0;
                            bestMove[2] = newScv;
                        }
                    } else if ((tabuList[i] + ts_size) <= ts_iter && newHcv < bestMove[1]) {
                        best_nbh_sol->copy(nbh_sol);
                        bestMove[0] = i;
                        bestMove[1] = newHcv;
                        bestMove[2] = 9999;
                    }
                }
            }
            if (foundbetter) {
                best_sol->copy(c);
                foundbetter = false;
                continue;
            }//end if
            evCount++;
        }
        // if bestmove not empty
        if (bestMove[0] != -1) {
            c->copy(best_nbh_sol);
            c->hcv = bestMove[1];
            c->scv = bestMove[2];
            bestMove[0] = ts_iter;
        } else if (timer.elapsedTime(Timer::VIRTUAL) < timeLimit && ts_iter < max_step) {
            c->randomMove();
            c->computeHcv();
            if (c->hcv == 0) {
                c->computeScv();
            }
        }
    }
    c->copy(best_sol);
    //cout << "TS:" << ts_iter << "\t/" << timer.elapsedTime(Timer::VIRTUAL) << "\t";
    delete nbh_sol;
    delete best_nbh_sol;
    delete best_sol;
    delete []tabuList;
}

void tabuSearchMO(VectorSolution &archiveSet, int archSize, Control &control, Problem* pb) {
    Timer timer;
    VectorSolution temp;
    Solution *nbh_sol = new Solution(pb, rnd),
            *best_sol = new Solution(pb, rnd),
            *best_nbh_sol = new Solution(pb, rnd);
    int eventList[pb->n_of_events]; // keep a list of events to go through
    int tabuList[archSize][pb->n_of_events];
    int ts_iter = 0;
    double limitTime = control.getTimeLimit2();
    const int ts_size = (int) (control.alfa * (double) pb->n_of_events);
    for (int i = 0; i < pb->n_of_events; i++) {
        for (int k = 0; k < archSize; k++) {
            tabuList[k][i] = -ts_size; //initialize tabu list
        }
        eventList[i] = i;
    }
    for (int i = 0; i < pb->n_of_events; i++) { // scramble the list of events to obtain a random order
        int j = (int) (rnd->next() * pb->n_of_events);
        int h = eventList[i];
        eventList[i] = eventList[j];
        eventList[j] = h;
    }
    int bestMove[3];
    int foundbetter = false;
    int i = -1; // current event pointer
    while (timer.elapsedTime(Timer::VIRTUAL) < limitTime) {
        int evCount = 0; // counter of events considered
        // reset
        bestMove[0] = -1;
        bestMove[1] = 9999;
        bestMove[2] = 9999;
        int bestHcv = 9999; //set equal to the hcv of the first found solution
        int bestScv = 9999;
        ts_iter++;
        for (int k = 0; k < (int) archiveSet.size(); ++k) {
            temp.push_back(new Solution(pb, rnd));
            temp[k]->copy(archiveSet[k]);
            while (evCount < pb->n_of_events && timer.elapsedTime(Timer::VIRTUAL) < limitTime) {
                i = (i + 1) % pb->n_of_events; //next event
                int currentHcv = temp[k]->eventHcv(eventList[i]);
                int currentScv = temp[k]->eventScv(eventList[i]);
                if ((currentHcv == 0 && temp[k]->hcv != 0) || (temp[k]->hcv == 0 && currentScv == 0)) { // if the event on the list does not cause any hcv
                    evCount++; // increase the counter
                    continue; // go to the next event
                }//end if

                // otherwise if the event in consideration caused hcv or scv
                int t_start = (int) (rnd->next()*45); // try moves of type 1
                int t_orig = temp[k]->sln[eventList[i]].first;
                for (int h = 0, t = t_start; h < 45; t = (t + 1) % 45, h++) {
                    if (timer.elapsedTime(Timer::VIRTUAL) > limitTime) break;
                    if (rnd->next() < control.getTS_N1Prob()) {
                        nbh_sol->copy(temp[k]);
                        nbh_sol->Move1(eventList[i], t);
                        int neighbourAffectedHcv = nbh_sol->eventAffectedHcv(eventList[i]) + nbh_sol->affectedRoomInTimeslotHcv(t_orig);
                        int currentAffectedHcv = temp[k]->eventAffectedHcv(eventList[i]) + temp[k]->affectedRoomInTimeslotHcv(t);
                        int delta1Hcv = neighbourAffectedHcv - currentAffectedHcv;
                        int newHcv = temp[k]->hcv + delta1Hcv;
                        if (newHcv < bestHcv) {
                            temp[k]->copy(nbh_sol);
                            temp[k]->hcv = newHcv;
                            // set tabu
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
                            int newScv;
                            if (temp[k]->hcv == 0) {
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
                                temp[k]->copy(nbh_sol);
                                temp[k]->hcv = newHcv;
                                temp[k]->scv = newScv;
                                // set tabu
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
                            } else if ((tabuList[k][i] + ts_size) <= ts_iter && newScv < bestMove[2]) {//memorize the best found non improving neighbouring solution
                                best_nbh_sol->copy(nbh_sol);
                                bestMove[0] = i;
                                bestMove[1] = 0;
                                bestMove[2] = newScv;
                            }
                        } else if ((tabuList[k][i] + ts_size) <= ts_iter && newHcv < bestMove[1]) {
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
                for (int j = (i + 1) % pb->n_of_events; j != i; j = (j + 1) % pb->n_of_events) { // try moves of type 2
                    if (timer.elapsedTime(Timer::VIRTUAL) > limitTime) break;
                    if (rnd->next() < control.getTS_N2Prob()) {
                        nbh_sol->copy(temp[k]);
                        nbh_sol->Move2(eventList[i], eventList[j]);
                        int newHcv;
                        if (temp[k]->hcv == 0) {
                            newHcv = nbh_sol->eventAffectedHcv(eventList[i]) +
                                    nbh_sol->eventAffectedHcv(eventList[j]);
                        } else {
                            int neighbourAffectedHcv = nbh_sol->eventAffectedHcv(eventList[i]) +
                                    nbh_sol->eventAffectedHcv(eventList[j]);
                            int currentAffectedHcv = temp[k]->eventAffectedHcv(eventList[i]) + temp[k]->eventAffectedHcv(eventList[j]);
                            int delta2Hcv = neighbourAffectedHcv - currentAffectedHcv;
                            newHcv = temp[k]->hcv + delta2Hcv;
                        }


                        if (newHcv < bestHcv) {
                            temp[k]->copy(nbh_sol);
                            temp[k]->hcv = newHcv;
                            // set tabu
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
                                temp[k]->copy(nbh_sol);
                                temp[k]->hcv = newHcv;
                                temp[k]->scv = newScv;
                                // set tabu
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
                            } else if ((tabuList[k][i] + ts_size) <= ts_iter && newScv < bestMove[2]) {//memorize the best found non improving neighbouring solution
                                best_nbh_sol->copy(nbh_sol);
                                bestMove[0] = i;
                                bestMove[1] = 0;
                                bestMove[2] = newScv;
                            }
                        } else if ((tabuList[k][i] + ts_size) <= ts_iter && newHcv < bestMove[1]) {
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
            temp[k]->copy(best_sol);
        }
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

void MOGA(Control &control) {
    ostream& os = control.getOutputStream();
    int popSize = control.getPOPSize();
    int archSize = 20;

    Problem *problem = new Problem(control.getInputStream());

    rnd = new Random((unsigned) control.getSeed());
    while (control.triesLeft()) {
        control.beginTry();
        int generation = 0;
        VectorSolution popu, front0, archiveSet;
        // Random generate solution
        for (int i = 0; i < popSize; i++) {
            popu.push_back(new Solution(problem, rnd));
            popu[i]->RandomInitialSolution();
            if (control.flag["LS1"])
                popu[i]->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());
            if (control.flag["LS2"])
                popu[i]->LS2(control.getMaxSteps(), control.getTimeLimit());
            popu[i]->computePenalty();
        }
        front0 = rankSolution(popu, problem);
        for (int i = 0; i < (int) front0.size() && i < archSize; i++) {
            archiveSet.push_back(new Solution(problem, rnd));
            archiveSet[i]->copy(front0[i]);
        }
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
                if (control.flag["LS1"])
                    child->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());
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
            }// while
            printPOP(front0, control.getOutputStream());
            control.endTry(archiveSet);
            //control.endTry(front0);
            //printSolutions(archiveSet, control.getOutputStream());
        }
        if (control.getMethod() == Control::METHOD_MOHA || control.getMethod() == Control::METHOD_MOTS) {
            os << "MOTS started!!" << endl;
            tabuSearchMO(archiveSet, archSize, control, problem);
            printSolutions(archiveSet, control.getOutputStream());
        }
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

void GA(Control &control) {
    int popSize = control.getPOPSize();
    Problem *problem = new Problem(control.getInputStream());
    rnd = new Random((unsigned) control.getSeed());
    while (control.triesLeft()) {
        control.beginTry();
        int generation = 0;
        VectorSolution pop;
        for (int i = 0; i < popSize; i++) {
            pop.push_back(new Solution(problem, rnd));
            pop[i]->RandomInitialSolution();
            if (control.flag["LS1"])
                pop[i]->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());
            if (control.flag["LS2"])
                pop[i]->LS2(control.getMaxSteps(), control.getTimeLimit());
            pop[i]->computePenalty();
        }
        sort(pop.begin(), pop.end(), compareSolution);
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
            if (control.flag["LS1"])
                child->localSearch(control.getMaxSteps(), control.getTimeLimit(), control.getProb1(), control.getProb2(), control.getProb3());
            if (control.flag["LS2"])
                child->LS2(control.getMaxSteps(), control.getTimeLimit());
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
        control.endTry(pop[0]);
        for (int i = 0; i < popSize; i++) {
            delete pop[i];
        }
    }
    delete problem;
    delete rnd;
}

int main(int argc, char** argv) {
    Control control(argc, argv);
    if (control.getMethod() / 100 == 1) MOGA(control);
    else if (control.getMethod() / 200 == 1) GA(control);
}


