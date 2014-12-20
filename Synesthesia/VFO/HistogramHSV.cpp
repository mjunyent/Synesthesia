//
//  HistogramHSV.cpp
//  Synesthesia
//
//  Created by Marc Junyent Martín on 26/11/14.
//  Copyright (c) 2014 Marc Junyent Martín. All rights reserved.
//

#include "HistogramHSV.h"

HistogramHSV::HistogramHSV(FrameGetter* p) {
    this->player = p;

    binsH = 16;
    binsS = 4;
    binsV = 4;
    numBins = binsH + binsS + binsV;

    framesize = player->getWidth()*player->getHeight();

    useOCL = false;
    if(Tobago.clcontext != NULL) {
        program = TOBAGO::createCLprogramFromFile("gpu_histogram_image.cl");
        if(!program) {
            useOCL = false;
            return;
        }

        histogram_kernel = TOBAGO::createKernel("histogram_image_rgba_unorm8", program);
        if(!histogram_kernel) {
            useOCL = false;
            return;
        }

        sum_partials_kernel = TOBAGO::createKernel("histogram_sum_partial_results_unorm8", program);
        if(!sum_partials_kernel) {
            useOCL = false;
            return;
        }

        clGetKernelWorkGroupInfo(histogram_kernel, Tobago.device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &workgroup_size_hk, NULL);
        {
            size_t  gsize[2];
            
            if (workgroup_size_hk <= 256)
            {
                gsize[0] = 16;
                gsize[1] = workgroup_size_hk / 16;
            }
            else if (workgroup_size_hk <= 1024)
            {
                gsize[0] = workgroup_size_hk / 16;
                gsize[1] = 16;
            }
            else
            {
                gsize[0] = workgroup_size_hk / 32;
                gsize[1] = 32;
            }
            
            local_work_size[0] = gsize[0];
            local_work_size[1] = gsize[1];
            
            global_work_size[0] = ((player->getWidth() + gsize[0] - 1) / gsize[0]);
            global_work_size[1] = ((player->getHeight() + gsize[1] - 1) / gsize[1]);
            
            num_groups = global_work_size[0] * global_work_size[1];
            global_work_size[0] *= gsize[0];
            global_work_size[1] *= gsize[1];
        }

        int err;
        histogram_buffer = clCreateBuffer(Tobago.clcontext, CL_MEM_WRITE_ONLY, 27*sizeof(unsigned int), NULL, &err);
        if (!histogram_buffer || err) {
            (*Tobago.log)(Log::ERROR) << "clCreateBuffer() failed: " << err;
            useOCL = false;
            return;
        }
        
        partial_histogram_buffer = clCreateBuffer(Tobago.clcontext, CL_MEM_READ_WRITE, num_groups*27*sizeof(unsigned int), NULL, &err);
        if (!partial_histogram_buffer || err) {
            (*Tobago.log)(Log::ERROR) << "clCreateBuffer() failed: " << err;
            useOCL = false;
            return;
        }

        clGetKernelWorkGroupInfo(sum_partials_kernel, Tobago.device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &workgroup_size_spk, NULL);
        if (workgroup_size_spk < 27) {
            (*Tobago.log)(Log::ERROR) << "A min. of 27 work-items in work-group is needed for histogram_sum_partial_results_unorm8 kernel:" << workgroup_size_spk;
            useOCL = false;
            return;        
        }

        partial_global_work_size[0] = 27;
        partial_local_work_size[0] = (workgroup_size_spk > 27) ? 27 : workgroup_size_spk;


        clSetKernelArg(histogram_kernel, 1, sizeof(cl_mem), &partial_histogram_buffer);
        
        clSetKernelArg(sum_partials_kernel, 0, sizeof(cl_mem), &partial_histogram_buffer);
        clSetKernelArg(sum_partials_kernel, 1, sizeof(int), &num_groups);
        clSetKernelArg(sum_partials_kernel, 2, sizeof(cl_mem), &histogram_buffer);

        (*Tobago.log)(Log::INFO) << "Histogram calculation using GPU.";
        useOCL = true;
    }
}

void HistogramHSV::iterate() {
    if(useOCL) iterateGPU();
    else iterateCPU();
}

