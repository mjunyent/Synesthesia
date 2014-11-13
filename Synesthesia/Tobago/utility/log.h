#pragma once
#ifndef TOBAGO_LOG
#define TOBAGO_LOG

#include <cstring>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;
//http://stackoverflow.com/questions/5028302/small-logger-class

class Log {
public:
    enum LogLevel { ERROR, WARNING, INFO, DEBUG };
    Log(const char* name);

	virtual ~Log();
    
    void setOutput(ostream* o); //if o null, default output set.

    ostream& operator()(LogLevel ERRNO);
    ostream& write(LogLevel ERRNO); //write is the same as (), kept for compatibility.

	void flush();

private:
	ostream *m_out; //ostream to be more flexible.
    ofstream *f_out;
	clock_t begin_time;
};

namespace TOBAGO {
	extern Log log;
}

#endif
