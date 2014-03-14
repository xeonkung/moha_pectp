/* 
 * File:   Ranking.h
 * Author: xeonkung
 *
 * Created on 12 March 2014, 19:21
 */

#ifndef RANKING_H
#define	RANKING_H

#include "Solution.h"
typedef Solution** SolSet;
typedef vector<Solution*> VectorSolution;
class Ranking {
public:
    Ranking(VectorSolution pop);
    Ranking(const Ranking& orig);
    virtual ~Ranking();
    vector<SolSet> rank;
    vector<VectorSolution> Front;
private:
    SolSet _solutionSet;   
    int compareOverall(Solution* a, Solution* b);
    int compareDominateSolution(Solution* a, Solution* b);
};

#endif	/* RANKING_H */

