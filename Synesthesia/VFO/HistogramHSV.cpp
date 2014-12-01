//
//  HistogramHSV.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 26/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "HistogramHSV.h"

HistogramHSV::HistogramHSV(Player* p) {
    this->player = p;
    
    binsH = 16;
    binsS = 4;
    binsV = 4;
    numBins = binsH + binsS + binsV;
    
    framesize = player->getWidth()*player->getHeight();
    initCL();
}

bool HistogramHSV::initCL() {
    // Get current CGL Context and CGL Share group
    CGLContextObj kCGLContext = CGLGetCurrentContext();

    if(!kCGLContext) {
        (*Tobago.log)(Log::ERROR) << "Could not get OpenGL context";
        return false;
    }

    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
    
    // Create CL context properties, add handle & share-group enum !
    cl_context_properties properties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)kCGLShareGroup, 0
    };
    
    // Create a context with device in the CGL share group
    context = clCreateContext(properties, 0, 0, NULL, 0, 0);

    if(!context) {
        (*Tobago.log).write(Log::ERROR) << "Could not create a context for OpenCL";
        return false;
    }




    unsigned int device_count;
    cl_device_id device_ids[16];
    size_t returned_size;

    
    int err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(device_ids), device_ids, &returned_size);
    if(err) {
        (*Tobago.log)(Log::ERROR) << "Could not retrieve compute devices for context.";
        return false;
    }
    
    device_count = returned_size / sizeof(cl_device_id);
    
    int i = 0;
    bool device_found = false;
    cl_device_type device_type;
    for(i = 0; i < device_count; i++) {
        clGetDeviceInfo(device_ids[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);
        if(device_type == CL_DEVICE_TYPE_GPU) {
            device_id = device_ids[i];
            device_found = true;
            break;
        }
    }
    
    if(!device_found) {
        (*Tobago.log)(Log::ERROR) << "Failed to locate a compute device!";
        return false;
    }
    

    // Create a command queue
    //
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    if (!command_queue) {
        (*Tobago.log)(Log::ERROR) << "Failed to locate a compute device!";
        return EXIT_FAILURE;
    }
    
    // Report the device vendor and device name
    //
    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};
    err = clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, &returned_size);
    err|= clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(device_name), device_name, &returned_size);
    if (err != CL_SUCCESS) {
        (*Tobago.log)(Log::WARNING) << "Failed to retrieve device info " << err;
    }
    
    (*Tobago.log)(Log::INFO) << "Connecting to " << vendor_name << " " << device_name << ".";




    char* source[1];
    size_t src_len[1];
    err = read_kernel_from_file("gpu_histogram_image.cl", &source[0], &src_len[0]);
    if(err) {
        (*Tobago.log)(Log::ERROR) << "read_kernel_from_file() failed. File not found";
    }
    
    program = clCreateProgramWithSource(context, 1, (const char **)source, (size_t *)src_len, &err);
    if(!program || err) (*Tobago.log)(Log::ERROR) << "clCreateProgramWithSource() failed: " << err;
    free(source[0]);
    
    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if(err != CL_SUCCESS) {
        char    buffer[2048] = "";
        (*Tobago.log)(Log::ERROR) << "clBuildProgram() failed: " << err;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        (*Tobago.log)(Log::INFO) << buffer;
    }
    
    kernel = clCreateKernel(program, "histogram_image_rgba_unorm8", &err);
    if(!kernel || err)
        (*Tobago.log)(Log::ERROR) << "clCreateKernel() failed creating kernel void histogram_image_rgba_unorm8(): " << err;

//    histogram_sum_partial_results_unorm8 = clCreateKernel(program, "histogram_sum_partial_results_unorm8", &err);
//    if(!histogram_sum_partial_results_unorm8 || err)
//        (*Tobago.log)(Log::ERROR) << "clCreateKernel() failed creating kernel void histogram_sum_partial_results_unorm8(): " << err;

    return true;
}


