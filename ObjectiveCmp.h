/* 
 * File:   ObjectiveCmp.h
 * Author: xeonkung
 *
 * Created on 14 March 2014, 13:35
 */

#ifndef OBJECTIVECMP_H
#define	OBJECTIVECMP_H
#include "Solution.h"
class ObjectiveCmp {
public:
    int obj_i;
    ObjectiveCmp();
    ObjectiveCmp(int i){
        obj_i = i;
    }
    virtual ~ObjectiveCmp();
    bool operator()(Solution* i, Solution* j){
        return i->objective[obj_i] < j->objective[obj_i];
    };
private:

};

#endif	/* OBJECTIVECMP_H */

