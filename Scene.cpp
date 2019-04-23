/*
@file Scene.cpp
*/
#include "Scene.h"
#include <iostream>

/*
コンストラクタ

@param name シーン名
*/
Scene::Scene(const char* name) : name(name)
{
	std::cout << "Scene コンストラクタ: " << name << "\n";
}

/*
デストラクタ
*/
Scene::~Scene()
{
	Finalize();
	std::cout << "Scene デストラクタ: " << name << "\n";
}

/*
シーンを活動状態にする
*/
void Scene::Play()
{
	isActive = true;
	std::cout << "Scene Play: " << name << "\n";
}

/*
シーンを停止状態にする
*/
void Scene::Stop()
{
	isActive = false;
	std::cout << "Scene Stop: " << name << "\n";
}

/*
シーンを表示する
*/
void Scene::Show()
{
	isVisible = true;
	std::cout << "Scene Show: " << name << "\n";
}

/*
シーンを非表示する
*/
void Scene::Hide()
{
	isActive = false;
	std::cout << "Scene Hide: " << name << "\n";
}

/*
シーン名を取得する

@param シーン名
*/
const std::string& Scene::Name() const
{
	return name;
}

/*
シーンの活動状態を調べる

@retval true  活動している
@retval false 停止している
*/
bool Scene::IsActive() const
{
	return isActive;
}

/*
シーンを表示状態を調べる

@retval true  表示状態
@retval false 非表示状態
*/
bool Scene::IsVisible() const
{
	return isVisible;
}