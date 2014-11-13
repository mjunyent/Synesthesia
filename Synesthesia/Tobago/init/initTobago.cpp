#include "initTobago.h"

TobagoHandler::TobagoHandler(const char* logName) {
	this->log = new Log(logName);
}

void TobagoHandler::init(vector<Context*> contexts) {
	this->contexts = contexts;
	initContextsGlewFmod();
}

void TobagoHandler::init(Context* c) {
	contexts.push_back(c);
	initContextsGlewFmod();
}

bool TobagoHandler::enabled(int id) {
	return contexts[id]->enabled;
}

void TobagoHandler::use(int id) {
	contexts[id]->use();
}

void TobagoHandler::stop(int id) {
	contexts[id]->stop();
}

void TobagoHandler::swap(int id) {
	contexts[id]->swap();
}

void TobagoHandler::initContextsGlewFmod() {
	//Contexts
	for(Context* c : this->contexts) {
		c->init();
	}

	//GLEW
	glewExperimental=GL_TRUE;
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		log->write(Log::ERROR) << "glewInit fail'd: " << glewGetErrorString(err);
		exit( EXIT_FAILURE );
	}

	glGetError(); //Purgue GL_INVALID_ENUM glew error, (it's ok, it's not our fault).

	//FMOD
	#ifndef NO_FMOD
	initSound();
	#endif

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

TobagoHandler Tobago = TobagoHandler("logTobago.txt");
