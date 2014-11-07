#include "VAO.h"

VAO::VAO(GLenum mode) {
	glGenVertexArrays(1, &id);
	ibo = NULL;
	this->mode = mode;
	elements = -1;
}

void VAO::bind() {
	glBindVertexArray(id);
}

void VAO::unbind() {
	glBindVertexArray(0);
}

void VAO::addAttribute(GLuint index, int dimension, VBO* vbo, int stride /* = 0 */, int offset /* = 0 */, GLboolean normalized /* = GL_FALSE */) {
	bind();
	vbo->bind();
	if(elements < 0) elements = vbo->elements / dimension;
	glVertexAttribPointer(index, dimension, vbo->type, normalized, stride, (void*)(vbo->sizeofelement*offset));
	glEnableVertexAttribArray(index);
}

void VAO::addIntAttribute(GLuint index, int dimension, VBO* vbo, int stride /* = 0 */, int offset /* = 0 */) {
	bind();
	vbo->bind();
	if(elements < 0) elements = vbo->elements / dimension;
	glVertexAttribIPointer(index, dimension, vbo->type, stride, (void*)(vbo->sizeofelement*offset));
	glEnableVertexAttribArray(index);
}

void VAO::addDoubleAttribute(GLuint index, int dimension, VBO* vbo, int stride /* = 0 */, int offset /* = 0 */) {
	bind();
	vbo->bind();
	if(elements < 0) elements = vbo->elements / dimension;
	glVertexAttribLPointer(index, dimension, vbo->type, stride, (void*)(vbo->sizeofelement*offset));
	glEnableVertexAttribArray(index);
}

void VAO::addIBO(IBO *ibo) {
	this->ibo = ibo;
	bind();
	ibo->bind();
}

void VAO::instancedAttribute(GLuint index, GLuint div /* = 1 */) {
	bind();
	glVertexAttribDivisor(index, div); 
}

void VAO::enableAttribute(GLuint index) {
	bind();
	glEnableVertexAttribArray(index);
}

void VAO::disableAttribute(GLuint index) {
	bind();
	glDisableVertexAttribArray(index);
}

void VAO::setDrawArraysNumberOfElements(GLsizei elements) {
	this->elements = elements;
}

void VAO::setMultiDrawArraysNumberOfElements(vector<GLsizei>& multipleElements) {
	arraysCount = multipleElements;
	arraysFirst.push_back(0);

	for(int i=0; i<arraysCount.size()-1; i++) {
		arraysFirst.push_back(arraysFirst[i]+arraysCount[i]);
	}
}

void VAO::setMultiDrawElementsNumberOfIndices(vector<GLsizei>& multipleIndices, vector<GLsizei>& numberofElements) {
	int s;
	if(ibo->type == GL_UNSIGNED_BYTE) s = 1;
	else if(ibo->type == GL_UNSIGNED_SHORT) s = 2;
	else s = 4;

	elementsCount = multipleIndices;
	vertexsOffset = numberofElements;

	elementsOffset.push_back((GLvoid*)0);
	GLsizei accum = 0;
	for(int i=0; i<multipleIndices.size()-1; i++) {
		accum += multipleIndices[i]*s;
		elementsOffset.push_back((GLvoid*)accum);
	}
}

void VAO::draw() {
	if(ibo == NULL) drawArrays();
	else drawElements();
}

void VAO::drawArrays() {
	bind();
	glDrawArrays(mode, 0, elements);
}

void VAO::drawElements() {
	bind();
	glDrawElements(mode, ibo->elements, ibo->type, (void*)0);
}

void VAO::drawInstanced(GLsizei times) {
	if(ibo == NULL) drawArraysInstanced(times);
	else drawElementsInstanced(times);
}

void VAO::drawArraysInstanced(GLsizei times) {
	bind();
	glDrawArraysInstanced(mode, 0, elements, times);
}

void VAO::drawElementsInstanced(GLsizei times) {
	bind();
	glDrawElementsInstanced(mode, ibo->elements, ibo->type, (void*)0, times);
}

void VAO::multiDraw() {
	if(ibo == NULL) multiDrawArrays();
	else multiDrawElements();
}

void VAO::multiDrawArrays() {
	bind();
	glMultiDrawArrays(mode, &arraysFirst[0], &arraysCount[0], arraysFirst.size());
}

void VAO::multiDrawElements() {
	bind();
	glMultiDrawElementsBaseVertex(mode, &elementsCount[0], ibo->type, (GLvoid**)&elementsOffset[0], vertexsOffset.size(), &vertexsOffset[0]);
}

VAO::~VAO() {
	glDeleteVertexArrays(1, &id);
}
