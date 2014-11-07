#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "../utility/log.h"
#include "TBO.h"

using namespace std;

class FBO {
public:
	enum FBOAttachment { NONE, DEPTH, STENCIL, DEPTHSTENCIL, COLOR0, COLOR1, COLOR2, COLOR3, COLOR4, COLOR5, COLOR6, COLOR7, COLOR8, COLOR9, COLOR10, COLOR11, COLOR12, COLOR13, COLOR14, COLOR15 };

	FBO();
	~FBO();

	// Binds fbo, sets viewport and clears buffers.
	void use(bool clear= true);
	void unuse();

	//FUNCTIONS TO CREATE FBO//
		void bind();
		/* FBO MUST BE BIND FIRST */
		void attachTexture(Texture* t, FBOAttachment type, int mipmapLevel=0, int layer=0);
		
		void attachLayeredTexture(Texture* t, FBOAttachment type, int mipmapLevel=0);

		/* Set color attachments where fragment will render (COLOR0, COLOR1...), order matches with layout.
		 * NEEDS TO BIND FBO FIRST.
		 * Example: buffers = COLOR0, NONE, COLOR9 -> fragment 0 is color0, fragment 1 is NONE, fragment 2 is color 9
		 */
		void setDrawBuffers(FBOAttachment* buffers, int size);
		void setDrawBuffers();

		/* NEEDS TO BIND FIRST */
		bool isFBOcomplete();

		void unbind();

		/* Sets viewport coords for framebuffer (by default they are the maximum intersection of textures) */
		void setViewPortCoords(int startX, int startY, int width, int height);

	//Can't copy depth or stencil with non GL_NEAREST filter.
	void copyTo(FBO* fbo,
				FBOAttachment readBuff,
				FBOAttachment* writeBuffs, int size,
				int srcX0, int srcY0, int srcX1, int srcY1,
				int dstX0, int dstY0, int dstX1, int dstY1,
				GLenum filter = GL_NEAREST,
				bool copyDepth = false, bool copyStencil = false);

	void viewPort(); //Saves current viewport and sets new one.
	/* NEEDS TO BIND FIRST
	 * Values: GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT
	 */
	void clearAll(GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	void popViewPort(); //Sets saved viewport.


	GLuint id;
	int offsetX, offsetY, width, height, depth;
	int previousViewPort[4];
	//Texture 0 is Depth, 1 is Stencil, 2+ are color attachments.
	vector<Texture*> textures;
	GLbitfield clearFlag;
	GLenum *drawBuffers;
	int drawBuffersSize;
};

class oldFBO {
private:
//	GLuint depthrenderbuffer;
	bool deltex;
	void shout_error(GLenum error);
	int viewport[4];

public:
	GLsizei width, height;
	int ntbo;
	vector<oldTBO*> textures;
	oldTBO *depthtexture;
	bool status;

	oldFBO(GLsizei width, GLsizei height, bool bdo, int ntbo, bool *qualite);

	oldFBO(GLsizei width, GLsizei height, vector<oldTBO*> texs, oldTBO *depth, bool *qualite);

	void bind(bool erase=true);

	void unbind();

	void erase();

	void bind_texture(int texture, int id);

	void bind_depth_texture(int id);

	//FBO id
	GLuint theID;
};