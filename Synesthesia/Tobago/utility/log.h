#pragma once
#ifndef TOBAGO_LOG
#define TOBAGO_LOG

#include <cstring>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;
//http://stackoverflow.com/questions/5028302/small-logger-class

enum LogLevel { ERROR, WARNING, INFO };

class Log {
public:
    Log(const char* name);

	virtual ~Log();

	ofstream& write(LogLevel ERRNO);
	void flush();

private:
	ofstream *m_out; //ostream to be more flexible.
	clock_t begin_time;
};

namespace TOBAGO {
	extern Log log;
}

#endif
