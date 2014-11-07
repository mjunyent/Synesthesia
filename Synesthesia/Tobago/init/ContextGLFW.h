#pragma once
#ifndef NO_GLFW
#ifndef TOBAGO_ContextGLFW
#define TOBAGO_ContextGLFW

#include "initTobago.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <functional>

void TobagoInitGLFW(int major, int minor);

class ContextGLFW : public Context {
public:
	ContextGLFW(int width, int height, const char* name, GLFWmonitor* monitor = NULL, ContextGLFW* shared = NULL);
 
//    void setWindowResizeCallback(void(*resizeCallback)(int, int));
    void setWindowResizeCallback(std::function<void(int,int)> resizeCallback);
//    void setWindowRefreshCallback(void(*refreshCallback)());
    void setWindowRefreshCallback(std::function<void()> refreshCallback);
//    void setWindowFrameBufferResizeCallback(void(*FBresizeCallback)(int, int));
    void setWindowFrameBufferResizeCallback(std::function<void(int, int)> FBresizeCallback);
    
//    void setKeyCallback(void(*keyCallback)(int, int, int, int));
    void setKeyCallback(std::function<void(int, int, int, int)> keyCallback);
//    void setMouseButtonCallback(void(*mouseButtonCallback)(int, int, int));
    void setMouseButtonCallback(std::function<void(int, int, int)> mouseButtonCallback);
//    void setCursorPosCallback(void(*cursorMoveCallback)(double, double));
    void setCursorPosCallback(std::function<void(double, double)> cursorMoveCallback);

	void init();
	void use();
	void stop();
	void swap();

	std::string windowName;
	GLFWmonitor* monitor;
	GLFWwindow* window;
	ContextGLFW* shared;
    
    std::function<void(int,int)> resizeCallback;
    std::function<void()> refreshCallback;
    std::function<void(int, int)> FBresizeCallback;
    std::function<void(int, int, int, int)> keyCallback;
    std::function<void(int, int, int)> mouseButtonCallback;
    std::function<void(double, double)> cursorMoveCallback;
    /*
    void(*resizeCallback)(int, int);
    void(*refreshCallback)();
    void(*FBresizeCallback)(int, int);
    
    void(*keyCallback)(int, int, int, int);
    void(*mouseButtonCallback)(int, int, int);
    void(*cursorMoveCallback)(double, double);*/
};

void glfw_framebuffersize_callback(GLFWwindow* window, int width, int height);
void glfw_windowResize_callback(GLFWwindow* window, int width, int height);
void glfw_windowRefresh_callback(GLFWwindow* window);

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_mouseButton_callback(GLFWwindow* window, int button, int action, int mods);
void glfw_cursorPos_callback(GLFWwindow* window, double xpos, double ypos);

void glfw_error_callback(int error, const char* description);

#endif
#endif