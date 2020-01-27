/*
@file TitleScene.cpp
*/
#include "TitleScene.h"
#include "MainGameScene.h"
#include "StatusScene.h"
#include "GLFWEW.h"

/*
�V�[��������������

@retval true  ����������
@retval false ���������s �Q�[���i�s���ׂɂ��A�v���O�������I�����邱��
*/
bool TitleScene::Initialize()
{
	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);
	Sprite spr(Texture::Image2D::Create("Res/ActionTitleBg.dds"));
	sprites.push_back(spr);

	Sprite buttonNone(Texture::Image2D::Create("Res/ButtonNone.tga"));
	buttonNone.Scale(glm::vec2(0.25f));
	buttonNone.Position(glm::vec3(200, -200, -2));
	sprites.push_back(buttonNone);

	Sprite buttonActive(Texture::Image2D::Create("Res/ButtonActive.tga"));
	buttonActive.Scale(glm::vec2(0.25f));
	buttonActive.Position(glm::vec3(-200, -200, -2));
	sprites.push_back(buttonActive);
	

	// BGM���Đ�����
	bgm = Audio::Engine::Instance().Prepare("Res/Audio/Title.mp3");
	bgm->Play(Audio::Flag_Loop);

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");

	return true;
}

/*
�v���C���[�̓��͂���������
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
	if (window.GetGamePad().buttonDown & GamePad::DPAD_LEFT)
	{
		menu = Menu::start;
		Sprite buttonNone(Texture::Image2D::Create("Res/ButtonNone.tga"));
		buttonNone.Scale(glm::vec2(0.25f));
		buttonNone.Position(glm::vec3(200, -200, -2));
		sprites.push_back(buttonNone);

		Sprite buttonActive(Texture::Image2D::Create("Res/ButtonActive.tga"));
		buttonActive.Scale(glm::vec2(0.25f));
		buttonActive.Position(glm::vec3(-200, -200, -2));
		sprites.push_back(buttonActive);

	}
	if (window.GetGamePad().buttonDown & GamePad::DPAD_RIGHT)
	{
		menu = Menu::end;
		Sprite buttonNone(Texture::Image2D::Create("Res/ButtonNone.tga"));
		buttonNone.Scale(glm::vec2(0.25f));
		buttonNone.Position(glm::vec3(-200, -200, -2));
		sprites.push_back(buttonNone);

		Sprite buttonActive(Texture::Image2D::Create("Res/ButtonActive.tga"));
		buttonActive.Scale(glm::vec2(0.25f));
		buttonActive.Position(glm::vec3(200, -200, -2));
		sprites.push_back(buttonActive);
	}
	
}

/*
�V�[�����X�V����

@param deltaTime �O��̍X�V����̌o�ߎ��ԁi�b�j
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
	//��������w�肷��Ƃ��ɐ擪�ɁuL�v��t����ƁuUTF-16�v������Ƃ��ăr���h�����
	fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHenght), L"�^�C�g�����");
	fontRenderer.AddString(glm::vec2(-128, 0), L"�A�N�V�����Q�[��");
	fontRenderer.EndUpdate();

	// �V�[���؂�ւ��҂�
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
�V�[����`�悷��
*/
void TitleScene::Render()
{
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	spriteRenderer.Draw(screenSize);
	fontRenderer.Draw(screenSize);
}