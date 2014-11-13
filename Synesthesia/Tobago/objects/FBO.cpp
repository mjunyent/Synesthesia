#include "FBO.h"

FBO::FBO() {
	glGenFramebuffers(1, &id);
	this->offsetX = this->offsetY = 0;
	this->width = this->height = INT_MAX;
	this->depth = 0;
	this->textures = vector<Texture*>(20, NULL);
	clearFlag = GL_COLOR_BUFFER_BIT;
}

FBO::~FBO() {
	glDeleteFramebuffers(1, &id);
}

void FBO::use(bool clear) {
	bind();
	viewPort();
	if(clear) glClear(clearFlag);
}

void FBO::unuse() {
	popViewPort();
	unbind();
}

void FBO::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FBO::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::setViewPortCoords(int startX, int startY, int width, int height) {
	this->width = width;
	this->height = height;
	this->offsetX = startX;
	this->offsetY = startY;
}

void FBO::setDrawBuffers(FBOAttachment* buffers, int size) {
	drawBuffers = new GLenum[size];
	drawBuffersSize = size;
	for(int i=0; i<size; i++) {
		if(buffers[i] == NONE) drawBuffers[i] = GL_NONE;
		else drawBuffers[i] = GL_COLOR_ATTACHMENT0 + (buffers[i]-COLOR0);
	}
	glDrawBuffers(size, drawBuffers); // "ntbo" is the size of DrawBuffers
}

void FBO::setDrawBuffers() {
	drawBuffers = new GLenum[16];
	int drawBuffersSize = 16;
	for(int i=COLOR0; i<=COLOR15; i++) {
		if(textures[i] == NULL) drawBuffers[i-COLOR0] = GL_NONE;
		else drawBuffers[i-COLOR0] = GL_COLOR_ATTACHMENT0+(i-COLOR0);
	}
}

void FBO::viewPort() {
	glGetIntegerv(GL_VIEWPORT, previousViewPort);
	glViewport(offsetX, offsetY, width, height);						//set viewport
}

void FBO::popViewPort() {
	glViewport(previousViewPort[0], previousViewPort[1],
			   previousViewPort[2], previousViewPort[3]);
}

void FBO::clearAll(GLbitfield mask /* = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT */) {
	glClear(mask);
}

void FBO::attachTexture(Texture* t, FBOAttachment type, int mipmapLevel/* =0 */, int layer/* =0 */) {
	//Get type of attachment given FBOAttachment type.
	GLenum attachmentPoint;
	if(type == DEPTH) {
		attachmentPoint = GL_DEPTH_ATTACHMENT;
		clearFlag = clearFlag | GL_DEPTH_BUFFER_BIT;
	} else if (type == STENCIL) {
		attachmentPoint = GL_STENCIL_ATTACHMENT;
		clearFlag = clearFlag | GL_STENCIL_BUFFER_BIT;
	} else if (type == DEPTHSTENCIL) {
		attachmentPoint = GL_DEPTH_STENCIL_ATTACHMENT;
		clearFlag = clearFlag | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
	} else {
		attachmentPoint = GL_COLOR_ATTACHMENT0 + (type-COLOR0);
	}

	t->bind();

	if(t->target == GL_TEXTURE_1D || t->target == GL_TEXTURE_BUFFER) {
		glFramebufferTexture1D(GL_FRAMEBUFFER, attachmentPoint, t->target, t->id, mipmapLevel);
	} else if(t->target == GL_TEXTURE_2D
		   || t->target == GL_TEXTURE_RECTANGLE
		   || t->target == GL_TEXTURE_2D_MULTISAMPLE) {

		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, t->target, t->id, mipmapLevel);
	} else { /* GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_3D, GL_TEXTURE_2D_MULTISAMPLE_ARRAY */
		glFramebufferTextureLayer(GL_FRAMEBUFFER, attachmentPoint, t->id, mipmapLevel, layer);
	}

	this->width = min(this->width, t->width);
	this->height = min(this->height, t->height);
	this->textures[type] = t;
}

