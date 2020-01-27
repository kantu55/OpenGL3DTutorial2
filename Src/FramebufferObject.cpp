/*
@file FramebufferObject.cpp
*/
#include "FramebufferObject.h"

/*
フレームバッファオブジェクトを作成する

@param w フレームバッファオブジェクトの幅(ピクセル単位)
@param h フレームバッファオブジェクトの高さ(ピクセル単位)

@return 作成したフレームバッファオブジェクトのへのポインタ
*/
FramebufferObjectPtr FramebufferObject::Create(int w, int h)
{
	FramebufferObjectPtr fbo = std::make_shared<FramebufferObject>();
	if (!fbo)
	{
		return nullptr;
	}

	// テクスチャを作成する
	fbo->texColor = std::make_shared<Texture::Image2D>(Texture::CreateImage2D(
		w, h, nullptr, GL_RGBA, GL_UNSIGNED_BYTE));
	fbo->texDepth = std::make_shared<Texture::Image2D>(Texture::CreateImage2D(
		w, h, nullptr, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F));

	// フレームバッファを作成する
	glGenFramebuffers(1, &fbo->id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		fbo->texColor->Target(), fbo->texColor->Get(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		fbo->texDepth->Target(), fbo->texDepth->Get(), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
}

/*
デストラクタ
*/
FramebufferObject::~FramebufferObject()
{
	if (id)
	{
		glDeleteFramebuffers(1, &id);
	}
}

/*
FBOのIDを取得

@return FBOのID
*/
GLuint FramebufferObject::GetFramebuffer() const
{
	return id;
}

/*
カラーバッファ用テクスチャ

@retrun カラーバッファ用テクスチャ
*/
const Texture::Image2DPtr& FramebufferObject::GetColorTexture() const
{
	return texColor;
}

/*
深度バッファ用テクスチャ

@return 深度バッファ用テクスチャ
*/
const Texture::Image2DPtr& FramebufferObject::GetDepthTexture() const
{
	return texDepth;
}