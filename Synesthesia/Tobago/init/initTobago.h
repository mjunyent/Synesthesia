#pragma once
#ifndef TOBAGO_INITTOBAGO
#define TOBAGO_INITTOBAGO

#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp> //GLM Libs...
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "../handlers/SoundHandler.h"
#include "../utility/log.h"

#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#ifndef NO_OCL
#include <OpenCL/cl.h>
#include <OpenCL/cl_gl_ext.h>
#endif

class Context {
public:
	bool enabled;
	int width, height;
	Context() { 
		enabled = false;
		width = -1;
		height = -1;
	};
	virtual void init() { enabled = true; };
	virtual void use() {};
	virtual void stop() {};
	virtual void swap() {};
};

class TobagoHandler {
public:
	TobagoHandler(const char* logName = "logTobago.txt");
	void init(vector<Context*> contexts);
	void init(Context* c);

	bool enabled(int id);
	void use(int id);
	void stop(int id);
	void swap(int id);

	Log *log;
	vector<Context*> contexts;

    cl_context clcontext;
    cl_device_id device_id;
    cl_command_queue command_queue;
private:
	void initContextsGlewFmod();
};

extern TobagoHandler Tobago;

#endif
