/* 
 * File:   Ranking.h
 * Author: xeonkung
 *
 * Created on 12 March 2014, 19:21
 */

#ifndef RANKING_H
#define	RANKING_H

#include "Solution.h"
#include <tr1/array>
typedef Solution** SolSet;
typedef vector<Solution*> VectorSolution;
class Ranking {
public:
    Ranking(VectorSolution);
    Ranking(const Ranking& orig);
    virtual ~Ranking();
    VectorSolution* Front;
    VectorSolution pop;
    static int compareOverall(Solution* a, Solution* b);
    static int compareDominateSolution(Solution* a, Solution* b);
};

#endif	/* RANKING_H */