#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "include/pectpfile.cpp"
#include "include/easylogging++.h"

_INITIALIZE_EASYLOGGINGPP

using namespace std;

int main (int argc, char* argv[]) {
	
	LOG(INFO) << "TEST";
	PECTPFile p;
	p.read("test-2.txt");
}