void HistogramHSV::iterateCL() {
    Texture* t = player->getTexture();
    int width = player->getWidth();
    int height = player->getHeight();

    cl_int err;
    cl_mem cl_image = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, t->target, 0, t->id, &err);
    if(!cl_image || err != CL_SUCCESS) (*Tobago.log)(Log::ERROR) << "Failed to create OpenGL texture reference! " << err;

    glFlush();

    // Acquire shared objects
    err = clEnqueueAcquireGLObjects(command_queue, 1, &cl_image, 0, NULL, NULL);
    if(err) (*Tobago.log)(Log::ERROR) << "CL acquireGLobjects error: " << err;

    // Enqueue OpenCL commands to operate on objects (kernels, read/write commands, etc)
    size_t workgroup_size;
    size_t global_work_size[2];
    size_t local_work_size[2];
    size_t num_groups;

    clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &workgroup_size, NULL);
    {
        size_t  gsize[2];
        
        if (workgroup_size <= 256)
        {
            gsize[0] = 16;
            gsize[1] = workgroup_size / 16;
        }
        else if (workgroup_size <= 1024)
        {
            gsize[0] = workgroup_size / 16;
            gsize[1] = 16;
        }
        else
        {
            gsize[0] = workgroup_size / 32;
            gsize[1] = 32;
        }
        
        local_work_size[0] = gsize[0];
        local_work_size[1] = gsize[1];
        
        global_work_size[0] = ((width + gsize[0] - 1) / gsize[0]);
        global_work_size[1] = ((height + gsize[1] - 1) / gsize[1]);

        num_groups = global_work_size[0] * global_work_size[1];
        global_work_size[0] *= gsize[0];
        global_work_size[1] *= gsize[1];
    }
    
    cl_mem partial_histogram_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, num_groups*27*sizeof(unsigned int), NULL, &err);
    if (!partial_histogram_buffer || err) (*Tobago.log)(Log::ERROR) << "clCreateBuffer() failed: " << err;
    
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &cl_image);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &partial_histogram_buffer);

    err = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    if (err) (*Tobago.log)(Log::ERROR) << "clEnqueueNDRangeKernel() failed for histogram_image_rgba_unorm8 kernel: " << err;






    unsigned int* gpu_histogram_results = (unsigned int*) malloc(num_groups*27*sizeof(unsigned int));

    err = clEnqueueReadBuffer(command_queue, partial_histogram_buffer, CL_TRUE, 0, num_groups*27*sizeof(unsigned int), gpu_histogram_results, 0, NULL, NULL);
    if (err) (*Tobago.log)(Log::ERROR) << "clEnqueueReadBuffer() failed: " << err;

    
    
    
    
    // Release shared objects
    err = clEnqueueReleaseGLObjects(command_queue, 1, &cl_image, 0, NULL, NULL);
    if(err) (*Tobago.log)(Log::ERROR) << "CL releaseGLObjects error: " << err;

    // Flush CL queue
    err = clFinish(command_queue);
    if(err) (*Tobago.log)(Log::ERROR) << "CL flush error: " << err;

    std::vector<int> cosa(27, 0);

    for(int l=0; l < num_groups; l++) {
        for(int k=0; k<27; k++) {
            cosa[k] += gpu_histogram_results[l*27 + k];
        }
    }
    
    free(gpu_histogram_results);
    
    clReleaseMemObject(partial_histogram_buffer);
    clReleaseMemObject(cl_image);
/*

    for(int l : cosa) {
        std::cout << l << " ";
    }
    std::cout << std::endl;

    for(int l : cosa) {
        std::cout << ((double) l)/(width*height) << " ";
    }
    std::cout << std::endl;
    std::cout << ((double) cosa[24])/(width*height*255.0f) << " " << ((double) cosa[25])/(width*height*255.0f) << " " << ((double) cosa[26])/(width*height*255.0f) << std::endl;

    int kl; std::cin >> kl;*/
}



int HistogramHSV::read_kernel_from_file(const char *filename, char **source, size_t *len) {
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


void HistogramHSV::iterate() {
    unsigned char* image = player->getPixels();

    std::vector<float> hist(numBins, 0.0);
    rgb mean = {0.0, 0.0, 0.0};

    for(int i=0; i<player->getNumChannels()*framesize; i+=player->getNumChannels()) {
        hsv chsv = rgb2hsv((rgb){image[i]/255.0f, image[i+1]/255.0f, image[i+2]/255.0f});

        if(!std::isnan(chsv.h)) hist[std::min(binsH-1, (int)(chsv.h/360.0*binsH))]++;
        hist[binsH+std::min(binsS-1, (int)(chsv.s*binsS))]++;
        hist[binsH+binsS+std::min(binsV-1, (int)(chsv.v*binsV))]++;
        
        mean.r += image[i]/255.0;
        mean.g += image[i+1]/255.0;
        mean.b += image[i+2]/255.0;
    }

    for(float& d : hist) d /= framesize;
    
    mean.r /= framesize;
    mean.g /= framesize;
    mean.b /= framesize;

    histograms.push_back(hist);
    rgbMeans.push_back(mean);
}

void HistogramHSV::save() {
    std::ofstream output("tempHist.txt");

    for(std::vector<float>& h : histograms) {
        for(float& v : h) output << v << " ";
        output << std::endl;
    }

    output.close();
    
    std::ofstream output2("tempMeans.txt");
    
    for(rgb& c : rgbMeans) {
        output2 << c.r << " " << c.g << " " << c.b << std::endl;
    }
    
    output2.close();
}

void HistogramHSV::load() {
    std::ifstream input1("tempHist.txt");
    std::string line;
    
    while(getline(input1, line)) {
        if(line=="-1") break;
        if(line=="") continue;

        std::stringstream in(line);
        
        std::vector<float> tmpHist(numBins);

        for(int i=0; i<tmpHist.size(); i++) {
            in >> tmpHist[i];
        }
        
        histograms.push_back(tmpHist);
    }
    input1.close();

    std::ifstream input2("tempMeans.txt");
    rgb t;
    while(input2 >> t.r) {
        input2 >> t.g >> t.b;
        rgbMeans.push_back(t);
    }
    input2.close();
    
    std::cout << histograms.size() << " " << rgbMeans.size() << std::endl;
}