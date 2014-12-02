//
//  initOpenCL.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 02/12/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//
#ifndef NO_OCL

#include "OCLHelper.h"

void TOBAGO::initOCLwithCurrentOGLcontext() {
    // Get current CGL Context and CGL Share group
    CGLContextObj kCGLContext = CGLGetCurrentContext();
    
    if(!kCGLContext)
        (*Tobago.log)(Log::ERROR) << "Could not get OpenGL context";
    
    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
    
    // Create CL context properties, add handle & share-group enum !
    cl_context_properties properties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)kCGLShareGroup, 0
    };
    
    // Create a context with device in the CGL share group
    Tobago.clcontext = clCreateContext(properties, 0, 0, NULL, 0, 0);
    
    if(!Tobago.clcontext) {
        (*Tobago.log).write(Log::ERROR) << "Could not create a context for OpenCL";
        Tobago.clcontext = NULL;
    }

    // Getting device
    unsigned long device_count;
    cl_device_id device_ids[16];
    size_t returned_size;

    int err = clGetContextInfo(Tobago.clcontext, CL_CONTEXT_DEVICES, sizeof(device_ids), device_ids, &returned_size);
    if(err) {
        (*Tobago.log)(Log::ERROR) << "Could not retrieve compute devices for context.";
        Tobago.clcontext = NULL;
    }

    device_count = returned_size / sizeof(cl_device_id);

    int i = 0;
    bool device_found = false;
    cl_device_type device_type;
    for(i = 0; i < device_count; i++) {
        clGetDeviceInfo(device_ids[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);
        if(device_type == CL_DEVICE_TYPE_GPU) {
            Tobago.device_id = device_ids[i];
            device_found = true;
            break;
        }
    }
    
    if(!device_found) {
        (*Tobago.log)(Log::ERROR) << "Failed to locate a compute device!";
        Tobago.clcontext = NULL;
    }
    
    // Create a command queue
    //
    Tobago.command_queue = clCreateCommandQueue(Tobago.clcontext, Tobago.device_id, 0, &err);
    if (!Tobago.command_queue) {
        (*Tobago.log)(Log::ERROR) << "Failed to locate a compute device! " << err;
        Tobago.clcontext = NULL;
    }
    
    // Report the device vendor and device name
    //
    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};
    err = clGetDeviceInfo(Tobago.device_id, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
    err|= clGetDeviceInfo(Tobago.device_id, CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
    if (err != CL_SUCCESS) {
        (*Tobago.log)(Log::WARNING) << "Failed to retrieve device info " << err;
    }
    
    (*Tobago.log)(Log::INFO) << "Connecting to " << vendor_name << " " << device_name << ".";
}



cl_program TOBAGO::createCLprogramFromFile(std::string filename) {
    cl_program program;
    char* source[1];
    size_t src_len[1];
    int err;
    
    err = read_kernel_from_file(filename.c_str(), &source[0], &src_len[0]);
    if(err) {
        (*Tobago.log)(Log::ERROR) << "read_kernel_from_file() failed, file " << filename << " not found.";
        return NULL;
    }
    
    program = clCreateProgramWithSource(Tobago.clcontext, 1, (const char**)source, (size_t *)src_len, &err);
    if(!program || err) {
        (*Tobago.log)(Log::ERROR) << "clCreateProgramWithSource() failed: " << err;
        return NULL;
    }
    free(source[0]);

    err = clBuildProgram(program, 1, &Tobago.device_id, NULL, NULL, NULL);
    if(err != CL_SUCCESS) {
        char    buffer[2048] = "";
        (*Tobago.log)(Log::ERROR) << "clBuildProgram() failed: " << err;
        clGetProgramBuildInfo(program, Tobago.device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        (*Tobago.log)(Log::INFO) << buffer;
        return NULL;
    }

    return program;
}

cl_kernel TOBAGO::createKernel(std::string name, cl_program program) {
    int err;
    cl_kernel kernel = clCreateKernel(program, name.c_str(), &err);
    if(!kernel || err) {
        (*Tobago.log)(Log::ERROR) << "clCreateKernel() failed creating kernel " << name << ": " << err;
        return NULL;
    }

    return kernel;
}

int TOBAGO::read_kernel_from_file(const char *filename, char **source, size_t *len) {
    struct stat statbuf;
    FILE        *fh;
    size_t      file_len;
    
    fh = fopen(filename, "r");
    if (fh == 0)
        return -1;
    
    stat(filename, &statbuf);
    file_len = (size_t)statbuf.st_size;
    *len = file_len;
    *source = (char *) malloc(file_len+1);
    fread(*source, file_len, 1, fh);
    (*source)[file_len] = '\0';
    
    fclose(fh);
    return 0;
}




#endif