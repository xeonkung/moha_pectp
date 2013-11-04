#include <iostream>
#include <string>
#include <fstream>
using namespace std;

/* PECTP FILE Class */
class PECTPFile {
  private:
  	ifstream _file;
  public:
  	void read (const char*);
};
void PECTPFile::read (const char* filename) {
	string line;
	_file.open(filename);
	if (_file.is_open()) {

	}else{
		// log error
	}
}


int main (int argc, char* argv[]) {
	cout << "Hello!" << endl;
	for(int i = 0; i < argc; i++)
		cout << "argv[" << i << "] = " << argv[i] << endl; 
	return 0;
}
