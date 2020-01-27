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

/*
シーンスタックを取得する

@retrun シーンスタック
*/
SceneStack& SceneStack::Instance()
{
	static SceneStack instance;
	return instance;
}

/*
コンストラクタ
*/
SceneStack::SceneStack()
{
	stack.reserve(16); //スタックの予約サイズ（シーンの数）を16個に設定d2
}

/*
シーンをプッシュする

@param p 新しいシーン
*/
void SceneStack::Push(ScenePtr p)
{
	//シーンが積まれているか調べる
	if (!stack.empty())
	{
		//現在のシーンを停止
		Current().Stop();
	}
	//シーンをプッシュ
	stack.push_back(p);
	std::cout << "[シーン プッシュ] " << p->Name() << "\n";
	//新しいシーンを起動
	Current().Initialize();
	Current().Play();
}

/*
シーンをポップする
*/
void SceneStack::Pop()
{
	//スタック状態を確認
	if (stack.empty())
	{
		//詰めれてなかったら警告表示
		std::cout << "[シーン ポップ] [警告] シーンスタックが空です." << "\n";
		return;
	}
	//現在上に積まれているシーンを停止
	Current().Stop();
	Current().Finalize();
	const std::string sceneName = Current().Name();
	stack.pop_back();
	std::cout << "[シーン ポップ] " << sceneName << "\n";
	//ポップして、まだシーンが積まれていたら実行
	if (!stack.empty())
	{
		Current().Play();
	}
}

/*
シーンを置き換える

@param p 新しいシーン
*/
void SceneStack::Replace(ScenePtr p)
{
	std::string sceneName = "(Empty)";
	if (stack.empty())
	{
		std::cout << "[シーン リプレース] [警告] シーンスタックが空です." << "\n";
	}
	else
	{
		sceneName = Current().Name();
		Current().Stop();
		Current().Finalize();
		stack.pop_back();
	}
	stack.push_back(p);
	std::cout << "[シーン リプレース] " << sceneName << " -> " << p->Name() << "\n";
	Current().Initialize();
	Current().Play();
}

/*
現在のシーンを取得する(メンバ関数)

@retrun 現在のシーン
*/
Scene& SceneStack::Current()
{
	//constのない(std::vectorの)back関数を呼び出し
	return *stack.back();
}

/*
現在のシーンを取得する(コンストメンバ関数)

@retrun 現在のシーン
*/
const Scene& SceneStack::Current() const
{
	//constのある(std::vectorの)back関数を呼び出し
	return *stack.back();
}

/*
シーンの数を取得する

@retrun スタックに積まれているシーンの数
*/
size_t SceneStack::Size() const
{
	return stack.size();
}

/*
スタックが空かどうか調べる

@retval true  スタックは空
@retval false スタックに1つ以上のシーンが積まれている
*/
bool SceneStack::Empty() const
{
	return stack.empty();
}

/*
シーンを更新する

@param deltaTime 前回の更新からの経過時間(秒)
*/
void SceneStack::Update(float deltaTime)
{
	if (!Empty())
	{
		Current().ProcessInput();
	}
	for (ScenePtr& e : stack)
	{
		if (e->IsActive())
		{
			e->Update(deltaTime);
		}
	}
}

/*
シーンを描画する
*/
void SceneStack::Render()
{
	for (ScenePtr& e : stack)
	{
		if (e->IsVisible())
		{
			e->Render();
		}
	}
}