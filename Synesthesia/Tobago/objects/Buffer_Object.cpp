#include "Buffer_Object.h"

BO::BO(GLenum target) {
	glGenBuffers(1, &id);
	glBindBuffer(target, id);
	this->target = target;
}

BO::BO(BO *bo) {
	glGenBuffers(1, &id);
	glBindBuffer(bo->target, id);
	this->target = bo->target;
	this->size = bo->size;
	this->usage = bo->usage;

	glBufferData(target, size, NULL, usage);

	copy(bo);
}

BO::~BO() {
	glDeleteBuffers(1, &id);
}

void BO::bind() {
	glBindBuffer(target, id);
}

void BO::bind(GLenum target) {
	glBindBuffer(target, id);
}

void BO::unbind() {
	glBindBuffer(target, 0);
}

void BO::data(const void* data, GLsizeiptr size, GLenum usage) {
	bind();
	this->usage = usage;
	this->size = size;
	glBufferData(target, size, data, usage);
}

void BO::subdata(const void* data, GLintptr offset, GLsizeiptr size) {
	bind();
	glBufferSubData(target, offset, size, data);
}

void BO::copy(BO *from) {
	from->bind(GL_COPY_READ_BUFFER);
	this->bind(GL_COPY_WRITE_BUFFER);

	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

	if(glGetError() != 0) Tobago.log->write(ERROR) << "BO copy error";
}

//http://www.opengl.org/wiki/GLAPI/glMapBufferRange
void* BO::map(IOType io) {
	bind();

	GLbitfield access;
	switch (io)
	{
	case BO::READ:
		access = GL_MAP_READ_BIT;
		break;
	case BO::WRITE:
		access = GL_MAP_WRITE_BIT;
		break;
	case BO::RW:
		access = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT;
		break;
	}

	return glMapBufferRange(target, 0, size, access);
}

void BO::unmap() {
	glUnmapBuffer(target);
}






void Buffer_Object::destroy()
{
	this-> ~Buffer_Object();
}

Buffer_Object::~Buffer_Object()
{
	glDeleteBuffers(1, &theID);
}