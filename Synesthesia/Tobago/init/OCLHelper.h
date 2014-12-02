//
//  initOpenCL.h
//  Synesthesia
//
#ifndef NO_OCL
#ifndef __Synesthesia__initOpenCL__
#define __Synesthesia__initOpenCL__

#include "initTobago.h"

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace TOBAGO {
    void initOCLwithCurrentOGLcontext();
    
    cl_program createCLprogramFromFile(std::string filename);
    
    cl_kernel createKernel(std::string name, cl_program program);
    
    int read_kernel_from_file(const char *filename, char **source, size_t *len);
}


#endif /* defined(__Synesthesia__initOpenCL__) */
#endif