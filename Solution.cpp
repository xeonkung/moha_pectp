#include "Solution.h"

Solution::Solution(Problem* pd, Random* rnd) {

	data = pd;
	rg = rnd;
	slnInit();
}

void Solution::slnInit() {

	pair<int,int> initPair;
	initPair.first = -1 ;
	initPair.second = -1 ;
	for (int i = 0; i < (*data).n_of_events; i++ ){
		sln.push_back( initPair ) ;
	}

}

void Solution::copy(Solution *orig)
{
	sln = orig->sln;
	data = orig->data; 
	timeslot_events = orig->timeslot_events;
	feasible = orig->feasible;
	scv = orig->scv;
        for(int i = 0; i < obj_N; i ++) {
            objective[i] = orig->objective[i];
        }
        distance = orig->distance;
        rank = orig->rank;
	hcv = orig->hcv;
	penalty = orig->penalty; 
}

void Solution::RandomInitialSolution( )
{
	// assign a random timeslot to each event
	for(int i = 0; i < data->n_of_events; i++){
		int t = (int)(rg->next() * 45);
		sln[i].first = t;
		timeslot_events[t].push_back(i);
	}
	// and assign rooms to events in each non-empty timeslot
	for(int j = 0; j < 45; j++){
		if((int)timeslot_events[j].size())
			assignRooms(j);
	}
}

bool Solution::computeFeasibility()
{ 
	for (int i = 0; i < data->n_of_events; i++) {    
		for (int j = i+1; j < data->n_of_events; j++) {
			if ((sln[i].first == sln[j].first) && (sln[i].second == sln[j].second)) { 
				feasible = false;
				return false;                                // only one class can be in each room at any timeslot
			}
			if ((data->eventCorrelations[i][j] == 1) && (sln[i].first == sln[j].first)) {
				feasible = false;
				return false;                                // two events sharing students cannot be in the same timeslot
			}
		}
		if( data->possibleRooms[i][sln[i].second]  == 0 ){
			feasible = false;
			return false;                 // each event should take place in a suitable room
		}
	}
	// if none of the previous hard constraint violations occurs the timetable is feasible
	feasible = true;
	return true;
}

int Solution::computeScv()
{
	int consecutiveClasses, classesDay;
	bool attendsTimeslot;

	scv = 0; // set soft constraint violations to zero to start with
        for(int i = 0; i < obj_N; i ++) {
            objective[i] = 0;
        }
	for(int i = 0; i < data->n_of_events; i++){ // classes should not be in the last slot of the day
		if( sln[i].first%9 == 8 ) {                    
                    scv += data->studentNumber[i];  // one penalty for each student attending such a class
                    objective[0] += data->studentNumber[i];
                }
	}

	for (int j = 0; j < data->n_of_students; j++) { // students should not have more than two classes in a row
		consecutiveClasses = 0;
		for (int i = 0; i < 45; i++) { // count consecutive classes on a day
			if ((i % 9) == 0) {
				consecutiveClasses = 0;
			}
			attendsTimeslot = false;
			for (int k = 0; k < (int)timeslot_events[i].size(); k++) {
				if (data->student_events[j][timeslot_events[i][k]] == 1) {
					attendsTimeslot = true;
					consecutiveClasses = consecutiveClasses + 1;
					if (consecutiveClasses > 2) {
						scv = scv + 1;
                                                objective[1] += 1;
					}
					break;
				}
			}
			if(!attendsTimeslot)
				consecutiveClasses = 0;
		}
	}
	for (int j = 0; j < data->n_of_students; j++) { //students should not have a single class on a day
		classesDay = 0;
		for (int d = 0; d < 5; d++) {   // for each day
			classesDay = 0;               //number of classes per day
			for(int t = 0; t < 9; t++){   // for each timeslot of the day
				for (int k = 0; k < (int)timeslot_events[9*d+t].size(); k++) { 
					if (data->student_events[j][timeslot_events[9*d+t][k]] == 1) {
						classesDay = classesDay + 1;
						break;
					}
				}
				if(classesDay > 1) // if the student is attending more than one class on that day
					break;	   // go to the next day
			}  
			if (classesDay == 1) {
				scv = scv + 1;
                                objective[2] += 1;
			}
		}
	}
	return scv;
}

int Solution::computeHcv()
{

	hcv = 0; // set hard constraint violations to zero to start with
	// and count them
	for (int i = 0; i < data->n_of_events; i++) {     
		for (int j = i+1; j < data->n_of_events; j++) {
			if ((sln[i].first == sln[j].first) && (sln[i].second == sln[j].second)) { // only one class can be in each room at any timeslot
				hcv = hcv + 1;
			}
			if ((sln[i].first == sln[j].first) && (data->eventCorrelations[i][j] == 1)) {  // two events sharing students cannot be in the same timeslot
				hcv = hcv + 1;
			}
		}
		if( data->possibleRooms[i][sln[i].second]  == 0 )  // an event should take place in a suitable room
			hcv = hcv + 1;
	}

	return hcv;
}

int Solution::computePenalty()
{
    computeFeasibility();
    penalty = computeScv();
    penalty += 1000000 * computeHcv();
    return penalty;
}

//compute hard constraint violations involving event e
int Solution::eventHcv(int e)
{
	int eHcv = 0; // set to zero hard constraint violations for event e
	int t = sln[e].first; // note the timeslot in which event e is
	for (int i = 0; i < (int)timeslot_events[t].size(); i++){
		if ((timeslot_events[t][i]!=e)){ 
			if (sln[e].second == sln[timeslot_events[t][i]].second) {
				eHcv = eHcv + 1; // adds up number of events sharing room and timeslot with the given one
				//cout << "room + timeslot in common "  <<eHcv <<" event " << i << endl;
			}   
			if(data->eventCorrelations[e][timeslot_events[t][i]] == 1) {
				eHcv = eHcv + 1;  // adds up number of incompatible( because of students in common) events in the same timeslot
				//cout << "students in common " << eHcv <<" event " << i << endl;
			}
		}
	}
	// the suitable room hard constraint is taken care of by the assignroom routine
	return eHcv;
}

