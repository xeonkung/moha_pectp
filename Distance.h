/* 
 * File:   Distance.h
 * Author: xeonkung
 *
 * Created on 13 March 2014, 10:09
 */

#ifndef DISTANCE_H
#define	DISTANCE_H
#include "Solution.h"
#include "Ranking.h"
class Distance {
public:
    Distance();
    Distance(const Distance& orig);
    virtual ~Distance();
    void crowdingDistanceAssignment(VectorSolution vSolution);
private:

};
class ObjectiveCmp {
public:
    int obj_i = 0;
    ObjectiveCmp();
    ObjectiveCmp(int i){
        obj_i = i;
    }
    virtual ~ObjectiveCmp();
    bool operator()(Solution* i, Solution* j){
        return i->objective[obj_i] < j->objective[obj_i];
    }
};
#endif	/* DISTANCE_H */