void FBO::attachLayeredTexture(Texture* t, FBOAttachment type, int mipmapLevel/* =0 */) {
	GLenum attachmentPoint;
	if(type == DEPTH) {
		attachmentPoint = GL_DEPTH_ATTACHMENT;
		clearFlag = clearFlag | GL_DEPTH_BUFFER_BIT;
	} else if (type == STENCIL) {
		attachmentPoint = GL_STENCIL_ATTACHMENT;
		clearFlag = clearFlag | GL_STENCIL_BUFFER_BIT;
	} else if (type == DEPTHSTENCIL) {
		attachmentPoint = GL_DEPTH_STENCIL_ATTACHMENT;
		clearFlag = clearFlag | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
	} else {
		attachmentPoint = GL_COLOR_ATTACHMENT0 + (type-COLOR0);
	}

	t->bind();
	glFramebufferTexture(GL_FRAMEBUFFER, attachmentPoint, t->id, mipmapLevel);

	this->width = min(this->width, t->width);
	this->height = min(this->height, t->height);
	this->depth = t->depth;
	this->textures[type] = t;
}

bool FBO::isFBOcomplete() {
	GLenum check_result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(check_result != GL_FRAMEBUFFER_COMPLETE) {
		switch (check_result) {
			case GL_FRAMEBUFFER_UNDEFINED:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_UNDEFINED";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
				break;
			case GL_FRAMEBUFFER_UNSUPPORTED:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_UNSUPPORTED";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
				break;
			case GL_FRAMEBUFFER_COMPLETE:
				Tobago.log->write(Log::ERROR) << "FBO Creation ok, why is this in LOG?";
				break;
			case 0:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: 0 returned";
			default:
				Tobago.log->write(Log::ERROR) << "FBO Creation error: Error not recognised";
			break;
		}
		return false;
	} return true;

	/* http://www.opengl.org/sdk/docs/man3/xhtml/glCheckFramebufferStatus.xml
		GL_FRAMEBUFFER_UNDEFINED is returned if target is the default framebuffer, but the default framebuffer does not exist.
		GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT is returned if any of the framebuffer attachment points are framebuffer incomplete.
		GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT is returned if the framebuffer does not have at least one image attached to it.
		GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER is returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAWBUFFERi.
		GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER is returned if GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.
		GL_FRAMEBUFFER_UNSUPPORTED is returned if the combination of internal formats of the attached images violates an implementation-dependent set of restrictions.
		GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is returned if the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES.
		GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.
		GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS is returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target.
	*/
}

void FBO::copyTo(FBO* fbo,
				 FBOAttachment readBuff,
				 FBOAttachment* writeBuffs, int size,
				 int srcX0, int srcY0, int srcX1, int srcY1,
				 int dstX0, int dstY0, int dstX1, int dstY1,
				 GLenum filter /* = GL_NEAREST */,
				 bool copyDepth /* = false */, bool copyStencil /* = false */) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->id);

	GLbitfield mask = 0x0;
	if(readBuff != NONE) mask = GL_COLOR_BUFFER_BIT;
	else glReadBuffer(GL_COLOR_ATTACHMENT0 + (readBuff-COLOR0));

	if(copyDepth) mask = mask | GL_DEPTH_BUFFER_BIT;
	if(copyStencil) mask = mask | GL_STENCIL_BUFFER_BIT;

	GLenum *drawBuffers = new GLenum[size];
	for(int i=0; i<size; i++) {
		if(writeBuffs[i] == NONE) drawBuffers[i] = GL_NONE;
		else drawBuffers[i] = GL_COLOR_ATTACHMENT0 + (writeBuffs[i]-COLOR0);
	}
	glDrawBuffers(size, drawBuffers);

	glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1,
					  dstX0, dstY0, dstX1, srcY1,
					  mask, filter);

	glDrawBuffers(fbo->drawBuffersSize, fbo->drawBuffers);
}







oldFBO::oldFBO(GLsizei width, GLsizei height, bool dbo, int ntbo, bool *qualite) 
{
	this->width = width;
	this->height = height;

	int maxDrawBuffers;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
	if(maxDrawBuffers <= ntbo) {
		TOBAGO::log.write(Log::WARNING) << "FBO: Be careful not to exceed MAX_DRAW_BUFFERS number!";
	}

	glGenFramebuffers(1, &theID);
	glBindFramebuffer(GL_FRAMEBUFFER, theID);

	//Create and attach color textures.
	this->ntbo = ntbo;
	textures.resize(ntbo);

	for(int i=0; i<ntbo; i++) {
		//Maybe GL_RGBA32F
		textures[i] = new oldTBO(GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0, qualite[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, textures[i]->theID, 0);
	}

	if(dbo) {
		depthtexture = new oldTBO(GL_DEPTH_COMPONENT24, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, 0, false);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,  depthtexture->theID, 0);
	}

	GLenum *DrawBuffers = new GLenum[ntbo];
	for(int i=0; i<ntbo; i++) DrawBuffers[i] = GL_COLOR_ATTACHMENT0+i;
	glDrawBuffers(ntbo, DrawBuffers); // "ntbo" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	GLenum check_result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(check_result != GL_FRAMEBUFFER_COMPLETE) {
		shout_error(check_result);
		status = false;
	} else status = true;

	if(!status) TOBAGO::log.write(Log::ERROR) << "Error creating FBO";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