//compute hard constraint violations that can be affected by moving event e from its timeslot
int Solution::eventAffectedHcv(int e)
{
	int aHcv = 0; // set to zero the affected hard constraint violations for event e
	int t = sln[e].first; // t timeslot where event e is
	for (int i = 0; i < (int)timeslot_events[t].size(); i++){
		for(int j= i+1;  j < (int)timeslot_events[t].size(); j++){
			if (sln[timeslot_events[t][i]].second == sln[timeslot_events[t][j]].second) {
				aHcv = aHcv + 1; // adds up number of room clashes in the timeslot of the given event (rooms assignement are affected by move for the whole timeslot)
				//cout << "room + timeslot in common "  <<aHcv <<" events " << timeslot_events[t][i] << " and " << timeslot_events[t][j] << endl;
			}
		}
		if(timeslot_events[t][i] != e){   
			if(data->eventCorrelations[e][timeslot_events[t][i]] == 1) {
				aHcv = aHcv + 1;  // adds up number of incompatible (because of students in common) events in the same timeslot
				// the only hcv of this type affected when e is moved are the ones involving e
				//cout << "students in common " << aHcv <<" event " << timeslot_events[t][i] << endl;
			}
		}
	}
	// the suitable room hard constraint is taken care of by the assignroom routine
	return aHcv;
}

//evaluate the "only one class can be in each room at any timeslot" hcv for all the events in the timeslot of event e, 
//excluding the ones involving e that are already taken into account in eventHcv(e)
//int Solution::affectedRoomHcv(int e)
//{
//  int t = sln[e].first;
//  int roomHcv = 0;
//  for(int i= 0;  i < (int)timeslot_events[t].size(); i++){
//    if(i != e)
//      for(int j= i+1;  j < (int)timeslot_events[t].size(); j++){
//	if(j != e)
//	  if (sln[timeslot_events[t][i]].second == sln[timeslot_events[t][j]].second)
//	    roomHcv += 1;
//      }
//  }
//  return roomHcv;
//}

// evaluate all the "only one class can be in each room at any timeslot" hcv this time for all the events in timeslot t
int Solution::affectedRoomInTimeslotHcv(int t)
{
	int roomHcv = 0;
	for(int i= 0;  i < (int)timeslot_events[t].size(); i++){
		for(int j= i+1;  j < (int)timeslot_events[t].size(); j++){
			if (sln[timeslot_events[t][i]].second == sln[timeslot_events[t][j]].second)
				roomHcv += 1;
		}
	}
	return roomHcv;
}

// evaluate the number of soft constraint violation involving event e
int Solution::eventScv(int e)
{
	int eScv = 0;
	int t = sln[e].first;
	bool foundRow; 
	int singleClasses = data->studentNumber[e]; // count each student in the event to have a single class on that day
	int otherClasses = 0;

	if( t%9 == 8) // classes should not be in the last slot of the day
		eScv += data->studentNumber[e]; 

	for(int i = 0; i < data->n_of_students; i++){ 
		if(data->student_events[i][e] == 1){ // student should not have more than two classes in a row
			if( t%9 < 8){          // check timeslots before and after the timeslot of event e
				foundRow = false;
				for(int j = 0; j < (int)timeslot_events[t+1].size(); j++){
					if( data->student_events[i][timeslot_events[t+1][j]] == 1){
						if(t%9 < 7){
							for(int k =0; k < (int)timeslot_events[t+2].size(); k++){
								if(data->student_events[i][timeslot_events[t+2][k]] == 1){
									eScv += 1;
									foundRow = true;
									break;
								}
							}
						}
						if(t%9 > 0){
							for(int k =0; k < (int)timeslot_events[t-1].size(); k++){
								if( data->student_events[i][timeslot_events[t-1][k]] == 1){
									eScv += 1;
									foundRow = true;
									break;
								}
							}
						}
					}
					if(foundRow)
						break;
				}
			}    
			if(t%9 >1){
				foundRow = false;
				for(int j = 0; j < (int)timeslot_events[t-1].size(); j++){
					for(int k =0; k < (int)timeslot_events[t-2].size(); k++){
						if( data->student_events[i][timeslot_events[t-1][j]] == 1 && data->student_events[i][timeslot_events[t-2][k]] == 1){
							eScv += 1;
							foundRow = true;
							break;
						}
					}
					if(foundRow)
						break;
				}
			}

			otherClasses = 0; // set other classes on the day to be zero for each student
			for(int s = t - (t%9); s < t-(t%9)+9; s++){ // students should not have a single class in a day
				if( s != t){
					for(int j = 0; j < (int)timeslot_events[s].size(); j++){
						if(data->student_events[i][timeslot_events[s][j]] == 1){
							otherClasses += 1;
							break;
						}
					}
					if( otherClasses > 0){ // if the student has other classe on the day
						singleClasses -= 1;  // do not count it in the number of student of event e having a single class on that day
						break;
					}
				}
			}
		}
	}
	eScv += singleClasses;

	return eScv;

}

// compute the number of single classes that event e "solves" in its timeslot
// obviously when the event is taken out of its timeslot this is also the number 
// of single classes introduced by the move in the day left by the event
int Solution::singleClassesScv(int e) 
{
	int t = sln[e].first;
	int classes, singleClasses = 0;
	for(int i = 0; i < data->n_of_students; i++){
		if(data->student_events[i][e] == 1){
			classes = 0;
			for(int s = t - (t%9); s < t - (t%9) + 9; s++){
				if(classes > 1)
					break;
				if( s != t){ // we are in the feasible region so there are not events sharing students in the same timeslot
					for(int j = 0; j < (int)timeslot_events[s].size(); j++){
						if(data->student_events[i][timeslot_events[s][j]] == 1){
							classes += 1;
							break;
						}
					}
				}
			}
			// classes = 0 means that the student under consideration has a single class in the day (for event e) but that W
			// but we are not interested in that here (it is counted in eventScv(e)) 
			if(classes == 1) 
				singleClasses +=1;
		}
	}
	return singleClasses;
}

