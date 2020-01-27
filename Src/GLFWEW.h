/*
@file GLFWEW.h
*/
#ifndef GLFWEW_H_INCLUDED
#define GLFWEW_H_INCLUDED
#include <GL\glew.h>
#include "GamePad.h"
#include <GLFW\glfw3.h>
#include <glm\vec3.hpp>

namespace GLFWEW
{
	/*
	GLFWとGLEWのラッパークラス
	*/
	class Window
	{
	private:
		Window();
		~Window();
		/*
		GLFWやGLEWを管理するオブジェクトを複数作れるようになると
		初期化処理や終了処理がオブジェクトの数だけ実行されてしまう危険性があるため
		Windowクラスの利用者が作成・コピーすることを禁止している
		*/
		Window(const Window&) = delete;  //コピーコンストラクタ
		Window& operator = (const Window&) = delete;  //コピー代入演算子
		void UpdateGamePad();

		bool isGLFWInitialized = false;
		bool isInitialized = false;
		GLFWwindow* window = nullptr;
		double previousTime = 0;
		double deltaTime = 0;
		int width = 0;
		int height = 0;
		GamePad gamepad;

	public:
		/*
		Windowクラスを利用できないため、instance関数をstatic関数として
		定義し、この関数の中でオブジェクトを作成し、管理するようにする
		*/
		static Window& Instance();
		bool Init(int w, int h, const char* title);
		bool ShouldClose() const;
		void SwapBuffers() const;

		void InitTimer();
		void UpdateTimer();
		double DeltaTime() const;
		double Time() const;

		//画面サイズの情報を取得している
		int Width() const { return width; }
		int Height() const { return height; }

		bool IsKeyPressed(int key) const;
		bool IsMousePressed(int mouse) const;
		void CursorPos(double mousex, double mousey) const;
		const GamePad& GetGamePad() const;
		glm::vec3 CursorPos() const;
		void CenterCursor() const;

		float windowWidth = 1920;
		float windowHeight = 1080;
	};
} // namespace GLFEEW

#endif // GLFWEW_H_INCLUDED
