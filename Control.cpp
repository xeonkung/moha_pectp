#include "Control.h"
#include <limits.h>
#include <getopt.h>
Control::Control( int argc, char** argv ) {
    
    //edit use getopt by xeonkung
    int c;
    while(1){
        int opt_index = 0;
        static struct option long_option[] = {
            {"help", no_argument, 0, 'h'},
            {"input", required_argument, 0, 'i'},
            {"output", required_argument, 0, 'o'},
            {"gen", required_argument, 0, 'g'},
            {"seed", required_argument, 0, 's'},
            {"time", required_argument, 0, 't'},
            {"LS1", no_argument, 0, 301},
            {"LS2", no_argument, 0, 302},
            {"time2", required_argument, 0, 303},
            {"alfa", required_argument, 0, 304},
            {"pts1", required_argument, 0, 305},
            {"pts2", required_argument, 0, 306},
            {"pop", required_argument, 0, 307},
            {"pm", required_argument, 0, 308},
            {"pc", required_argument, 0, 309}
        };
        c = getopt_long(argc, argv, "hi:o:m:n:g:s:t:a:", long_option, &opt_index);
        if (c == -1) break;
        switch(c){
            case 'h':
                cout << "This is MOHA help." << endl;
                cout << " -a \t\t Method Number 101=NSGA2, 102=NSGA+MOTS, 103=MOTS." << endl;
                cout << " -g, --gen \t Generation number" << endl;
                cout << " -i, --input \t Input path." << endl;
                cout << " -m \t\t Max step number in local search." << endl;
                cout << " -n \t\t Try number" << endl;
                cout << " -o, --output \t Output path. Default is screen." << endl;
                cout << " -s, --seed \t Set random seed" << endl;
                cout << " -t, --time \t Set limit time" << endl;
                cout << " --time2 \t Set limit time 2 for hybrid" << endl;
                cout << " --LS1 \t\t Enable LS1" << endl;
                cout << " --LS2 \t\t Enable LS2" << endl;
                cout << " --alfa \t Set alfa for ts" << endl;
                cout << " --pts1 \t Set alfa for ts" << endl;
                cout << " --pts2 \t Set alfa for ts" << endl;
                cout << " --pop \t Set popsize default = 50" << endl;
                cout << " --pc \t Crossover Prob. default = 0.8" << endl;
                cout << " --pm \t Mutation Prob. default = 0.5" << endl;
                
                exit(1);
                break;
            case 'a':
                parameters["-a"] = optarg;
                break;
            case 'g':
                parameters["-g"] = optarg;
                break;
            case 'i':
                parameters["-i"] = optarg;
                break;
            case 'o':
                parameters["-o"] = optarg;
                break;
            case 'm':
                parameters["-m"] = optarg;
                break;
            case 'n':
                parameters["-n"] = optarg;
                break;
            case 's':
                parameters["-s"] = optarg;
                break;
            case 't':
                parameters["-t"] = optarg;
                break;
            case 301:
                cout << "Enable: LS1" << endl;
                flag["LS1"] = true;
                break;
            case 302:
                cout << "Enable: LS2" << endl;
                flag["LS2"] = true;
                break;
            case 303:
                parameters["-t2"] = optarg;
                break;
            case 304:
                parameters["-alfa"] = optarg;
                break;
            case 305:
                parameters["pts1"] = optarg;
                break;
            case 306:
                parameters["pts2"] = optarg;
                break;
            case 307:
                parameters["pop"] = optarg;
                break;
            case 308:
                parameters["pm"] = optarg;
                break;
            case 309:
                parameters["pc"] = optarg;
                break;
        }
    }
			
	nrTry = 0;
	
	// check for input parameter
        if( parameterExists( "-a" ) ) {
		method = getIntParameter( "-a" );
	} else {
            cerr << "Error: Not Found -a option please read help (-h)" << endl;
            exit(1);
        }
	
	if( parameterExists( "-i") ) {
		is = new ifstream( getStringParameter( "-i" ).c_str() );
	} else {
		cerr << "Error: No input file given, exiting" << endl;
		cerr << "Usage:" << endl;
		cerr << argv[ 0 ] << " -i InputFile [-o OutputFile] [-n NumberOfTries] [-s RandomSeed] [-t TimeLimit] [-p ProblemType]" << endl;
		exit(1);
	}
	
	// check for ouput parameter
	
	if( parameterExists( "-o" ) ) {
		os = new ofstream( getStringParameter( "-o" ).c_str() );
	} else {
		cerr << "Warning: No output file given, writing to stdout" << endl;
		os = &cout;
	}
	
	// check for number of tries parameter
	
	if( parameterExists( "-n" ) ) {
		maxTry = getIntParameter( "-n" );
		cout << "Max number of tries " << maxTry << endl;
	} else {
		cerr << "Warning: Number of tries is set to default (10)" << endl;
		maxTry = 10; // default number of tries
	}
        if( parameterExists( "-g" ) ) {
		limitGen = getIntParameter( "-g" );
		cout << "Max number of gens " << limitGen << endl;
	} else {
                limitGen = 100000;
		cerr << "Warning: Number of gens is set to default " << limitGen << endl;
	}
        gen = 0;
	// check for time limit parameter
	
	if( parameterExists( "-t" ) ) {
		timeLimit = getDoubleParameter( "-t" );
		cout << "Time limit " << timeLimit << endl;
	} else {
		cerr << "Warning: Time limit is set to default (90 sec)" << endl;
		timeLimit = 90; // default time limit
	}
        
        if( parameterExists( "-t2" ) ) {
		timeLimit2 = getDoubleParameter( "-t2" );
		cout << "Time limit2 " << timeLimit2 << endl;
	} else {
		timeLimit2 = 0; // default time limit
	}

        // check for problem instance type parameter for the local search

	if( parameterExists( "-p" ) ) {
		problemType = getIntParameter( "-p" );
		cout <<"Problem instance type " << problemType << endl;
	} else {
//                cerr << "Warning: The problem instance type is set by default to 1 (easy)" << endl;
		problemType = 1; // default problem type
	}

        if( parameterExists( "-alfa" ) ) {
		alfa = getDoubleParameter( "-alfa" );
		cout << "Tabu list length factor " << alfa <<endl;
	} else {
//		cerr << "Warning: The tabu list length factor is set to default 0.01" << endl;
		alfa = 0.01; // default local search probability for each move of type 1 to be performed
	}
        
        // check for maximum steps parameter for the local search
	
	if( parameterExists( "-m" ) ) {
		maxSteps = getIntParameter( "-m" );
		cout <<"Max number of steps in the local search " << maxSteps << endl;
	} else {                
		maxSteps = 100; // default max steps
                cerr << "Warning: The maximum number of steps for the local search is set by default to 100" << endl;
	}

        // check for time limit parameter for the local search
	
	if( parameterExists( "-l" ) ) {
		LS_limit = getDoubleParameter( "-l" );
		cout <<"Local search time limit " << LS_limit << endl;
	} else {
//		cerr << "Warning: The local search time limit is set to default (99999 sec)" << endl;
		LS_limit = 99999; // default local search time limit
	}

        // check for probability parameter for each move in the local search
	
	if( parameterExists( "-p1" ) ) {
		prob1 = getDoubleParameter( "-p1" );
		cout << "LS move 1 probability " << prob1 <<endl;
	} else {
//		cerr << "Warning: The local search move 1 probability is set to default 1.0" << endl;
		prob1 = 1.0; // default local search probability for each move of type 1 to be performed
	}

	if( parameterExists( "-p2" ) ) {
		prob2 = getDoubleParameter( "-p2" );
		cout <<"LS move 2 probability " << prob2 << endl;
	} else {
//		cerr << "Warning: The local search move 2 probability is set to default 1.0" << endl;
		prob2 = 1.0; // default local search probability for each move to be performed
	}

	if( parameterExists( "-p3" ) ) {
		prob3 = getDoubleParameter( "-p3" );
		cout <<"LS move 3 probability " << prob3 <<  endl;
	} else {
//		cerr << "Warning: The local search move 3 probability is set to default 1.0" << endl;
		prob3 = 1.0; // default local search probability for each move to be performed
	}

	// check for random seed
	
	if( parameterExists( "-s" ) ) {
		seed = getIntParameter( "-s" );
		srand( seed );
	} else {
		seed = time( NULL );
		cerr << "Warning: " << seed << " used as default random seed" << endl;
		srand( seed );
	}
        
        if( parameterExists( "pts1" ) ) {
		pts1 = getDoubleParameter( "pts1" );
		cout << "TS N1 probability " << pts1 <<  endl;
	} else {
//		cerr << "Warning: The local search move 3 probability is set to default 1.0" << endl;
		pts1 = 0.1; // default local search probability for each move to be performed
	}
        
        if( parameterExists( "pts2" ) ) {
		pts2 = getDoubleParameter( "pts2" );
		cout <<"TS N2 probability " << pts2 <<  endl;
	} else {
//		cerr << "Warning: The local search move 3 probability is set to default 1.0" << endl;
		pts2 = 0.1; // default local search probability for each move to be performed
	}
        if( parameterExists( "pop" ) ) {
		popSize = getIntParameter( "pop" );
		cout <<"set popsize to " << popSize <<  endl;
	} else {
//		cerr << "Warning: The local search move 3 probability is set to default 1.0" << endl;
		popSize = 50; // default local search probability for each move to be performed
	}
        if( parameterExists( "pc" ) ) {
		pc = getDoubleParameter( "pc" );
		cout <<"Crossover probability " << pc <<  endl;
	} else {
//		cerr << "Warning: The local search move 3 probability is set to default 1.0" << endl;
		pc = 0.8; // default local search probability for each move to be performed
                cerr <<"Crossover probability " << pc <<  endl;
	}
        if( parameterExists( "pm" ) ) {
		pm = getDoubleParameter( "pm" );
		cout << "Mutation probability " << pm <<  endl;
	} else {
//		cerr << "Warning: The local search move 3 probability is set to default 1.0" << endl;
		pm = 0.5; // default local search probability for each move to be performed
                cerr << "Mutation probability " << pm <<  endl;
	}
        if (method == METHOD_MOTS){
            timeLimit2 = timeLimit;
        }
}

