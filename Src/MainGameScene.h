/*
@file MainGameScene.h
*/
#ifndef MAINGAMESCENE_H_INCLUDED
#define MAINGAMESCENE_H_INCLUDED
#include "Scene.h"
#include "Font.h"
#include "Mesh.h"
#include "Terrain.h"
#include "Actor.h"
#include "PlayerActor.h"
#include "Enemy.h"
#include "JizoActor.h"
#include "Light.h"
#include "FramebufferObject.h"
#include <random>
#include <vector>

/*
メインゲーム画面
*/

class MainGameScene : public Scene
{
public:
	MainGameScene() : Scene("MainGameScene") {}
	virtual ~MainGameScene() = default;

	virtual bool Initialize() override;
	virtual void ProcessInput() override;
	virtual void Update(float) override;
	virtual void Render() override;
	virtual void Finalize() override {}

	bool HandleJizoEffects(int id, const glm::vec3& pos);

	//プレイヤーのカメラ位置と注視点
	glm::vec3 viewPos;
	glm::vec3 targetPos;
	glm::vec3 viewoffset;
	glm::vec3 targetoffset;
	glm::vec3 vCamera;
	glm::vec3 vTraget;
	glm::vec3 vCameraTraget;
	glm::vec3 vTragetoffset;
	glm::vec3 cameraVelocity;
	glm::vec3 TragetVelocity;

private:
	bool flag = false;
	std::mt19937 rand;
	int jizoId = -1; // 現在戦闘中のお地蔵様のID
	bool achivements[4] = { false, false, false, false }; // 敵討伐状態
	std::vector<Sprite> sprites;
	SpriteRenderer spriteRenderer;
	FontRenderer fontRenderer;
	Mesh::Buffer meshBuffer;
	Terrain::HeightMap heightMap;


	struct Camera
	{
		glm::vec3 target = glm::vec3(100, 0, 100);
		glm::vec3 position = glm::vec3(100, 50, 150);
		glm::vec3 up = glm::vec3(0, 1, 0);
		glm::vec3 velocity = glm::vec3(0);

		// 画面パラメータ
		float width = 1280; // 画面の幅(ピクセル数)
		float height = 720; // 画面の高さ(ピクセル数)
		float near = 1;     // 最小Z値(メートル)
		float far = 500;    // 最大Z値(メートル)

		// カメラパラメータ
		float fNumber = 1.4f; // エフ・ナンバー = カメラのF値
		float fov = glm::radians(60.0f); // フィールド・オブ・ビュー = カメラの視野角(ラジアン)
		float sensorSize = 36.0f; // フォーカル・プレーン = ピントの合う距離
		
		// Update関数で計算するパラメータ
		float focalLength = 50.0f; // フォーカル・レングス = 焦点距離(ミリ)
		float apertrue = 20.0f; // アパーチャー　= 開口(ミリ)
		float focalPlane = 10000.0f; // フォーカル・プレーン = ピントの合う距離

		void Update(const glm::mat4& matView);
	};
	Camera camera;

	EnemyActorPtr enemy;
	PlayerActorPtr player;
	
	ActorList enemies;
	ActorList objects;

	LightBuffer lightBuffer;
	ActorList lights;
	FramebufferObjectPtr fboMain;
};

#endif // !MAINGAMESCENE_H_INCLUDED

