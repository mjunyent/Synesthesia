#pragma once

#include "Buffer_Object.h"
#include <cstdlib>
#include <vector>
#include <type_traits>

class VBO : public BO {
public:
	template<class T>
	VBO(std::vector<T>& data) : BO(GL_ARRAY_BUFFER) {
		dataType(&data[0]);
		elements = data.size();
		this->data(&data[0], elements*sizeof(T), GL_STATIC_DRAW);
	}

	template<class T>
	VBO(const T* data, int elements) : BO(GL_ARRAY_BUFFER) {
		dataType(data);
		this->elements = elements;
		this->data(data, elements*sizeof(T), GL_STATIC_DRAW);
	}

	template<class T>
	void dataType(const T* data) {
		sizeofelement = sizeof(T);
		//http://www.opengl.org/wiki/Vertex_Buffer_Object#Component_type
		if(std::is_same<T, int>::value) {
			type = GL_INT;
		} else if(std::is_same<T, unsigned int>::value) {
			type = GL_UNSIGNED_INT;
		} else if(std::is_same<T, short>::value) {
			type = GL_SHORT;
		} else if(std::is_same<T, unsigned short>::value) {
			type = GL_UNSIGNED_SHORT;
		} else if(std::is_same<T, float>::value) {
			type = GL_FLOAT;
		} else if(std::is_same<T, double>::value) {
			type = GL_DOUBLE;
		} else if(std::is_same<T, char>::value) {
			type = GL_BYTE;
		} else if(std::is_same<T, unsigned char>::value) {
			type = GL_UNSIGNED_BYTE;
		} else {
			Tobago.log->write(ERROR) << "VBO datatype not valid.";
		}
	}

	int elements;
	int sizeofelement;
	GLenum type;
};


class oldVBO : public Buffer_Object
{
	public :
	//El conjunto de floats que , leidos de 3 en 3 , forman los vertices.
	GLfloat *theFloats;
	//El indice del buffer.
	GLuint theIndex;
	oldVBO() {};
	oldVBO( GLfloat *theArray, GLuint theSizeof , GLuint theIx ) {};
	oldVBO( std::vector<float> theArray, GLuint theIx ) {};
	//Habilitar puntero.
	void enable( GLubyte Rn ) {};
	void draw( GLushort ) {};
	//Deshabilitar puntero.
	void disable() {};
	~oldVBO() {};

	int components;
};