void Solution::Move1(int e, int t)
{
	//move event e to timeslot t
	int tslot =  sln[e].first;
	sln[e].first = t;
	vector<int>::iterator i;
	for(i = timeslot_events[tslot].begin(); i !=timeslot_events[tslot].end(); i++){
		if( *i == e)
			break;
	}
	timeslot_events[tslot].erase(i); // erase event e from the original timeslot
	timeslot_events[t].push_back(e); // and place it in timeslot t
	// reorder in label order events in timeslot t
	sort(timeslot_events[t].begin(),timeslot_events[t].end());
	// reassign rooms to events in timeslot t
	assignRooms(t);
	// do the same for the original timeslot of event e if it is not empty
	if((int)timeslot_events[tslot].size() > 0)
		assignRooms(tslot);
}

void Solution::Move2(int e1, int e2)
{
	//swap timeslots between event e1 and event e2
	int t = sln[e1].first;
	sln[e1].first = sln[e2].first;
	sln[e2].first = t;
	vector<int>::iterator i;
	for(i = timeslot_events[t].begin(); i !=timeslot_events[t].end(); i++){
		if( *i == e1)
			break;
	}
	timeslot_events[t].erase(i);
	timeslot_events[t].push_back(e2);
	for(i = timeslot_events[sln[e1].first].begin(); i !=timeslot_events[sln[e1].first].end(); i++){
		if( *i == e2)
			break;
	}
	timeslot_events[sln[e1].first].erase(i);
	timeslot_events[sln[e1].first].push_back(e1);

	sort(timeslot_events[t].begin(),timeslot_events[t].end());
	sort(timeslot_events[sln[e1].first].begin(),timeslot_events[sln[e1].first].end());

	assignRooms( sln[e1].first);
	assignRooms( sln[e2].first);
}

void Solution::Move3(int e1, int e2, int e3)
{
	// permute event e1, e2, and e3 in a 3-cycle
	int t = sln[e1].first;
	sln[e1].first = sln[e2].first;
	sln[e2].first = sln[e3].first;
	sln[e3].first = t;
	vector<int>::iterator i;
	for(i = timeslot_events[t].begin(); i !=timeslot_events[t].end(); i++){
		if( *i == e1)
			break;
	}
	timeslot_events[t].erase(i);
	timeslot_events[t].push_back(e3);
	for(i = timeslot_events[sln[e1].first].begin(); i !=timeslot_events[sln[e1].first].end(); i++){
		if( *i == e2)
			break;
	}
	timeslot_events[sln[e1].first].erase(i);
	timeslot_events[sln[e1].first].push_back(e1);
	for(i = timeslot_events[sln[e2].first].begin(); i !=timeslot_events[sln[e2].first].end(); i++){
		if( *i == e3)
			break;
	}
	timeslot_events[sln[e2].first].erase(i);
	timeslot_events[sln[e2].first].push_back(e2);

	sort(timeslot_events[sln[e1].first].begin(),timeslot_events[sln[e1].first].end());
	sort(timeslot_events[sln[e2].first].begin(),timeslot_events[sln[e2].first].end());
	sort(timeslot_events[sln[e3].first].begin(),timeslot_events[sln[e3].first].end());

	assignRooms( sln[e1].first);
	assignRooms( sln[e2].first);
	assignRooms( sln[e3].first);
}

void Solution::randomMove()
{
	//pick at random a type of move: 1, 2, or 3
	int moveType, e1;
	moveType = (int)(rg->next()*3) + 1;
	e1 = (int)(rg->next()*(data->n_of_events));
	if(moveType == 1){  // perform move of type 1
		int t = (int)(rg->next()*45);
		Move1( e1, t);
		//cout<< "event " << e1 << " in timeslot " << t << endl;
	}
	else if(moveType == 2){ // perform move of type 2
		int e2 = (int)(rg->next()*(data->n_of_events));
		while(e2 == e1) // take care of not swapping one event with itself
			e2 = (int)(rg->next()*(data->n_of_events));
		Move2( e1, e2);
		// cout << "e1 "<< e1 << " e2 " << e2 << endl; 
	}
	else{ // perform move of type 3
		int e2 = (int)(rg->next()*(data->n_of_events));
		while(e2 == e1)
			e2 = (int)(rg->next()*(data->n_of_events));
		int e3 = (int)(rg->next()*(data->n_of_events));
		while(e3 == e1 || e3 == e2) // take care of having three distinct events
			e3= (int)(rg->next()*(data->n_of_events));
		//cout<<"e1 " << e1 << " e2 " << e2 << " e3 " << e3<< endl;
		Move3( e1, e2, e3);
	}
}

