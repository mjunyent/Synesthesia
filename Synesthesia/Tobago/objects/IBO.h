#pragma once

#include "Buffer_Object.h"
#include <vector>
#include <type_traits>

class IBO : public BO {
public:
	template<class T>
	IBO(std::vector<T>& data) : BO(GL_ELEMENT_ARRAY_BUFFER) {
		this->data(&data[0], data.size()*sizeof(T), GL_STATIC_DRAW);
		elements = data.size();
		sizeofelement = sizeof(T);

		//http://www.opengl.org/wiki/Vertex_Buffer_Object#Component_type
		if(std::is_same<T, unsigned char>::value) {
			type = GL_UNSIGNED_BYTE;
		} else if(std::is_same<T, unsigned short>::value) {
			type = GL_UNSIGNED_SHORT;
		} else if(std::is_same<T, unsigned int>::value) {
			type = GL_UNSIGNED_INT;
		} else {
			Tobago.log->write(ERROR) << "IBO datatype not valid.";
		}
	}

	template<class T>
	IBO(const T* data, int elements) : BO(GL_ELEMENT_ARRAY_BUFFER) {
		this->data(data, elements*sizeof(T), GL_STATIC_DRAW);
		this->elements = elements;
		sizeofelement = sizeof(T);

		//http://www.opengl.org/wiki/Vertex_Buffer_Object#Component_type
		if(std::is_same<T, unsigned char>::value) {
			type = GL_UNSIGNED_BYTE;
		} else if(std::is_same<T, unsigned short>::value) {
			type = GL_UNSIGNED_SHORT;
		} else if(std::is_same<T, unsigned int>::value) {
			type = GL_UNSIGNED_INT;
		} else {
			Tobago.log->write(ERROR) << "IBO datatype not valid.";
		}
	}

	int elements;
	int sizeofelement;
	GLenum type;
};

class oldIBO : public Buffer_Object
{
	public :
	//El conjunto de halfs que leidos de 3 en 3 , forman triangulos.
	GLushort *theIndices;
	oldIBO() {};
	oldIBO( GLushort *theArray , GLuint theSizeof ) {};
	oldIBO( std::vector<GLushort> theArray ) {};
	void draw( GLushort theDraw ) {};
	~oldIBO() {};
};