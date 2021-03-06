/*
@file TitleScene.cpp
*/
#include "TitleScene.h"
#include "MainGameScene.h"
#include "StatusScene.h"
#include "GLFWEW.h"

/*
シーンを初期化する

@retval true  初期化成功
@retval false 初期化失敗 ゲーム進行負荷につき、プログラムを終了すること
*/
bool TitleScene::Initialize()
{
	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);
	Sprite spr(Texture::Image2D::Create("Res/ActionTitleBg.dds"));
	spr.Scale(glm::vec2(1.5));
	sprites.push_back(spr);

	// BGMを再生する
	bgm = Audio::Engine::Instance().Prepare("Res/Audio/Title.mp3");
	bgm->Play(Audio::Flag_Loop);
	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");

	return true;
}

/*
プレイヤーの入力を処理する
*/
void TitleScene::ProcessInput()
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	
	if (timer <= 0 && window.GetGamePad().buttonDown & GamePad::START)
	{
		se = Audio::Engine::Instance().Prepare("Res/Audio/Start.wav");
		se->SetVolume(0.1f);
		se->Play();
		timer = 0.5f;
	}	
}

/*
シーンを更新する

@param deltaTime 前回の更新からの経過時間（秒）
*/
void TitleScene::Update(float deltaTime)
{
	spriteRenderer.BeginUpdate();
	for (const Sprite& e : sprites)
	{
		spriteRenderer.AddVertices(e);
	}
	spriteRenderer.EndUpdate();

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const float w = window.Width();
	const float h = window.Height();
	const float lineHenght = fontRenderer.LineHeight();
	fontRenderer.BeginUpdate();
	//文字列を指定するときに先頭に「L」を付けると「UTF-16」文字列としてビルドされる
	fontRenderer.AddString(glm::vec2(-128, 0), L"ステルスゲーム");
	fontRenderer.AddString(glm::vec2(-130, -200), L"Press Enter Button");
	fontRenderer.EndUpdate();

	// シーン切り替え待ち
	if (timer > 0)
	{
		timer -= deltaTime;
		if (timer <= 0)
		{
			bgm->Stop();
			SceneStack::Instance().Replace(std::make_shared<StatusScene>());
			return;
		}
	}
}

/*
シーンを描画する
*/
void TitleScene::Render()
{
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);
}