void Solution::localSearch(int maxSteps, double LS_limit, double prob1, double prob2, double prob3)
{
	// perform local search with given time limit and probabilities for each type of move
	timer.resetTime(); // reset time counter for the local search

	int eventList[data->n_of_events]; // keep a list of events to go through
	for(int i = 0; i < data->n_of_events; i++)
		eventList[i] = i;
	for(int i = 0; i < data->n_of_events; i++){ // scramble the list of events to obtain a random order
		int j = (int)(rg->next()*data->n_of_events);      
		int h = eventList[i];
		eventList[i] = eventList[j];
		eventList[j] = h;
	}
	/*cout <<"event list" <<endl;
	for(int i = 0 ; i< data->n_of_events; i++)
	cout<< eventList[i] << " ";
	cout << endl;*/

	int neighbourAffectedHcv = 0; // partial evaluation of neighbour solution hcv
	int neighbourScv = 0; // partial evaluation of neighbour solution scv
	int evCount = 0;     // counter of events considered
	int stepCount = 0; // set step counter to zero
	int foundbetter = false;
	computeFeasibility();
	if(!feasible ){ // if the timetable is not feasible try to solve hcv
		for( int i = 0; evCount < data->n_of_events; i = (i+1)% data->n_of_events){
			if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps )
				break;
			int currentHcv = eventHcv(eventList[i]);
			if(currentHcv == 0 ){ // if the event on the list does not cause any hcv
				evCount++; // increase the counter
				continue; // go to the next event
			}
			// otherwise if the event in consideration caused hcv
			int currentAffectedHcv;
			int t_start = (int)(rg->next()*45); // try moves of type 1
			int t_orig = sln[eventList[i]].first;
			for(int h = 0, t = t_start; h < 45; t= (t+1)%45, h++){ 
				if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
					break;
				if(rg->next() < prob1){ // with given probability
					stepCount++;
					Solution *neighbourSolution = new Solution( data, rg );
					neighbourSolution->copy( this );
					//cout<< "event " << eventList[i] << " timeslot " << t << endl;
					neighbourSolution->Move1(eventList[i],t);
					neighbourAffectedHcv = neighbourSolution->eventAffectedHcv(eventList[i]) + neighbourSolution->affectedRoomInTimeslotHcv(t_orig);
					currentAffectedHcv = eventAffectedHcv(eventList[i]) + affectedRoomInTimeslotHcv(t);
					if( neighbourAffectedHcv < currentAffectedHcv){
						copy( neighbourSolution );
						delete neighbourSolution;
						evCount = 0;
						foundbetter = true;
						break;
					}
					delete neighbourSolution;
				}
			}
			if(foundbetter){
				foundbetter = false;
				continue;
			}
			if(prob2 != 0){
				for(int j= (i+1)%data->n_of_events; j != i ;j = (j+1)%data->n_of_events){ // try moves of type 2
					if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
						break;
					if(rg->next() < prob2){ // with given probability
						stepCount++;
						Solution *neighbourSolution = new Solution( data, rg );
						neighbourSolution->copy( this );
						neighbourSolution->Move2(eventList[i],eventList[j]);
						//cout<< "event " << eventList[i] << " second event " << eventList[j] << endl;
						neighbourAffectedHcv = neighbourSolution->eventAffectedHcv(eventList[i])+neighbourSolution->eventAffectedHcv(eventList[j]);
						currentAffectedHcv = eventAffectedHcv(eventList[i]) + eventAffectedHcv(eventList[j]);
						if( neighbourAffectedHcv < currentAffectedHcv){
							copy( neighbourSolution );
							delete neighbourSolution;
							evCount = 0;
							foundbetter = true;
							break;
						}
						delete neighbourSolution;
					}
				}
				if(foundbetter){
					foundbetter = false;
					continue;
				}
			}
			if(prob3 != 0){
				for(int j= (i+1)%data->n_of_events; j != i; j = (j+1)%data->n_of_events){ // try moves of type 3
					if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
						break;
					for(int k= (j+1)%data->n_of_events; k != i ; k = (k+1)%data->n_of_events){
						if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
							break;
						if(rg->next() < prob3){ // with given probability
							stepCount++;
							currentAffectedHcv = eventAffectedHcv(eventList[i]) + eventAffectedHcv(eventList[j]) + eventAffectedHcv(eventList[k]);
							Solution *neighbourSolution = new Solution( data, rg );
							neighbourSolution->copy( this );
							neighbourSolution->Move3(eventList[i],eventList[j], eventList[k]); //try one of the to possible 3-cycle
							//cout<< "event " << eventList[i] << " second event " << eventList[j] << " third event "<< eventList[k] << endl;
							neighbourAffectedHcv = neighbourSolution->eventAffectedHcv(eventList[i])+ neighbourSolution->eventAffectedHcv(eventList[j]) 
								+ neighbourSolution->eventAffectedHcv(eventList[k]);
							if( neighbourAffectedHcv < currentAffectedHcv ){
								copy( neighbourSolution );
								delete neighbourSolution;
								evCount = 0;
								foundbetter = true;
								break;
							} 
							delete neighbourSolution;
						}
						if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
							break;
						if(rg->next() < prob3){  // with given probability
							stepCount++;
							currentAffectedHcv = eventAffectedHcv(eventList[i]) + eventAffectedHcv(eventList[k]) + eventAffectedHcv(eventList[j]);
							Solution *neighbourSolution = new Solution( data, rg );
							neighbourSolution->copy( this );
							neighbourSolution->Move3(eventList[i],eventList[k], eventList[j]); //try one of the to possible 3-cycle
							//cout<< "event " << eventList[i] << " second event " << eventList[j] << " third event "<< eventList[k] << endl;
							neighbourAffectedHcv = neighbourSolution->eventAffectedHcv(eventList[i])+ neighbourSolution->eventAffectedHcv(eventList[k]) 
								+ neighbourSolution->eventAffectedHcv(eventList[j]);
							if( neighbourAffectedHcv < currentAffectedHcv ){
								copy( neighbourSolution );
								delete neighbourSolution;
								evCount = 0;
								foundbetter = true;
								break;
							} 
							delete neighbourSolution;
						}
					}
					if(foundbetter)
						break;   
				}  
				if(foundbetter){
					foundbetter = false;
					continue;
				} 
			}
			evCount++;
		}
	}
	computeFeasibility();
	if(feasible){ // if the timetable is feasible
		evCount = 0;
		int neighbourHcv;
		for( int i = 0; evCount < data->n_of_events; i = (i+1)% data->n_of_events){ //go through the events in the list
			if(stepCount > maxSteps || timer.elapsedTime(Timer::VIRTUAL) > LS_limit)
				break;
			int currentScv = eventScv(eventList[i]);
			//cout << "event " << eventList[i] << " cost " << currentScv<<endl;
			if(currentScv == 0 ){ // if there are no scv
				evCount++; // increase counter
				continue;  //go to the next event       
			}
			// otherwise try all the possible moves
			int t_start = (int)(rg->next()*45); // try moves of type 1
			for(int h= 0, t = t_start; h < 45; t= (t+1)%45, h++){
				if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
					break;
				if(rg->next() < prob1){ // each with given propability
					stepCount++;
					Solution *neighbourSolution = new Solution( data, rg );
					neighbourSolution->copy( this );
					neighbourSolution->Move1(eventList[i],t);
					//cout<< "event " << eventList[i] << " timeslot " << t << endl;
					neighbourHcv =  neighbourSolution->eventAffectedHcv(eventList[i]); //count possible hcv introduced by move
					if(neighbourHcv == 0){ // consider the move only if no hcv are introduced
						neighbourScv = neighbourSolution->eventScv(eventList[i])  // respectively Scv involving event e 
							+ singleClassesScv(eventList[i]) // + single classes introduced in day of original timeslot
							- neighbourSolution->singleClassesScv(eventList[i]); // - single classes "solved" in new day
						//cout<< "neighbour cost " << neighbourScv<<" " << neighbourHcv<< endl;
						if( neighbourScv < currentScv){
							copy( neighbourSolution );
							delete neighbourSolution;
							evCount = 0;
							foundbetter = true;
							break;
						}
					}
					delete neighbourSolution;
				}
			}
			if(foundbetter){
				foundbetter = false;
				continue;
			}
			if(prob2 != 0){
				for(int j= (i+1)%data->n_of_events; j != i ;j = (j+1)%data->n_of_events){ //try moves of type 2
					if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
						break;
					if(rg->next() < prob2){ // with the given probability
						stepCount++;
						Solution *neighbourSolution = new Solution( data, rg );
						neighbourSolution->copy( this );
						//cout<< "event " << eventList[i] << " second event " << eventList[j] << endl;
						neighbourSolution->Move2(eventList[i],eventList[j]);
						//count possible hcv introduced with the move
						neighbourHcv = neighbourSolution->eventAffectedHcv(eventList[i]) + neighbourSolution->eventAffectedHcv(eventList[j]); 
						if( neighbourHcv == 0){ // only if no hcv are introduced by the move
							// compute alterations on scv for neighbour solution 
							neighbourScv =  neighbourSolution->eventScv(eventList[i]) + singleClassesScv(eventList[i]) - neighbourSolution->singleClassesScv(eventList[i])
								+ neighbourSolution->eventScv(eventList[j]) + singleClassesScv(eventList[j]) - neighbourSolution->singleClassesScv(eventList[j]);
							// cout<< "neighbour cost " << neighbourScv<<" " << neighbourHcv<< endl;
							if( neighbourScv < currentScv + eventScv(eventList[j])){ // if scv are reduced
								copy( neighbourSolution ); // do the move
								delete neighbourSolution;
								evCount = 0;
								foundbetter = true;
								break;
							}
						}
						delete neighbourSolution;
					}
				}
				if(foundbetter){
					foundbetter = false;
					continue;
				}
			}
			if(prob3 != 0){
				for(int j= (i+1)%data->n_of_events; j != i; j = (j+1)%data->n_of_events){ //try moves of type 3
					if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
						break;
					for(int k= (j+1)%data->n_of_events; k != i ; k = (k+1)%data->n_of_events){
						if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
							break;
						if(rg->next() < prob3){ // with given probability try one of the 2 possibles 3-cycles
							stepCount++;
							Solution *neighbourSolution = new Solution( data, rg );
							neighbourSolution->copy( this );
							neighbourSolution->Move3(eventList[i],eventList[j], eventList[k]);
							// cout<< "event " << eventList[i] << " second event " << eventList[j] << " third event "<< eventList[k] << endl;
							// compute the possible hcv introduced by the move
							neighbourHcv = neighbourSolution->eventAffectedHcv(eventList[i]) + neighbourSolution->eventAffectedHcv(eventList[j]) 
								+ neighbourSolution->eventAffectedHcv(eventList[k]);
							if(neighbourHcv == 0){ // consider the move only if hcv are not introduced 
								// compute alterations on scv for neighbour solution 
								neighbourScv = neighbourSolution->eventScv(eventList[i]) + singleClassesScv(eventList[i]) - neighbourSolution->singleClassesScv(eventList[i])
									+ neighbourSolution->eventScv(eventList[j]) + singleClassesScv(eventList[j]) - neighbourSolution->singleClassesScv(eventList[j])
									+ neighbourSolution->eventScv(eventList[k]) + singleClassesScv(eventList[k]) - neighbourSolution->singleClassesScv(eventList[k]);
								// cout<< "neighbour cost " << neighbourScv<<" " << neighbourHcv<< endl;
								if( neighbourScv < currentScv+eventScv(eventList[j])+eventScv(eventList[k])){
									copy( neighbourSolution );
									delete neighbourSolution;
									evCount = 0;
									foundbetter = true;
									break;
								}
							}
							delete neighbourSolution;
						}
						if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit || stepCount > maxSteps)
							break;
						if(rg->next() < prob3){ // with the same probability try the other possible 3-cycle for the same 3 events
							stepCount++;
							Solution *neighbourSolution = new Solution( data, rg );
							neighbourSolution->copy( this );
							neighbourSolution->Move3(eventList[i],eventList[k], eventList[j]);
							// cout<< "event " << eventList[i] << " second event " << eventList[k] << " third event "<< eventList[j] << endl;
							// compute the possible hcv introduced by the move
							neighbourHcv = neighbourSolution->eventAffectedHcv(eventList[i]) + neighbourSolution->eventAffectedHcv(eventList[k]) 
								+ neighbourSolution->eventAffectedHcv(eventList[j]);
							if(neighbourHcv == 0){ // consider the move only if hcv are not introduced 
								// compute alterations on scv for neighbour solution 
								neighbourScv = neighbourSolution->eventScv(eventList[i]) + singleClassesScv(eventList[i]) - neighbourSolution->singleClassesScv(eventList[i])
									+ neighbourSolution->eventScv(eventList[k]) + singleClassesScv(eventList[k]) - neighbourSolution->singleClassesScv(eventList[k])
									+ neighbourSolution->eventScv(eventList[j]) + singleClassesScv(eventList[j]) - neighbourSolution->singleClassesScv(eventList[j]);
								// cout<< "neighbour cost " << neighbourScv<<" " << neighbourHcv<< endl;
								if( neighbourScv < currentScv+eventScv(eventList[k])+eventScv(eventList[j])){
									copy( neighbourSolution );
									delete neighbourSolution;
									evCount = 0;
									foundbetter = true;
									break;
								}
							}
							delete neighbourSolution;
						}   
					}
					if(foundbetter)
						break;  
				}
				if(foundbetter){
					foundbetter = false;
					continue;
				}
			}
			evCount++;
		}
	}
}

