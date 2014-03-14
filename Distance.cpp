/* 
 * File:   Distance.cpp
 * Author: xeonkung
 * 
 * Created on 13 March 2014, 10:09
 */

#include "Distance.h"

Distance::Distance() {
}

Distance::Distance(const Distance& orig) {
}

Distance::~Distance() {
}

void Distance::crowdingDistanceAssignment(VectorSolution vSolution) {
    int size = vSolution.size();
    if(size == 0) return;
    const double inf = numeric_limits<double>.max();
    if(size == 1) {
        vSolution[0]->distance = inf;
        return;
    }
    if(size == 2) {
        vSolution[0]->distance = inf;
        vSolution[1]->distance = inf;
        return;
    }
    for(int i = 0; i < size; i++) {
        vSolution[i]->distance = 0.0;
    }
    ObjectiveCmp objCmp;
    double min, max, dist;
    for(int i = 0; i < Solution.obj_N; i++) {
        objCmp.obj_i = i;
        sort(vSolution.begin(), vSolution.end(), objCmp);
        min = vSolution[0]->objective[i];
        max = vSolution[size - 1]->objective[i];
        
        vSolution[0]->distance = inf;
        vSolution[size - 1]->distance = inf;
        
        for(int j = 1; j < size - 1; j++){
            dist = vSolution[j + 1]->objective[i] - vSolution[j - 1]->objective[i];
            dist = dist / (max - min);
            dist += vSolution[j]->distance;
            vSolution[j]->distance = dist;
        }
    }
}
