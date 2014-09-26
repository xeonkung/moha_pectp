/* 
 * File:   Distance.cpp
 * Author: xeonkung
 * 
 * Created on 13 March 2014, 10:09
 */

#include "Distance.h"
/**
 * Calculate crowding distance in each solution
 * @param vSolution is vector of solution (population)
 */
void Distance::crowdingDistanceAssignment(VectorSolution vSolution) {
    // get vector solution size
    int size = vSolution.size();
    if(size == 0) return;
    const double inf = numeric_limits<double>::max();
    // if size is 1 or 2 set to infinity
    if(size == 1) {
        vSolution[0]->distance = inf;
        return;
    }
    if(size == 2) {
        vSolution[0]->distance = inf;
        vSolution[1]->distance = inf;
        return;
    }
    // define default distance 0.0 
    for(int i = 0; i < size; i++) {
        vSolution[i]->distance = 0.0;
    }
    if (vSolution[0]->hcv != 0) return;
    double min, max, dist;
    // for each objective
    for(int i = 0; i < Solution::obj_N; i++) {
        ObjectiveCmp objCmp(i);
        // sort by objective
        sort(vSolution.begin(), vSolution.end(), objCmp);
        min = vSolution[0]->objective[i];
        max = vSolution[size - 1]->objective[i];
        
        vSolution[0]->distance = inf;
        vSolution[size - 1]->distance = inf;
        // calculate distance
        for(int j = 1; j < size - 1; j++){
            dist = vSolution[j + 1]->objective[i] - vSolution[j - 1]->objective[i];
            dist = dist / (max - min);
            dist += vSolution[j]->distance;
            vSolution[j]->distance = dist;
        }
    }
}
Distance::Distance(){
    
}
Distance::~Distance(){
    
}