// assign rooms to events in timeslot t
void Solution::assignRooms(int t)
{
	val.clear();
	dad.clear();
	vector<int> assigned; // vector keeping track for each event if it is assigned or not
	int lessBusy= 0; // room occupied by the fewest events
	int busy[data->n_of_rooms]; // number of events in a room
	int N = (int)timeslot_events[t].size(); 
	int V = N+2+data->n_of_rooms;
	// initialize the bipartite graph
	size = IntMatrixAlloc(V+1,V+1);
	flow = IntMatrixAlloc(V+1,V+1);
	for (int i = 0; i <= V; i++) {
		for (int j = 0; j <= V; j++) { 
			size[i][j] = 0;
			flow[i][j] = 0;
		}
	}
	for(int i =0; i < N; i++){ 
		size[1][i+2] = 1; 
		size[i+2][1] = -1;
		for(int j = 0; j < data->n_of_rooms; j++)
			if(data->possibleRooms[timeslot_events[t][i]][j] == 1){   
				size[i+2][N+j+2] = 1; 
				size[N+j+2][i+2] = -1;
				size[N+j+2][V] = 1;
				size[V][N+j+2] = -1;
			}
	}
	maxMatching(V); // apply the matching algorithm  
	for(int i =0; i < N; i++){ // check if there are unplaced events
		assigned.push_back(0);
		for(int j = 0; j < data->n_of_rooms; j++){
			if(flow[i+2][N+j+2] == 1){
				sln[timeslot_events[t][i]].second = j;
				// cout << "room " << j << endl;
				assigned[i] = 1;
				busy[j] =+ 1;
			}
		}
	}
	for(int i = 0; i < N; i++){ // place the unplaced events in the less busy possible rooms
		if(assigned[i] == 0){
			for(int j = 0; j < data->n_of_rooms; j++){
				if(data->possibleRooms[timeslot_events[t][i]][j] == 1){
					lessBusy = j;
					break;
				}
			}
			for(int j = 0; j < data->n_of_rooms; j++){
				if(data->possibleRooms[timeslot_events[t][i]][j] == 1){
					if(busy[j] < busy[lessBusy])
						lessBusy = j;
				}
			}
			sln[timeslot_events[t][i]].second = lessBusy;
		}
	}
	free(size); // don't forget to free the memory
	free(flow);
}

