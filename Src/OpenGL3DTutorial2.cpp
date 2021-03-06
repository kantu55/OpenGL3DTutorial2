// OpenGL3DTutorial2.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include <Windows.h>
#include "TitleScene.h"
#include "GLFWEW.h"
#include "SkeletalMesh.h"
#include "Audio/Audio.h"
#include <iostream>

int main()
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	window.Init(1920, 1080, u8"アクションゲーム");
	//ShowCursor(FALSE);
	// 音声再生プログラムを初期化する
	Audio::Engine& audioEngine = Audio::Engine::Instance();
	if (!audioEngine.Initialize())
	{
		return 1;
	}
	
	// スケルタル・アニメーションを利用可能にする
	Mesh::SkeletalAnimation::Initialize();
	
	SceneStack& sceneStack = SceneStack::Instance();
	/*
	std::make_shared関数
	shared_ptrと呼ばれるクラス関数を作成する関数
	shared_ptrは自分が削除されるときに、指している変数を
	削除してくれる
	*/
	sceneStack.Push(std::make_shared<TitleScene>());

	while(!window.ShouldClose())
	{
		const float deltaTime = window.DeltaTime();
		window.UpdateTimer();

		//ESCキーが押されたら終了ウィンドウを表示
		if (window.IsKeyPressed(GLFW_KEY_ESCAPE))
		{
			if (MessageBox(nullptr, L"ゲームを終了しますか？", L"終了", MB_OKCANCEL) == IDOK)
			{
				break;
			}
		}

		// スケルタル・アニメーション用データの作成準備
		Mesh::SkeletalAnimation::ResetUniformData();

		sceneStack.Update(deltaTime);

		// スケルタル・アニメーション用データをGPUメモリの転送
		Mesh::SkeletalAnimation::UploadUniformData();

		// 音声再生プログラムを更新する
		audioEngine.Update();

		//バックバッファを消去する
		glClearColor(0.8f, 0.2f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//GLコンテキストのパラメータを設定
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		sceneStack.Render();
		window.SwapBuffers();
	}

	// スケルタル・アニメーションの利用を終了する
	Mesh::SkeletalAnimation::Finalize();

	// 音声プログラムを終了する
	audioEngine.Finalize();
}

