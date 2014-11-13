#ifndef NO_GLFW
#include "ContextGLFW.h"

void TobagoInitGLFW(int major, int minor) {
	if(!glfwInit()) {
		Tobago.log->write(Log::ERROR) << "Couldn't init GLFW";
		exit(EXIT_FAILURE);
	}

	glfwSetErrorCallback(glfw_error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

ContextGLFW::ContextGLFW(int width, int height, const char* name, GLFWmonitor* monitor, ContextGLFW* shared) : Context() {	
	this->width = width;
	this->height = height;
	this->windowName = name;
	this->monitor = monitor;
	this->shared = shared;

    this->resizeCallback = NULL;
    this->refreshCallback = NULL;
    this->FBresizeCallback = NULL;

    this->keyCallback = NULL;
    this->mouseButtonCallback = NULL;
    this->cursorMoveCallback = NULL;
}

void ContextGLFW::setWindowResizeCallback(std::function<void(int,int)> resizeCallback) {
    this->resizeCallback = resizeCallback;
}

void ContextGLFW::setWindowRefreshCallback(std::function<void()> refreshCallback) {
    this->refreshCallback = refreshCallback;
}

void ContextGLFW::setWindowFrameBufferResizeCallback(std::function<void(int, int)> FBresizeCallback) {
    this->FBresizeCallback = FBresizeCallback;
}

void ContextGLFW::setKeyCallback(std::function<void(int, int, int, int)> keyCallback) {
    this->keyCallback = keyCallback;
}

void ContextGLFW::setMouseButtonCallback(std::function<void(int, int, int)> mouseButtonCallback) {
    this->mouseButtonCallback = mouseButtonCallback;
}

void ContextGLFW::setCursorPosCallback(std::function<void(double, double)> cursorMoveCallback) {
    this->cursorMoveCallback = cursorMoveCallback;
}

void ContextGLFW::init() {
	GLFWwindow* sharedWin;
	if(shared == NULL) sharedWin = NULL;
	else sharedWin = shared->window;

	window = glfwCreateWindow(width, height, windowName.c_str(), monitor, sharedWin);

	glfwSetFramebufferSizeCallback(window, glfw_framebuffersize_callback);
    glfwSetWindowSizeCallback(window, glfw_windowResize_callback);
    glfwSetWindowRefreshCallback(window, glfw_windowRefresh_callback);

    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetMouseButtonCallback(window, glfw_mouseButton_callback);
    glfwSetCursorPosCallback(window, glfw_cursorPos_callback);
//    glfwSetCursorPosCallback(window, glfw_cursorPos_callback);

	if(!window) {
		Tobago.log->write(Log::ERROR) << "GLFW could not create window " << windowName.c_str();
	}

	glfwMakeContextCurrent(window);
	enabled = true;
}

void ContextGLFW::use() {
	glfwMakeContextCurrent(window);
}

void ContextGLFW::stop() {
	if(enabled) {
		glfwSetWindowShouldClose(window, GL_TRUE);
		glfwDestroyWindow(window);
		enabled = false;
	}
}

void ContextGLFW::swap() {
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void glfw_framebuffersize_callback(GLFWwindow* window, int width, int height) {
	for(Context* c : Tobago.contexts) {
		ContextGLFW* cg;
		cg = dynamic_cast<ContextGLFW*>(c);
		if(cg != NULL) {
			if(cg->window == window) {
				cg->width = width;
				cg->height = height;
                if(cg->FBresizeCallback != NULL) cg->FBresizeCallback(width, height);
				//makecontext && glViewport(0, 0, width, height);
			}
		}
	}
}

void glfw_windowResize_callback(GLFWwindow* window, int width, int height) {
	for(Context* c : Tobago.contexts) {
		ContextGLFW* cg;
		cg = dynamic_cast<ContextGLFW*>(c);
		if(cg != NULL) {
			if(cg->window == window) {
                if(cg->resizeCallback != NULL) cg->resizeCallback(width, height);
			}
		}
	}
}

void glfw_windowRefresh_callback(GLFWwindow* window) {
    for(Context* c : Tobago.contexts) {
		ContextGLFW* cg;
		cg = dynamic_cast<ContextGLFW*>(c);
		if(cg != NULL) {
			if(cg->window == window) {
                if(cg->refreshCallback != NULL) cg->refreshCallback();
			}
		}
	}
}

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    for(Context* c : Tobago.contexts) {
		ContextGLFW* cg;
		cg = dynamic_cast<ContextGLFW*>(c);
		if(cg != NULL) {
			if(cg->window == window) {
                if(cg->keyCallback != NULL) cg->keyCallback(key, scancode, action, mods);
			}
		}
	}
}

void glfw_mouseButton_callback(GLFWwindow* window, int button, int action, int mods) {
    for(Context* c : Tobago.contexts) {
		ContextGLFW* cg;
		cg = dynamic_cast<ContextGLFW*>(c);
		if(cg != NULL) {
			if(cg->window == window) {
                if(cg->mouseButtonCallback != NULL) cg->mouseButtonCallback(button, action, mods);
			}
		}
	}
}

void glfw_cursorPos_callback(GLFWwindow* window, double xpos, double ypos) {
    for(Context* c : Tobago.contexts) {
		ContextGLFW* cg;
		cg = dynamic_cast<ContextGLFW*>(c);
		if(cg != NULL) {
			if(cg->window == window) {
                if(cg->cursorMoveCallback != NULL) cg->cursorMoveCallback(xpos, ypos);
			}
		}
	}
}

void glfw_error_callback(int error, const char* description) {
	Tobago.log->write(Log::ERROR) << "GLFW ERROR CALLBACK: " << description;
}

#endif