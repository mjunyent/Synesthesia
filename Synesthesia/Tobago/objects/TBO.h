#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "lodepng/lodepng.h"
#include "jpeg-compressor/jpgd.h"
#include "../utility/log.h"
#include "../init/initTobago.h"
#include "Buffer_Object.h"

class Texture {
public:
    enum IMAGE_TYPE { PNG, JPG };
	/* Types of texture:
		GL_TEXTURE_1D							<- glTexImage1D
		GL_TEXTURE_2D							<- glTexImage2D
		GL_TEXTURE_3D							<- glTexImage3D
		GL_TEXTURE_1D_ARRAY						<- glTexImage2D
		GL_TEXTURE_2D_ARRAY						<- glTexImage3D
		GL_TEXTURE_RECTANGLE					<- glTexImage2D (level must be 0).
		GL_TEXTURE_BUFFER
		GL_TEXTURE_2D_MULTISAMPLE				<- glTexImage2DMultisample
		GL_TEXTURE_2D_MULTISAMPLE_ARRAY			<- glTexImage3DMultisample
		(and others).
	*/
	Texture(GLenum target);
	Texture(const char* filename, IMAGE_TYPE type);
    Texture(GLenum target, int width, int height, int depth, GLuint id);
	~Texture();

	void bind();
	void unbind();

	void generateMipmap();

	/* Bind texture to texture image unit */
	void bindToGLSL(unsigned int id);

	void loadFromPNG(const char* filename);
    void loadFromJPG(const char* filename);
	/* Allocates memory in GPU and transfers texture data to it.
	 * width, height, depth: size of texture
	 * data: pointer to array of data. NULL by default.
	 * mipmapLevel: mipmap level we're accessing. 0 by default.
	 * dataType: format of the data we'll feed to the texture.
				Most used values are:
				GL_UNSIGNED_BYTE		<- also default value.
				GL_BYTE
				GL_UNSIGNED_SHORT
				GL_SHORT
				GL_UNSIGNED_INT
				GL_INT
				GL_FLOAT
	 * internalFormat: number of color components in the texture.
				Most used values are:
				(not sized)
				GL_RGBA
				GL_RGB
				GL_DEPTH_COMPONENT
				GL_STENCIL_INDEX
				(sized)
				GL_RGB10_A2
				GL_DEPTH_COMPONENT24 / GL_DEPTH_COMPONENT32F
				GL_STENCIL_INDEX8
				GL_DEPTH_24_STENCIL8

				Default color format enumerators (sized): GL_[components][size][type]
					Where [components] is R, RG, RGB, RGBA
						  [type] can be "": normalized integer.
										"_SNORM": signed normalized integer [-1,1].
										"F": floating point.
										"I": integral.
										"UI": unsigned integer.
				For more information about internalFormat check: http://www.opengl.org/wiki/Image_Format
	 * format: format of the pixel data.
				Most used values are:
				GL_RGBA (normalized integer or floating point color)
				GL_RGBA_INTEGER (non normalized integer)
				GL_DEPTH_COMPONENT
atoi()				GL_STENCIL_INDEX (OGL 4.4)
				GL_DEPTH_STENCIL
				*RGBA can be changed for RED, GREEN, BLUE, RG, RGB, BGR, BGRA.

	For more information check : http://www.opengl.org/wiki/GLAPI/glTexImage2D    */
	void setData(int width, int height,
				 const GLvoid *data = NULL,
				 int mipmapLevel = 0,
				 GLenum dataType = GL_UNSIGNED_BYTE,
				 GLenum internalFormat = GL_RGBA,
				 GLenum format = GL_RGBA);

	void setData(int width, int height, int depth,
				 const GLvoid *data = NULL,
				 int mipmapLevel = 0,
				 GLenum dataType = GL_UNSIGNED_BYTE,
				 GLenum internalFormat = GL_RGBA,
				 GLenum format = GL_RGBA);

	void setData(int width,
				 const GLvoid *data = NULL,
				 int mipmapLevel = 0,
				 GLenum dataType = GL_UNSIGNED_BYTE,
				 GLenum internalFormat = GL_RGBA,
				 GLenum format = GL_RGBA);

	/* Like setData for multisample textures */
	void setDataMultisample(int width, int height,
							int samples,
							GLenum internalFormat = GL_RGBA,
							GLboolean fixedLocation = false);

	void setDataMultisample(int width, int height, int depth,
							int samples,
							GLenum internalFormat = GL_RGBA,
							GLboolean fixedLocation = false);

	/* Set BO where data comes from, only for GL_TEXTURE_BUFFER
	   Internal size like in setData. GL_[components][size][type]
	   Examples: GL_R16 (normalized), GL_RG8I, GL_RGB16F, GL_RGBA32UI...
	*/
	void setBuffer(GLenum internalFormat, BO* buffer);

	// TODO http://stackoverflow.com/questions/425401/what-is-the-preferred-way-to-show-large-images-in-opengl 
	// TODO glTexSubImage*D

	//Texture Parameters//

	/* Set max an minimum mipmap levels, levels outside range won't be accessed. */
	void setMipmapLevels(unsigned int min, unsigned int max);

	/* Specify where the RGBA values of the texture come from.
	 * Possible values are: GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_ZERO, GL_ONE
	 */
	void setSwizzle(GLint r, GLint g, GLint b, GLint a);


	//Sampling parameters//

	/* Magnification filter type. Can be GL_LINEAR, GL_NEAREST */
	void setMagnificationFilter(GLenum filter);

	/* Minification filter type. Without mipmap can be GL_NEAREST, GL_LINEAR
	 * Otherwise options are: GL_*_MIPMAP_* where * = NEAREST or LINEAR
	 * First * is within mip-level, second * is between mip-levels.
	 */
	void setMinificationFilter(GLenum filter);

	/* Anisotropic filter. Specify number of samples to take (as float). */
	void setAnisotropicFilter(float samples);

	/* Sets sampling mode on edges.
	 * GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE,
	 * GL_CLAMP_TO_BORDER (you need to set the border color),
	 */
	void setTextureWrapS(GLenum mode);
	void setTextureWrapT(GLenum mode);
	void setTextureWrapR(GLenum mode);

	/* Sets the border color (RGBA), needed if you set TextureWrap to GL_CLAMP_TO_BORDER */
	void setBorderColor(glm::vec4 *color);

	/* Enable or disable comparison mode */
	void setComparisonMode(bool enabled);

	/* r Reference given by GLSL call, t value from Texture
	 * GL_LEQUAL		1 if r <= t
	 * GL_GEQUAL		1 if r >= t
	 * GL_LESS			1 if r <  t
	 * GL_GREATER		1 if r >  t
	 * GL_EQUAL			1 if r == t
	 * GL_NOTEQUAL		1 if r != t
	 * GL_ALWAYS		1
	 * GL_NEVER			0
	 */
	void setComparisonFunction(GLenum f);

	GLenum target;
	GLuint id;
	bool multisample;
	int width, height, depth, samples;
};

class oldTBO {
public:
	//Creates empty texture, (fill with load).
	oldTBO();
	
	//Creates texture from array.
	oldTBO(GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, bool goodfiltering);
	
	//Creates texture from PNG.
	oldTBO(const char* filename, bool goodfiltering);
	

	void load(GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, bool goodfiltering);

	void load(const char* filename, bool goodfiltering);

	//Binds to TextureUnit id
	void bind(int id);

	//Set filtering...
	void qualite(bool qualite);
	
	//erases it?
	void erase();

	void clamp(bool doit);

	//texture name.
	GLuint theID;
	int width, height;
};