#include "basic.h"

glm::vec2 screen2pixel(glm::vec2 screenCoords) {
	glm::vec2 norm = screen2normalized(screenCoords);
	norm.y = 1-norm.y;
	return normalized2pixel(norm);
}

glm::vec2 pixel2screen(glm::vec2 pixelCoords) {
	glm::vec2 norm = pixel2normalized(pixelCoords);
	norm.y = 1-norm.y;
	return normalized2screen(norm);
}

//from [0,1]x[0,1] range to [0, width]x[0, height] range
glm::vec2 normalized2pixel(glm::vec2 normCoords) {
	glm::vec2 sSize = viewPortSize();
	return normCoords*sSize;
}

//from [0, width]x[0, height] range to [0,1]x[0,1] range
glm::vec2 pixel2normalized(glm::vec2 pixelCoords) {
	glm::vec2 sSize = viewPortSize();
	return pixelCoords/sSize;
}


//from [0,1]x[0,1] range to [-1,1]x[-1,1] range
glm::vec2 normalized2screen(glm::vec2 normCoords) {
	normCoords *= 2.0;  //range [0,2]x[0,2]
	return (normCoords - glm::vec2(1,1));
}

//from [-1,1]x[-1,1] range to [0, 1]x[0, 1] range
glm::vec2 screen2normalized(glm::vec2 screenCoords) {
	screenCoords = (screenCoords + glm::vec2(1,1));
	screenCoords /= 2.0;
	return screenCoords;
}

glm::vec2 viewPortSize() {
	glm::vec4 vp = viewPort();
	return glm::vec2(vp.z-vp.x, vp.w-vp.y);
}

glm::vec4 viewPort() {
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	return glm::vec4(m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3]);
}

double randValue(float min, float max) {
	float val = (float)rand(); //0-RAND_MAX
	val /= RAND_MAX; //0, 1
	val *= (max-min); //0, (max-min)
	val += min; //min, max
	return val;
}