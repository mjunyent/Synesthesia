#pragma once
#ifndef TOBAGO_BASIC
#define TOBAGO_BASIC

#include "../init/initTOBAGO.h"
#include <cstdlib>
#include <utility>

//from [-1,1]x[-1,1] range to [0, width]x[0, height] range
glm::vec2 screen2pixel(glm::vec2 screenCoords);
//from [0, width]x[0, height] range to [-1,1]x[-1,1] range
glm::vec2 pixel2screen(glm::vec2 pixelCoords);

//from [0,1]x[0,1] range to [0, width]x[0, height] range
glm::vec2 normalized2pixel(glm::vec2 normCoords);
//from [0, width]x[0, height] range to [0,1]x[0,1] range
glm::vec2 pixel2normalized(glm::vec2 pixelCoords);

//from [0,1]x[0,1] range to [-1,1]x[-1,1] range
glm::vec2 normalized2screen(glm::vec2 normCoords);
//from [-1,1]x[-1,1] range to [0, 1]x[0, 1] range
glm::vec2 screen2normalized(glm::vec2 screenCoords);

glm::vec2 viewPortSize();
glm::vec4 viewPort();

double randValue(float min, float max);

template<typename T>
bool inRange(const T &x, const T &a, const T &b) {
	if(x <= std::max(a, b) && x >= std::min(a, b)) return true;
	return false;
}

#endif
