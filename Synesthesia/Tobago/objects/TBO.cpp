#include "TBO.h"

Texture::Texture(GLenum target) {
	this->target = target;
	if(target == GL_TEXTURE_2D_MULTISAMPLE || target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) multisample = true;
	else multisample = false;
	width = height = depth = 0;
	samples = 1;
	glGenTextures(1, &id);
}

Texture::Texture(const char* filename, IMAGE_TYPE type) {
	this->target = GL_TEXTURE_2D;
	width = height = depth = 0;
	samples = 1;
	multisample = false;
	glGenTextures(1, &id);
    if(type == PNG) loadFromPNG(filename);
    else loadFromJPG(filename);
}

Texture::Texture(GLenum target, int width, int height, int depth, GLuint id) {
    this->id = id;
    this->target = target;
    if(target == GL_TEXTURE_2D_MULTISAMPLE || target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) multisample = true;
    else multisample = false;
    this->width = width;
    this->height = height;
    this->depth = depth;
    samples = 1;
}

Texture::~Texture() {
	glDeleteTextures(1, &id);
}

void Texture::bind() {
	glBindTexture(target, id);
}

void Texture::unbind() {
	glBindTexture(target, 0);
}

void Texture::generateMipmap() {
	bind();
	glGenerateMipmap(target);
}

void Texture::bindToGLSL(unsigned int id) {
	glActiveTexture(GL_TEXTURE0+id);
	bind();
}

void Texture::loadFromPNG(const char* filename) {
	unsigned char* image;
	unsigned w, h;
	unsigned error;

	error = LodePNG_decode32_file(&image, &w, &h, filename);
	if(error) Tobago.log->write(Log::ERROR) << "Error loading PNG texture: " << filename;

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	setData((int)w, (int)h, image);
	free(image);
}

void Texture::loadFromJPG(const char* filename) {
    unsigned char* image;
	int w, h, comps;
    
    image = jpgd::decompress_jpeg_image_from_file(filename, &w, &h, &comps, 3);

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    if(comps == 1) setData(w, h, image, 0, GL_UNSIGNED_BYTE, GL_RGBA, GL_RED);
    else setData(w, h, image, 0, GL_UNSIGNED_BYTE, GL_RGBA, GL_RGB);
	free(image);
}

void Texture::setData(int width, int height,
					  const GLvoid *data /* = NULL */,
					  int mipmapLevel /* = 0 */,
					  GLenum dataType /* = GL_UNSIGNED_BYTE */,
					  GLenum internalFormat /* = GL_RGBA */,
					  GLenum format /* = GL_RGBA */) {
	bind();
	glTexImage2D(target, mipmapLevel, internalFormat, width, height, 0, format, dataType, data);
	this->width = width;
	this->height = height;
}

void Texture::setData(int width, int height, int depth,
					  const GLvoid *data /* = NULL */,
					  int mipmapLevel /* = 0 */,
					  GLenum dataType /* = GL_UNSIGNED_BYTE */,
					  GLenum internalFormat /* = GL_RGBA */,
					  GLenum format /* = GL_RGBA */) {
	bind();
	glTexImage2D(target, mipmapLevel, internalFormat, width, height, 0, format, dataType, data);
	this->width = width;
	this->height = height;
	this->depth = depth;
}

void Texture::setData(int width,
					  const GLvoid *data /* = NULL */,
					  int mipmapLevel /* = 0 */,
					  GLenum dataType /* = GL_UNSIGNED_BYTE */,
					  GLenum internalFormat /* = GL_RGBA */,
					  GLenum format /* = GL_RGBA */) {
	bind();
	glTexImage2D(target, mipmapLevel, internalFormat, width, height, 0, format, dataType, data);
	this->width = width;
}

void Texture::setDataMultisample(int width, int height,
								 int samples,
								 GLenum internalFormat /* = GL_RGBA */,
								 GLboolean fixedLocation /* = true */) {
	bind();
	glTexImage2DMultisample(target, samples, internalFormat, width, height, fixedLocation);
	this->width = width;
	this->height = height;
}