Control::~Control() {
}

bool
Control::parameterExists( string paramName ) {
	for( map< string, string >::iterator i = parameters.begin(); i != parameters.end(); i++ ) {
		if( i-> first == paramName )
			return true;
	}
	return false;
}

int
Control::getIntParameter( string paramName ) {
	if( parameterExists( paramName ) )
		return atoi( parameters[paramName].c_str() );
	else {
		return 0;
	}
}

double
Control::getDoubleParameter( string paramName ) {
	if( parameterExists( paramName ) )
		return atof( parameters[paramName].c_str() );
	else {
		return 0;
	}
}

string
Control::getStringParameter( string paramName ) {
	if( parameterExists( paramName ) )
		return parameters[paramName];
	else {
		return 0;
	}
}

void
Control::resetTime() {
	timer.resetTime();
}

double
Control::getTime() {
	return timer.elapsedTime( Timer::VIRTUAL );
}

void
Control::beginTry() {
	srand( seed++ );
	(*os) << "begin try " << ++nrTry << endl;
	resetTime();
	feasible = false;
	bestScv = INT_MAX;
	bestEvaluation = INT_MAX;
        gen = 0;
}

void
Control::endTry( Solution *bestSolution) {
  (*os) << "begin solution " << nrTry << endl;
  (*os) << "total time: " << getTime() << " gen: " << gen << endl;
  if(bestSolution->feasible){
    (*os) << "feasible: evaluation function = " << bestSolution->scv <<endl;
    for(int i = 0; i < (*bestSolution).data->n_of_events; i++)
      (*os) << bestSolution->sln[i].first << " " ;
    (*os) << endl;
    for(int i = 0; i < (*bestSolution).data->n_of_events; i++)
      (*os) << bestSolution->sln[i].second << " " ;
    (*os) << endl;
  }
  else{
    (*os) << "unfeasible: evaluation function = " << (bestSolution->computeHcv() * 1000000) + bestSolution->computeScv() <<endl;
  }
  (*os) << "end solution " << nrTry << endl;
  (*os) << "end try " << nrTry << endl;

  // The following output might be used if the ./checksln program wants to be used to check the validity of solutions.
  // Remember that the output file has then to have the .sln extension and has to have the same name as the .tim instance file
  /*for(int i = 0; i < (*bestSolution).data->n_of_events; i++){
    (*os) << bestSolution->sln[i].first << " "<< bestSolution->sln[i].second;
    (*os) << endl;
    }*/
}