oldFBO::oldFBO(GLsizei width, GLsizei height, vector<oldTBO*> texs, oldTBO *depth, bool *qualite) 
{
	this->width = width;
	this->height = height;
	this->ntbo = texs.size();

	int maxDrawBuffers;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
	if(maxDrawBuffers <= ntbo) {
		TOBAGO::log.write(Log::WARNING) << "FBO: Be careful not to exceed MAX_DRAW_BUFFERS number!";
	}

	glGenFramebuffers(1, &theID);
	glBindFramebuffer(GL_FRAMEBUFFER, theID);

	depthtexture = NULL;

	for(unsigned int i=0; i<texs.size(); i++) {
		texs[i]->load(GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0, qualite[i]);
	}
	if(depth != NULL) depth->load(GL_DEPTH_COMPONENT24, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, 0, false);

	// Set "renderedTexture" as our colour attachement #0
	for(unsigned int i=0; i<texs.size(); i++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, texs[i]->theID, 0);
	}
	if(depth != NULL) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,  depth->theID, 0);

	// Set the list of draw buffers.
	GLenum *DrawBuffers = new GLenum[ntbo];
	for(int i=0; i<ntbo; i++) DrawBuffers[i] = GL_COLOR_ATTACHMENT0+i;
	glDrawBuffers(ntbo, DrawBuffers); // "ntbo" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	GLenum check_result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(check_result != GL_FRAMEBUFFER_COMPLETE) {
		shout_error(check_result);
		status = false;
	} else status = true;

	if(!status) TOBAGO::log.write(Log::ERROR) << "Error creating FBO";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void oldFBO::bind(bool erase) 
{
	glBindFramebuffer(GL_FRAMEBUFFER, theID);				//bind buffer
	glGetIntegerv(GL_VIEWPORT, viewport);
//	glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);			//push viewport and enable config
	glViewport(0, 0, width, height);						//set viewport
	if(erase) glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	//clean things
}

void oldFBO::unbind() 
{
//	glPopAttrib();											//pop config
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);	//set viewport
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void oldFBO::erase() {
	for(unsigned int i=0; i<textures.size(); i++) textures[i]->erase();
	if(depthtexture != NULL) depthtexture->erase();
//	glDeleteRenderbuffers(1, &depthrenderbuffer);
	glDeleteFramebuffers(1, &theID);
}

void oldFBO::bind_texture(int texture, int id) 
{
	textures[texture]->bind(id);
}

void oldFBO::bind_depth_texture(int id)
{
	depthtexture->bind(id);
}


void oldFBO::shout_error(GLenum error) {
	switch (error)
	{
		case GL_FRAMEBUFFER_UNDEFINED:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_UNDEFINED";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_UNSUPPORTED";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
			break;
		case GL_FRAMEBUFFER_COMPLETE:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation ok, why is this in LOG?";
			break;
		case 0:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: 0 returned";
		default:
			TOBAGO::log.write(Log::ERROR) << "FBO Creation error: Error not recognised";
		break;
	}
	/* http://www.opengl.org/sdk/docs/man3/xhtml/glCheckFramebufferStatus.xml
	GL_FRAMEBUFFER_UNDEFINED is returned if target is the default framebuffer, but the default framebuffer does not exist.

	GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT is returned if any of the framebuffer attachment points are framebuffer incomplete.

	GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT is returned if the framebuffer does not have at least one image attached to it.

	GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER is returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAWBUFFERi.

	GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER is returned if GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.

	GL_FRAMEBUFFER_UNSUPPORTED is returned if the combination of internal formats of the attached images violates an implementation-dependent set of restrictions.

	GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is returned if the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES.

	GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE is also returned if the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.

	GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS is returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target.
	*/
}