// maximum matching algorithm
void Solution::maxMatching(int V)
{

	while(networkFlow(V)){  
		int x = dad[V];
		int y = V;
		while( x != 0){
			flow[x][y] = flow[x][y] + val[V];
			flow[y][x] = - flow[x][y];
			y = x; 
			x = dad[y];
		}
	}
}

//network flow algorithm
bool Solution::networkFlow(int V)
{
	int k,t,min=0;
	int priority = 0;
	val.clear();
	dad.clear();
	for( k = 1; k <= V+1; k++){
		val.push_back( -10); // 10 unseen value
		dad.push_back( 0);
	}
	val[0] = -11;  //sentinel
	val[1] = -9; // the source node
	for( k = 1; k != 0; k = min, min = 0){
		val[k] = 10+val[k];
		//cout << "val" << k << val[k] << endl;
		if(val[k] == 0)
			return false;
		if( k == V)
			return true;
		for(t = 1; t <= V; t++){  
			//cout<< " valt" << t << " = " << val[t]<< endl;   
			if(val[t] < 0){
				//cout << "flow" << k << t << "= "<< flow[k][t]<< endl; 
				priority = - flow[k][t];
				if( size[k][t] > 0) 
					priority += size[k][t];
				if(priority > val[k]) 
					priority = val[k];
				priority = 10 - priority;
				if(size[k][t] && val[t] < - priority){  // forse qui dovrei spezzare l'if in due cosi' che non calcolo le priority quando non le uso...
					val[t] = -priority;
					dad[t] = k;
				}
				if(val[t] > val[min])
					min = t;
			}
		}
	}
	return false;
}

void Solution::crossover(Solution* parent1, Solution* parent2){

	// assign some timeslots from the first parent and some from the second
	for(int i = 0; i < data->n_of_events; i++){
		if(rg->next()<0.5)
			sln[i].first = parent1->sln[i].first;
		else
			sln[i].first = parent2->sln[i].first;

		timeslot_events[sln[i].first].push_back(i);
	}
	// and assign rooms to events in each non-empty timeslot
	for(int j = 0; j < 45; j++){
		if((int)timeslot_events[j].size())
			assignRooms(j);
	}

}

void Solution::mutation(){
	randomMove();
}

