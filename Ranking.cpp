/* 
 * File:   Ranking.cpp
 * Author: xeonkung
 * 
 * Created on 12 March 2014, 19:21
 */

#include "Ranking.h"

Ranking::Ranking(VectorSolution pop) {
    int popSize = pop.size();
    int dominateMe[popSize];
    vector<int> iDominate[popSize];
    vector<int> front[popSize + 1];
    int flagDominate;
    
    for(int p = 0; p < popSize; p++) {
        dominateMe[p] = 0;
    }
    for(int p = 0; p < popSize - 1; p++) {
        for(int q = p + 1; q < popSize; q++) {
            flagDominate = compareOverall(pop[p], pop[q]);
            if(flagDominate == 0){
                flagDominate = compareDominateSolution(pop[p], pop[q]);
            }
            if(flagDominate == -1){
                iDominate[p].push_back(q);
                dominateMe[q]++;
            }else if(flagDominate == 1){
                iDominate[q].push_back(p);
                dominateMe[p]++;
            }
        }
    }
    // if no dominateMe is front 0
    for(int p = 0; p < popSize; p++){
        if(dominateMe[p] == 0){
            front[0].push_back(p);
            pop[p]->rank = 0;
        }
    }
    int i = 0;
    vector<int>::iterator it1, it2;
    while (front[i].size() != 0) {
        i++;
        it1 = front[i-1].begin();
        for (it1 = front[i-1].begin(); it1 != front[i-1].end(); ++it1) {
            for (it2 = iDominate[*it1].begin(); it2 != iDominate[*it1].end(); ++it2){
                dominateMe[*it2]--;
                if(dominateMe[*it2] == 0){
                    front[i].push_back(*it2);
                    pop[*it2]->rank = i;
                }
            }
        }
    }
    
    for (int j = 0; j < i; j++) {
        Solution* x[front[j].size()];
        for(int i = 0; i < front[j].size(); i++){
            x[i] = pop[i];
        }
        Front.push_back(x);
    }
}

Ranking::Ranking(const Ranking& orig) {
}

Ranking::~Ranking() {
}

int Ranking::compareDominateSolution(Solution* a, Solution* b){
    int val1, val2, dominate1 = 0, dominate2 = 0;
    for(int i = 0; i < Solution.obj_N; i++) {
        val1 = a->objective[i];
        val2 = b->objective[i];
        if(val1 < val2){
            dominate1 = 1;
        }else if (val1 > val2){
            dominate2 = 1;
        }
    }
    if(dominate1 == dominate2){
        return 0;
    }
    if(dominate1 == 1){
        return -1;
    }
    return 1;
}

int Ranking::compareOverall(Solution* a, Solution* b){
    if(a->hcv < b->hcv){
        return -1;
    }else if(a->hcv > b->hcv){
        return 1;
    }else if(a->hcv != 0 && b->hcv != 0){
        if(a->scv < b->scv) return -1;
        else if (a->scv > b->scv) return 1;
        else return 0;
    }else{
        return 0;
    }
}

