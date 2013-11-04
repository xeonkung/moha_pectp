#include <fstream>
#include "easylogging++.h"

using namespace std;
/* Split string */
class splitstring : public string {
    vector<string> flds;
public:
    splitstring(char *s) : string(s) { };
    splitstring(string s) : string(s) { };
    vector<string>& split(char delim, int rep=0);
};

// split: receives a char delimiter; returns a vector of strings
// By default ignores repeated delimiters, unless argument rep == 1.
vector<string>& splitstring::split(char delim, int rep) {
    if (!flds.empty()) flds.clear();  // empty vector if necessary
    string work = data();
    string buf = "";
    int i = 0;
    while (i < work.length()) {
        if (work[i] != delim)
            buf += work[i];
        else if (rep == 1) {
            flds.push_back(buf);
            buf = "";
        } else if (buf.length() > 0) {
            flds.push_back(buf);
            buf = "";
        }
        i++;
    }
    if (!buf.empty())
        flds.push_back(buf);
    return flds;
}

/* PECTP FILE Class */
class PECTPFile {
  private:
  	ifstream _file;
  	int * _dimention;
  	int * _stdnEvnt, * _roomFeat, * _evntFeat, * _evntSlot, * _evntEvnt;
  	int * _readFirst (void);
  	int * _readData (int);
  	// STDN / EVNT
  	
  public:
  	enum SIZE{
  		EVNT,
  		ROOM,
  		FEAT,
  		STDN,
  		SLOT
  	};
  	void read (const char*);
};
void PECTPFile::read (const char* filename) {
	string line;	
	_file.open(filename);
	if (_file.is_open()) {
		_dimention = _readFirst();
		// read student / event
		_stdnEvnt = _readData(_dimention[STDN] * _dimention[EVNT]);
		// read room / feature
		_roomFeat = _readData(_dimention[ROOM] * _dimention[FEAT]);
		// read event / feature
		_evntFeat = _readData(_dimention[EVNT] * _dimention[FEAT]);
		// read event / timeslot
		_evntSlot = _readData(_dimention[EVNT] * _dimention[SLOT]);
		// read event / event
		_evntEvnt = _readData(_dimention[EVNT] * _dimention[EVNT]);
	}
	_file.close();
}
int * PECTPFile::_readFirst (void) {
	int * ret = new int [5];
	string line;
	if (getline(_file, line)) {
		splitstring s(line);
		vector<string> ss = s.split(' ');
		for (int k = 0; k < ss.size(); k++) {
        	ret[k] = atoi(ss[k].c_str());
        }
		if (ret[SLOT] == 0) {
			ret[SLOT] = 9;
		}
		// for (int k = 0; k < 5; k++) LOG(DEBUG) << ret[k] << endl;
		return ret;
	} else {
		LOG(ERROR) << "Can't found First Line" << endl;
		throw;
		return NULL;
	}
}
int * PECTPFile::_readData (int length) {
	int * ret = new int [length];
	int i = 0;
	string line;
	while (i < length) {
		if (getline(_file, line)) {
			ret[i] = atoi(line.c_str());
			// LOG(DEBUG) << ret[i];
			i++;
		} else {
			LOG(ERROR) << "Missing Line" << endl;
			return NULL;
		}
	}
	return ret;
}