void Solution::tabuSearch(double timeLimit, double a, double prob1, double prob2)
{// perform tabu search with given time limit and probabilities for each type of move

	double alfa = a;
  	timer.resetTime(); // reset time counter for the local search
 // 	computeHcv();
 // 	computeScv();
	int bestHcv = hcv; //set equal to the hcv of the first found solution
	int bestScv = scv; // if the first found solution is not feasible scv == 99999
	

	
	//best found solution
	Solution *bestSolution = new Solution( data, rg );
	bestSolution->copy( this ); //at the moment the best found solution is the first solution
		
	//best non improving neighbour solution, there is one iff we are in a local optimum
	Solution *bestNeighbourSolution = new Solution( data, rg );
	//neighbour solution
	Solution *neighbourSolution = new Solution( data, rg );
			
	int eventList[data->n_of_events]; // keep a list of events to go through
	int * tabuList = new int [data->n_of_events]; // tabu list of events

	for(int i = 0; i < data->n_of_events; i++)

	{
		tabuList[i] = -(int)(alfa*(double)data->n_of_events); //initialize tabu list
		eventList[i] = i;	
	}
	
	for(int i = 0; i < data->n_of_events; i++)
	{ // scramble the list of events to obtain a random order
		int j = (int)(rg->next()*data->n_of_events);      
		int h = eventList[i];
		eventList[i] = eventList[j];
		eventList[j] = h;
	}
 
	int iterCount = 0; // set iteration counter to zero
		
	move bestMove; //in bestMove are stored the events that are moved according to 
				   //the best not improving neighbour solution
	
	int foundbetter = false;
	int i = -1; // current event pointer
	
	//repeat untill the time limit is reached
	while(timer.elapsedTime(Timer::VIRTUAL) < timeLimit)
	{
		int evCount = 0;     // counter of events considered
		bestMove.reset();	// best non improving move is initually set to be empty
		iterCount++;		//iteration counter
				
		//evCount is increased untill or a better solution is found or
		//we have already visited the whole neighborhood (evCount == n_of_events)
		//In both case a move is made and evCount is reset to zero
		while(evCount < data->n_of_events && timer.elapsedTime(Timer::VIRTUAL) < timeLimit)
		{
			i = (i+1)% data->n_of_events; //next event

			int currentHcv = eventHcv(eventList[i]);
			int currentScv = eventScv(eventList[i]);
			if((currentHcv == 0 && hcv !=0) || (hcv==0 && currentScv == 0))
			{ // if the event on the list does not cause any hcv
				evCount++; // increase the counter
				continue; // go to the next event
			}//end if

			// otherwise if the event in consideration caused hcv or scv
			int t_start = (int)(rg->next()*45); // try moves of type 1
			int t_orig = sln[eventList[i]].first;
			for(int h = 0, t = t_start; h < 45; t= (t+1)%45, h++)
			{
				if(timer.elapsedTime(Timer::VIRTUAL) > timeLimit)
					break;
				if(rg->next() < prob1)
				{ // with given probability			
					neighbourSolution->copy( this );
					neighbourSolution->Move1(eventList[i],t);
					int neighbourAffectedHcv = neighbourSolution->eventAffectedHcv(eventList[i]) + neighbourSolution->affectedRoomInTimeslotHcv(t_orig);
					int currentAffectedHcv = eventAffectedHcv(eventList[i]) + affectedRoomInTimeslotHcv(t);
					int delta1Hcv = neighbourAffectedHcv - currentAffectedHcv;		
					int newHcv = hcv + delta1Hcv;
					
					if(newHcv < bestHcv) //aspiration criteria
					{						
//					    cout<< "Hcv" << newHcv << endl;
						copy( neighbourSolution );
						hcv = newHcv;
						setTabu(move(i), tabuList, iterCount);
						bestHcv = hcv;
						if(hcv == 0)
						{
								computeScv();
						}
						evCount = 0;
						iterCount++;
						foundbetter = true;
						bestMove.reset();
						break;					
					}else if(newHcv == 0)
					{
							int newScv;
							if(hcv == 0)
							{
						 		int neighbourScv = neighbourSolution->eventScv(eventList[i]) +   // respectively Scv involving event e
								    singleClassesScv(eventList[i]) -  // + single classes introduced in day of original timeslot
							   		neighbourSolution->singleClassesScv(eventList[i]); // - single classes "solved" in new day
								int delta1Scv = neighbourScv - eventScv(eventList[i]);
								newScv = scv +delta1Scv;
							}else
							{
								neighbourSolution->computeScv();
								newScv = neighbourSolution->scv;
							}
							
							if(newScv < bestScv) //aspiration criteria
							{
								copy( neighbourSolution );		
								hcv = newHcv;					
								scv = newScv;
								setTabu(move(i), tabuList, iterCount);
								evCount = 0;
								iterCount++;
								foundbetter = true;
								bestMove.reset();
								break;
							}else if(!tabu(i, tabuList, alfa, iterCount) && bestMove.Scv > newScv)
							{//memorize the best found non improving neighbouring solution
								bestNeighbourSolution->copy(neighbourSolution);
								bestMove.reset(i,-1,0,newScv);
							}						
					}else if( !tabu(i, tabuList, alfa, iterCount) && bestMove.Hcv > newHcv)
					{
						bestNeighbourSolution->copy(neighbourSolution);
						bestMove.reset(i,-1,newHcv);
					}//end if					
				}//end if				
			}//end for
			

			if(foundbetter)
			{
				bestSolution->copy( this );				
				foundbetter = false;
				continue;
			}
			
			if(prob2 != 0)
			{
				for(int j= (i+1)%data->n_of_events; j != i ;j = (j+1)%data->n_of_events)
				{ // try moves of type 2
					if(timer.elapsedTime(Timer::VIRTUAL) > timeLimit)
						break;
					if(rg->next() < prob2)
					{ // with given probability
						//Solution *neighbourSolution = new Solution( data, rg );
						neighbourSolution->copy( this );
						neighbourSolution->Move2(eventList[i],eventList[j]);
						
						int newHcv;
						if(hcv == 0) //if feasible
						{
								newHcv = neighbourSolution->eventAffectedHcv(eventList[i]) +
		    									 neighbourSolution->eventAffectedHcv(eventList[j]);
		    			}else
		    			{
		    					int neighbourAffectedHcv = neighbourSolution->eventAffectedHcv(eventList[i])+
											   								   neighbourSolution->eventAffectedHcv(eventList[j]);
								int currentAffectedHcv = eventAffectedHcv(eventList[i]) + eventAffectedHcv(eventList[j]);
								int delta2Hcv = neighbourAffectedHcv - currentAffectedHcv;		
								newHcv = hcv + delta2Hcv;
						}
    					

						if(newHcv < bestHcv) //aspiration criteria						
						{
							copy( neighbourSolution );
						   // cout<< "Hcv" << newHcv << endl;
							hcv = newHcv;
							bestHcv = hcv;
							setTabu(move(i,j), tabuList, iterCount);
							if(hcv == 0)
							{
								computeScv();
							}
							evCount = 0;
							iterCount++;
							foundbetter = true;
							bestMove.reset();
							break;
						}else if(newHcv == 0)
						{// only if no hcv are introduced by the move
											
								int newScv;
								if(hcv == 0)
								{
										// compute alterations on scv for neighbour solution
									int neighbourScv =  neighbourSolution->eventScv(eventList[i]) +
														  singleClassesScv(eventList[i]) -
														  neighbourSolution->singleClassesScv(eventList[i]) +
														  neighbourSolution->eventScv(eventList[j]) +
														  singleClassesScv(eventList[j]) -
														  neighbourSolution->singleClassesScv(eventList[j]);
					
								     int delta2Scv = neighbourScv - ( eventScv(eventList[i]) + eventScv(eventList[j]) );
			  						 newScv = scv + delta2Scv;									
								}else
								{
	   								 neighbourSolution->computeScv();
									 newScv = neighbourSolution->scv;
								}

								
								if(newScv < bestScv) //aspiration criteria
								{
									copy( neighbourSolution );		
									hcv = newHcv;					
									scv = newScv;
									setTabu(move(i,j), tabuList, iterCount);
									evCount = 0;
									iterCount++;
									foundbetter = true;
									bestMove.reset();
									break;
								}else if(!tabu(move(i,j), tabuList, alfa, iterCount) && bestMove.Scv > newScv)
								{//memorize the best found non improving neighbouring solution
									bestNeighbourSolution->copy(neighbourSolution);
									bestMove.reset(i,j,0,newScv);
								}
							}else if( bestMove.Hcv > newHcv && !tabu(move(i,j), tabuList, alfa, iterCount))
							{
								bestNeighbourSolution->copy(neighbourSolution);
								bestMove.reset(i,j,newHcv);
							}//end if						
					}//end if
				}//end for
				if(foundbetter)
				{
					bestSolution->copy(this);
					foundbetter = false;
					continue;
				}//end if
			}//end if
			
			evCount++; //event counter			
			
		}//end while(2)

		
		if(!bestMove.empty())
		{
			copy( bestNeighbourSolution );
			hcv = bestMove.Hcv;
			scv = bestMove.Scv;
			setTabu(bestMove, tabuList, iterCount);
		}else if(timer.elapsedTime(Timer::VIRTUAL) < timeLimit)
		{
			randomMove();
			computeHcv();
			if(hcv == 0)
			{
				computeScv();
			}
		}
		//cout<< "Hcv" << hcv << " Scv" << scv << endl;
	
		
	}//end while(1)

    copy( bestSolution );	
    delete neighbourSolution;
    delete bestSolution;
    delete bestNeighbourSolution;
    delete []tabuList;
	
}//end tabu search
bool Solution::tabu(move m, int *tabuList,double alfa, int iterCount){
	if( tabuList[m.x] + (int)(alfa * (double)(data->n_of_events) ) - rg->next() * 0 > iterCount )
		return true;
	return false;
}