void Texture::setDataMultisample(int width, int height, int depth,
								 int samples,
								 GLenum internalFormat /* = GL_RGBA */,
								 GLboolean fixedLocation /* = true */) {
	bind();
	glTexImage3DMultisample(target, samples, internalFormat, width, height, depth, fixedLocation);
	this->width = width;
	this->height = height;
	this->depth = depth;
}

void Texture::setBuffer(GLenum internalFormat, BO* buffer) {
	bind();
	glTexBuffer(target, internalFormat, buffer->id);
}

void Texture::setMipmapLevels(unsigned int min, unsigned int max) {
	bind();
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, min);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL,  max);
}

void Texture::setSwizzle(GLint r, GLint g, GLint b, GLint a) {
	GLint swizzleMask[] = {r, g, b, a};
	bind();
	glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
}

void Texture::setMagnificationFilter(GLenum filter) {
	bind();
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
}

void Texture::setMinificationFilter(GLenum filter) {
	bind();
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);
}

void Texture::setAnisotropicFilter(float samples) {
	bind();
	glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, samples);
}

void Texture::setTextureWrapS(GLenum mode) {
	bind();
	glTexParameteri(target, GL_TEXTURE_WRAP_S, mode);
}

void Texture::setTextureWrapT(GLenum mode) {
	bind();
	glTexParameteri(target, GL_TEXTURE_WRAP_T, mode);
}

void Texture::setTextureWrapR(GLenum mode) {
	bind();
	glTexParameteri(target, GL_TEXTURE_WRAP_R, mode);
}

void Texture::setBorderColor(glm::vec4 *color) {
	bind();
	glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, &(*color)[0]);
}

void Texture::setComparisonMode(bool enabled) {
	if(enabled) {
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	} else {
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
}

void Texture::setComparisonFunction(GLenum f) {
	glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, f);
}


oldTBO::oldTBO() {
//	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &theID);
	glBindTexture(GL_TEXTURE_2D, theID);
	glBindTexture(GL_TEXTURE_2D, 0);
	width = 0;
	height = 0;
}

oldTBO::oldTBO(GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, bool goodfiltering) {
//	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &theID);
	glBindTexture(GL_TEXTURE_2D, theID);

	load(internalFormat, width, height, format, type, data, goodfiltering);

	glBindTexture(GL_TEXTURE_2D, 0);
}

oldTBO::oldTBO(const char* filename, bool goodfiltering) {
//	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &theID);
	glBindTexture(GL_TEXTURE_2D, theID);

	load(filename, goodfiltering);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void oldTBO::load(GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, bool goodfiltering) {
//	glEnable(GL_TEXTURE_2D); //just in case...
	glBindTexture(GL_TEXTURE_2D, theID);

	if(goodfiltering) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_LINEAR_MIPMAP_LINEAR); 
		//glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	}
	if(data == NULL) glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);

	glBindTexture(GL_TEXTURE_2D, 0); //just in case...

	this->width = width;
	this->height = height;
}

void oldTBO::load(const char* filename, bool goodfiltering) {
	unsigned char* image;
	unsigned w, h;
	unsigned error;

	error = LodePNG_decode32_file(&image, &w, &h, filename);
	if(error) TOBAGO::log.write(Log::ERROR) << "Error loading PNG";
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	this->width = w;
	this->height = h;

//	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, theID);

	if(goodfiltering) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_LINEAR_MIPMAP_LINEAR); 
		//glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	}

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0, GL_RGBA,GL_UNSIGNED_BYTE,image);
	free(image);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void oldTBO::bind(int id) {
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0+id);
	glBindTexture(GL_TEXTURE_2D, theID);
}

void oldTBO::qualite(bool qualite) {
//	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, theID);

	if(qualite) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_LINEAR_MIPMAP_LINEAR); 
		//glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	}


}

void oldTBO::erase() {
	glDeleteTextures(1, &theID);
//	glDisable(GL_TEXTURE_2D); //not sure...
}

void oldTBO::clamp(bool doit) {
	glBindTexture(GL_TEXTURE_2D, theID);

	if(!doit) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}