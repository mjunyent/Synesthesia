#include "log.h"

using namespace std;

Log::Log(const char* name) {
	m_out = new ofstream;
	(*m_out).open(name);
	if(!(*m_out)) cerr << "Unable to open file " << name << endl;
	(*m_out) << "A2058 \t Error Log \t " << name << endl;
	begin_time = clock();
}

Log::~Log() {
	delete m_out;
}

ofstream& Log::write(LogLevel ERRNO) {
	(*m_out) << endl << float(clock()-begin_time)/CLOCKS_PER_SEC << "\t ";
	if(ERRNO == ERROR)			(*m_out) << "ERROR";
	else if (ERRNO == WARNING) 	(*m_out) << "WARNING";
	else if (ERRNO == INFO) 	(*m_out) << "INFO";
	(*m_out) << ": ";
	return (*m_out);
}

void Log::flush() {
	(*m_out).flush();
}

namespace TOBAGO {
	Log log = Log("logTobago.txt");
}