void
Control::setCurrentCost(Solution *currentSolution ) {
  //if( timeLeft() ) {
	  int currentScv = currentSolution->scv;
          gen++;
	  if( currentSolution->feasible && currentScv < bestScv ) {
	    bestScv = currentScv;
	    bestEvaluation = currentScv;
	    double time = getTime();
	    (*os) << "best " << bestScv;
            (*os) << "<";
            for (int j = 0; j < Solution::obj_N; j++){
                (*os) << currentSolution->objective[j] << ", ";
            }
            (*os) << ">";
            (*os) << " time ";
            os->flags( ios::fixed );
            (*os) << ( time < 0 ? 0.0 : time ) << " gen: " << gen << "!!" << endl;
	  }
	  else if(!currentSolution->feasible){ 
	    int currentEvaluation = (currentSolution->computeHcv() * 1000000 + currentSolution->computeScv()) ;
	    if(currentEvaluation < bestEvaluation){
	      bestEvaluation = currentEvaluation;
	      double time = getTime();
	      (*os) << "best " << bestEvaluation << " time ";
	      os->flags( ios::fixed );
	      (*os) << ( time < 0 ? 0.0 : time ) << " gen: " << gen << endl;
	    }
	  }
	  //}
}
void Control::endTry(VectorSolution vs) {
    (*os) << "begin solution " << nrTry << endl;
    (*os) << "total time: " << getTime() << " gen: " << gen << endl;
    int size = (int) vs.size();
    (*os) << "Front 0" << endl;
    for(int i = 0; i < size; i++){
        (*os) << "solution " << (i+1) << " = <";
        (*os) << vs[i]->hcv << " // ";
        for (int j = 0; j < Solution::obj_N; j++){
            (*os) << vs[i]->objective[j] << ", ";
        }
        (*os) << ">" << endl;
        for (int j = 0; j < vs[i]->data->n_of_events; j++)
            (*os) << vs[i]->sln[j].first << " ";
        (*os) << endl;
        for (int j = 0; j < vs[i]->data->n_of_events; j++)
            (*os) << vs[i]->sln[j].second << " ";
        (*os) << endl;
    }   
    (*os) << "end solution " << nrTry << endl;
    (*os) << "end try " << nrTry << endl;
}