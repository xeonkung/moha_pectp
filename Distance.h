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
#include "ObjectiveCmp.h"
class Distance {
public:
    Distance();
    Distance(const Distance& orig);
    virtual ~Distance();
    void crowdingDistanceAssignment(VectorSolution vSolution);

};
#endif	/* DISTANCE_H */

