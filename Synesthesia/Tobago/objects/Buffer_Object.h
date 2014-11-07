#ifndef BUFFEROBJECT_H
#define BUFFEROBJECT_H

#include "../init/initTobago.h"

class BO {
public:
	enum IOType	{ READ, WRITE, RW };

	BO(GLenum target);
	BO(BO *bo);
	~BO();
	void bind();
	void bind(GLenum target);
	void unbind();
	void data(const void* data, GLsizeiptr size, GLenum usage); //GL_a_b: a = { STATIC, DYNAMIC, STREAM }, b = {DRAW, READ, COPY}
	void subdata(const void* data, GLintptr offset, GLsizeiptr size);
	void copy(BO *from);
	void* map(IOType io);
	void unmap();
//	void clear(); OGL 4.3 http://www.opengl.org/wiki/Buffer_Object#Clearing
//	void invalidate(); OGL 4.3

	GLuint id;
	GLenum target;
	GLenum usage;
	GLintptr size;
};

class Buffer_Object
{
	public :
	//Tipo del buffer , se llena con el adecuado en funcion de que heredada se llame.
	GLuint BUFFER;
	//Tipo de variable , idem.
	GLuint TYPE  ;
	//Los bytes que ocupa el array "nucleo" del objeto , es necesario guardarlo.
	GLuint theBytes;
	//La ID generada del buffer.
	GLuint theID;
	//Destruir el objeto.
	void destroy();
	//Destructor propiamente dicho , no hay por que llamarlo.
	~Buffer_Object();
};
#endif