void Solution::setTabu(move m, int *tabuList, int iterCount){
	tabuList[m.x] = iterCount;
	if(m.y >= 0)
		tabuList[m.x] = iterCount;
}

void Solution::LS2(int maxSteps, double LS_limit, double prob1){    
    timer.resetTime();
    // occupied size 20%
    double occ_size = 0.2;
    int step = 0;
    vector<int> occupied, occupiedNew;
    map<int, long> timeslot_penalty;
    for(map<int, vector<int> >::iterator i = timeslot_events.begin();i != timeslot_events.end(); i++){
        if(i->second.size() != 0) {
            occupied.push_back(i->first);
        }
    }
    occ_size = occ_size * occupied.size();
    
    occupied = suffle(occupied);    
    for(int i = 0; i < (int) occ_size; i++) {
        occupiedNew.push_back(occupied[i]);
    }
    // now we get x % of occupied time slot
    for(vector<int>::iterator i = occupiedNew.begin(); i != occupiedNew.end(); i++) {
        long sum = 0;
        for(vector<int>::iterator j = timeslot_events[*i].begin(); j != timeslot_events[*i].end(); j ++) {
            sum += 1000000 * eventHcv(*j) + eventScv(*j);
        }
        timeslot_penalty[*i] = sum;
    }    
    int wt = -1;
    long bigPenalty = 0;
    for(map<int, long>::iterator i = timeslot_penalty.begin(); i != timeslot_penalty.end(); i++) {
        if (i->second > bigPenalty){
            wt = i->first;
            bigPenalty = i->second;
        }
    }
    // now we have wt
    //vector<int> eventList = suffle(timeslot_events[wt]);    
    
    map<int, int> ev_timeslot;
    Solution * neiborSolution = new Solution( data, rg );    
    while(step < maxSteps) {
        int currHcv = 0, currScv = 0;
        int neiborHcv = 0, neiborScv = 0;        
        if(timer.elapsedTime(Timer::VIRTUAL) > LS_limit) break;
        neiborSolution->copy(this);        
        for(vector<int>::iterator i = timeslot_events[wt].begin(); i != timeslot_events[wt].end(); i++) {
            step++;
            ev_timeslot[*i] = occupiedNew[(int)(rg->next() * occupiedNew.size())];
            neiborSolution->Move1(*i, ev_timeslot[*i]);
        }
        for(vector<int>::iterator i = timeslot_events[wt].begin(); i != timeslot_events[wt].end(); i++) {
            neiborHcv += neiborSolution->eventAffectedHcv(*i) + neiborSolution->affectedRoomInTimeslotHcv(ev_timeslot[*i]);
            currHcv += eventHcv(*i) + affectedRoomInTimeslotHcv(wt);
        }
        if(neiborHcv == 0) {
            for(vector<int>::iterator i = timeslot_events[wt].begin(); i != timeslot_events[wt].end(); i++) {
                neiborScv += neiborSolution->eventScv(*i) + singleClassesScv(*i);
                currScv += eventScv(*i) + neiborSolution->singleClassesScv(*i);
            }
            if (neiborScv < currScv) {
                copy(neiborSolution);
                break;
            }
        } else {
            if (neiborHcv < currHcv) {
                copy(neiborSolution);
                break;
            }
        }
    }
    delete neiborSolution;
}

vector<int> Solution::suffle(vector<int> source) {
    vector<int> a;
    int j;
    for(vector<int>::iterator i = source.begin(); i != source.end(); i++){
        j = (int)(rg->next() * a.size());
        if (j == (int)a.size()){
            a.push_back(*i);
        }else{
            a.push_back(a[j]);
            a[j] = *i;
        }
    }
    return a;
}

bool Solution::equ(Solution* a){
    for(int i = 0; i < (int)sln.size(); i++){
        if(sln[i].first != a->sln[i].first || sln[i].second != a->sln[i].second)
            return false;
    }
    return true;
}