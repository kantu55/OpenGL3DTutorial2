/*
@file StatusScene.cpp
*/
#include "StatusScene.h"
#include "MainGameScene.h"
#include "GLFWEW.h"

/*
ƒvƒŒƒCƒ„[‚Ì“ü—Í‚ğˆ—‚·‚é
*/
bool StatusScene::Initialize()
{
	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);
	Sprite spr(Texture::Image2D::Create("Res/ActionTitleBg.tga"));
	sprites.push_back(spr);
	SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
	return true;
}