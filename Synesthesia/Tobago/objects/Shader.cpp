#include "Shader.h"


Shader::Shader() {
	numShaders = 0;
	p = 0;
	attributeList.clear();
	uniformList.clear();
}

Shader::~Shader() {
	attributeList.clear();
	uniformList.clear();
	for(int i=0; i<numShaders; i++) glDeleteShader(shaders[i]);
	glDeleteProgram(p);
}

bool Shader::loadFromFile(GLenum type, const char* filename) {
	std::string buffer;
	if(!readFile(filename, buffer)) {
		Tobago.log->write(Log::ERROR) << "Could not open " << filename << " shader file.";
		return false;
	}

	return loadFromString(type, buffer.c_str());
}

bool Shader::loadFromString(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	if(printShaderInfoLog(shader, source)) return false;

	shaders[numShaders] = shader;
	numShaders++;
	return true;
}

bool Shader::link() {
	p = glCreateProgram();

	for(int i=0; i<numShaders; i++) glAttachShader(p, shaders[i]);
	glLinkProgram(p);

	if(printProgramInfoLog(p)) return false;

	return true;
}

void Shader::use() {
	glUseProgram(p);
}


void Shader::addUniform(const string& uniform) {
	GLint uid = glGetUniformLocation(p, uniform.c_str());
	if(uid == -1) Tobago.log->write(Log::WARNING) << "Uniform not found: " << uniform;
	uniformList[uniform] = uid;
}

void Shader::addAttribute(const string& attribute) {
	GLint aid = glGetAttribLocation(p, attribute.c_str());
	if(aid == -1) Tobago.log->write(Log::WARNING) << "Attribute not found: " << attribute;
	attributeList[attribute] = aid;
}

GLint Shader::operator[](const string& attribute) {
	return attributeList[attribute];
}

GLint Shader::operator()(const string& uniform) {
	return uniformList[uniform];
}

void Shader::operator()(const string& uniform, GLfloat v0) {
	glUniform1f(uniformList[uniform], v0);
}

void Shader::operator()(const string& uniform, GLint v0) {
	glUniform1i(uniformList[uniform], v0);
}

void Shader::operator()(const string& uniform, glm::vec2 *v2) {
	glUniform2fv(uniformList[uniform], 1, &(*v2)[0]);
}

void Shader::operator()(const string& uniform, glm::vec3 *v3) {
	glUniform3fv(uniformList[uniform], 1, &(*v3)[0]);
}

void Shader::operator()(const string& uniform, glm::vec4 *v4) {
	glUniform4fv(uniformList[uniform], 1, &(*v4)[0]);
}

void Shader::operator()(const string& uniform, glm::mat2 *m2) {
	glUniformMatrix2fv(uniformList[uniform], 1, GL_FALSE, &(*m2)[0][0]);
}

void Shader::operator()(const string& uniform, glm::mat3 *m3) {
	glUniformMatrix3fv(uniformList[uniform], 1, GL_FALSE, &(*m3)[0][0]);
}

void Shader::operator()(const string& uniform, glm::mat4 *m4) {
	glUniformMatrix4fv(uniformList[uniform], 1, GL_FALSE, &(*m4)[0][0]);
}

bool Shader::readFile(const char* filename, std::string& buffer) {
	std::ifstream fp;
	fp.open(filename, std::ios_base::in);
	if(!fp) return false;

	std::string line;
	bool commented = false;
	while(std::getline(fp, line)) {
		std::size_t openMLComment, closeMLComment, include, lineComment;
		openMLComment = line.find("/*");
		closeMLComment = line.find("*/");
		include = line.find("#include");
		lineComment = line.find("//");

		if(openMLComment != std::string::npos) commented = true;
		if(closeMLComment != std::string::npos) commented = false;

		//Not in comment, include appears in the line, and there is no line comment or it's after include.
		if(!commented && include != std::string::npos &&
			(lineComment == std::string::npos || lineComment > include)	) {
				std::size_t start = line.find("\"");
				std::size_t end = line.find("\"", start+1);
				std::string includeFile = line.substr(start+1, end-start-1);

				buffer.append("//Included file: ");
				buffer.append(includeFile);
				buffer.append("\r\n");
				if(!readFile(includeFile.c_str(), buffer)) {
					Tobago.log->write(Log::WARNING) << "Could not open shader file: " << includeFile << " included from " << filename;
				}
				buffer.append("//End of included file\r\n");
		} else {
			buffer.append(line);
			buffer.append("\r\n");
		}
	}

	fp.close();

	return true;
}

bool Shader::printShaderInfoLog(GLuint shader, const char* source) {
    int infologLength = 0;
    int charsWritten  = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
	
    if (infologLength > 1)
    {
        GLchar *infoLog = new GLchar[infologLength];
        glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
		Tobago.log->write(Log::ERROR) << "Shader compiling error:" << std::endl << infoLog << "Full shader dump: " << endl << source << "END of Shader dump" << endl;
        delete [] infoLog;
		return true;
    }

	return false;
}

bool Shader::printProgramInfoLog(GLuint program) {
    int infologLength = 0;
    int charsWritten  = 0;
    
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 1)
    {
		GLchar *infoLog = new GLchar[infologLength];
        glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
		Tobago.log->write(Log::ERROR) << "Shader program linking error:" << infoLog;
		delete [] infoLog;
		return true;
    }

	return false;
}