void HistogramHSV::iterateGPU() {
    Texture* t = player->getTexture();

    cl_int err;
    cl_mem cl_image = clCreateFromGLTexture(Tobago.clcontext, CL_MEM_READ_WRITE, t->target, 0, t->id, &err);
    if(!cl_image || err != CL_SUCCESS) (*Tobago.log)(Log::ERROR) << "Failed to create OpenGL texture reference! " << err;

    glFlush();

    // Acquire shared objects
    err = clEnqueueAcquireGLObjects(Tobago.command_queue, 1, &cl_image, 0, NULL, NULL);
    if(err) (*Tobago.log)(Log::ERROR) << "CL acquireGLobjects error: " << err;

    // Enqueue OpenCL commands to operate on objects (kernels, read/write commands, etc)
    clSetKernelArg(histogram_kernel, 0, sizeof(cl_mem), &cl_image);

    err = clEnqueueNDRangeKernel(Tobago.command_queue,
                                 histogram_kernel,
                                 2, NULL,
                                 global_work_size, local_work_size,
                                 0, NULL, NULL);
    if (err) (*Tobago.log)(Log::ERROR) << "clEnqueueNDRangeKernel() failed for histogram_image_rgba_unorm8 kernel: " << err;

    err = clEnqueueNDRangeKernel(Tobago.command_queue,
                                 sum_partials_kernel,
                                 1, NULL,
                                 partial_global_work_size, partial_local_work_size,
                                 0, NULL, NULL);
    if (err)
        (*Tobago.log)(Log::ERROR) << "clEnqueueNDRangeKernel() failed for histogram_sum_partial_results_unorm8 kernel:" << err;


    //Read the reesults!
    unsigned int* gpu_histogram_results = (unsigned int *)malloc(27*sizeof(unsigned int));
    err = clEnqueueReadBuffer(Tobago.command_queue, histogram_buffer, CL_TRUE, 0, 27*sizeof(unsigned int), gpu_histogram_results, 0, NULL, NULL);
    if (err) (*Tobago.log)(Log::ERROR) << "clEnqueueReadBuffer() failed: " << err;

    // Release shared objects
    err = clEnqueueReleaseGLObjects(Tobago.command_queue, 1, &cl_image, 0, NULL, NULL);
    if(err) (*Tobago.log)(Log::ERROR) << "CL releaseGLObjects error: " << err;

    // Flush CL queue
    err = clFinish(Tobago.command_queue);
    if(err) (*Tobago.log)(Log::ERROR) << "CL flush error: " << err;


    std::vector<double> hist(24);
    rgb mean;
    
    for(int i=0; i<24; i++) hist[i] = gpu_histogram_results[i]/(double)(framesize);
    mean.r = gpu_histogram_results[24]/(double)(255*framesize);
    mean.g = gpu_histogram_results[25]/(double)(255*framesize);
    mean.b = gpu_histogram_results[26]/(double)(255*framesize);
    
    histograms.push_back(hist);
    rgbMeans.push_back(mean);
    free(gpu_histogram_results);
}


void HistogramHSV::iterateCPU() {
    unsigned char* image = player->getPixels();

    std::vector<double> hist(numBins, 0.0);
    rgb mean = {0.0, 0.0, 0.0};

    for(int i=0; i<4*framesize; i+=4) {
        hsv chsv = rgb2hsv((rgb){image[i]/255.0f, image[i+1]/255.0f, image[i+2]/255.0f});

        if(!std::isnan(chsv.h)) hist[std::min(binsH-1, (int)(chsv.h/360.0*binsH))]++;
        hist[binsH+std::min(binsS-1, (int)(chsv.s*binsS))]++;
        hist[binsH+binsS+std::min(binsV-1, (int)(chsv.v*binsV))]++;
        
        mean.r += image[i]/255.0;
        mean.g += image[i+1]/255.0;
        mean.b += image[i+2]/255.0;
    }

    for(double& d : hist) d /= framesize;
    
    mean.r /= framesize;
    mean.g /= framesize;
    mean.b /= framesize;

    histograms.push_back(hist);
    rgbMeans.push_back(mean);
}

void HistogramHSV::save() {
    std::ofstream output("tempHist.txt");

    for(std::vector<double>& h : histograms) {
        for(double& v : h) output << v << " ";
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
        
        std::vector<double> tmpHist(numBins);

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