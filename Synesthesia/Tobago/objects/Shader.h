//
//  Shader.h
//

#ifndef shader_h
#define shader_h

#include "../init/initTobago.h"
#include <cstdlib>
#include <string>
#include <fstream>
#include <map>

class Shader {
public:
	Shader();
	~Shader();

	bool loadFromFile(GLenum type, const char* filename); //false -> shader not added (error), true -> shadder added.
	bool loadFromString(GLenum type, const char* source);
	bool link();

	void use();

	void addUniform(const string& uniform);
	void addAttribute(const string& attribute);

	GLint operator[](const string& attribute);
	GLint operator()(const string& uniform);

	void operator()(const string& uniform, GLfloat v0);
	void operator()(const string& uniform, GLint v0);
	void operator()(const string& uniform, glm::vec2 *v2);
	void operator()(const string& uniform, glm::vec3 *v3);
	void operator()(const string& uniform, glm::vec4 *v4);
	void operator()(const string& uniform, glm::mat2 *m2);
	void operator()(const string& uniform, glm::mat3 *m3);
	void operator()(const string& uniform, glm::mat4 *m4);

	GLuint p;
	bool printShaderInfoLog(GLuint shader, const char* source); //checks for errors, return true if errors found.
	bool printProgramInfoLog(GLuint program);
	bool readFile(const char* filename, std::string& buffer);
	GLuint shaders[5];
	int numShaders;
	std::map<std::string, GLint> attributeList;
	std::map<std::string, GLint> uniformList;
};

#endif
