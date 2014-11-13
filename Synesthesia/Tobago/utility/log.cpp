#include "log.h"

using namespace std;

Log::Log(const char* name) {
    f_out = new ofstream;
	(*f_out).open(name);
	if(!(*f_out).is_open()) cerr << "Unable to open file " << name << endl;

    m_out = f_out;

	(*m_out) << "A2058 \t Error Log \t " << name << endl;
	begin_time = clock();
}

Log::~Log() {
	delete m_out;
}

void Log::setOutput(ostream *o) {
    (*m_out).flush();
    if(o == NULL) m_out = f_out;
    else m_out = o;
}

ostream& Log::write(LogLevel ERRNO) {
	(*m_out) << endl << float(clock()-begin_time)/CLOCKS_PER_SEC << "\t ";
	if(ERRNO == ERROR)			(*m_out) << "ERROR";
	else if (ERRNO == WARNING) 	(*m_out) << "WARNING";
	else if (ERRNO == INFO) 	(*m_out) << "INFO";
	(*m_out) << ": ";
	return (*m_out);
}

ostream& Log::operator()(LogLevel ERRNO) {
    (*m_out) << endl << float(clock()-begin_time)/CLOCKS_PER_SEC << "\t ";
    if(ERRNO == Log::ERROR)             (*m_out) << "ERROR";
    else if (ERRNO == Log::WARNING) 	(*m_out) << "WARNING";
    else if (ERRNO == Log::INFO)        (*m_out) << "INFO";
    else if (ERRNO == Log::DEBUG)       (*m_out) << "DEBUG";
    (*m_out) << ": ";
    return (*m_out);
}

void Log::flush() {
	(*m_out).flush();
}

namespace TOBAGO {
	Log log = Log("logTobago.txt");
}