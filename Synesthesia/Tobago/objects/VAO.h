#ifndef VAO_H
#define VAO_H

#include "../init/initTobago.h"
#include "VBO.h"
#include "IBO.h"

class VAO
{
public:
	/* Create VAO
	 * GLenum mode: drawing mode, GL_LINES, GL_TRIANGLES, GL_QUADS, ......
	 */
	VAO(GLenum mode);
	~VAO(); //TODO VAO DESTRUCTOR DON'T DELETE VBOS! (or is it?)

	//Bind VAO to state
	void bind();
	//Bind VAO 0 to state
	void unbind();

	/* Add a float attribute.
	 * index: layout id of attribute
	 * dimension: number of elements per vertex (1,2,3,4)
	 * vbo: pointer to the vbo we want to bind.
	 * stride: separation between elements.
	 * offset: offset of first element in number of elements (not vertex or bytes).
	 * normalize: normalize or not the elements.
	 */
	void addAttribute(GLuint index, int dimension, VBO* vbo, int stride = 0, int offset = 0, GLboolean normalized = GL_FALSE);
	//Add Int attribute
	void addIntAttribute(GLuint index, int dimension, VBO* vbo, int stride = 0, int offset = 0);
	//Add double attribute
	void addDoubleAttribute(GLuint index, int dimension, VBO* vbo, int stride = 0, int offset = 0);
	/* Add an ibo. If not specified from now on VAO will render with elements. You need to add an ibo
	 * before calling setMultidrawElementsNumberOfIndices. 
	 */
	void addIBO(IBO *ibo);

	/* Make attribute id = index, instanced. It will be accessed according to instance number
	 * divided by div, instead of using verdex number.
	 */
	void instancedAttribute(GLuint index, GLuint div = 1);

	// Enable attribute array by id.
	void enableAttribute(GLuint index);
	// Disable attribute array by id.
	void disableAttribute(GLuint index);

	//Set the number of elements (number of vertex) to draw. By default this is the number of vertex of the first VBO added to the VAO.
	void setDrawArraysNumberOfElements(GLsizei elements);
	//Set the number of elements for each Array we want to draw. You MUST specify this before calling multiDraw()
	void setMultiDrawArraysNumberOfElements(vector<GLsizei>& multipleElements);
	/* Set the number of indices and the vertex number offset. You MUST specify this before calling multiDraw()
	 * numberOfIndices: number of Indices each draw call has.
	 * vertexsOffset: vertex pointer to the first vertex for each draw call (offset for the indices).
	 */
	void setMultiDrawElementsNumberOfIndices(vector<GLsizei>& numberOfIndices, vector<GLsizei>& vertexsOffset);

	// Normal draw calls.
	void draw();
	void drawArrays();
	void drawElements();

	// Instanced draw calls.
	void drawInstanced(GLsizei times);
	void drawArraysInstanced(GLsizei times);
	void drawElementsInstanced(GLsizei times);

	// Multi draw calls.
	void multiDraw();
	void multiDrawArrays();
	void multiDrawElements();

	GLuint id;
	GLenum mode;
	IBO *ibo;
	GLsizei elements;
	vector<GLsizei> arraysFirst, arraysCount;
	vector<GLsizei> vertexsOffset, elementsCount;
	vector<GLvoid*> elementsOffset;
};


/*
	glBindVertexArray

	Enable/disable array access:
		glEnableVertexAttribArray(index);
		glDisableVertexAttribArray(index);


		*/
#endif