/*
@file Enemy.h
*/
#ifndef ENEMY_H_INCLUDED
#define ENEMY_H_INCLUDED
#include "GLFWEW.h"
#include "SkeletalMeshActor.h"
#include "Terrain.h"
#include<memory>
#include <random>
#include <stdio.h>
#include <tchar.h>
#include <map>



/*
エネミーアクター
*/
class EnemyActor : public SkeletalMeshActor
{
public:
	EnemyActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
		const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0));
	virtual ~EnemyActor() = default;

	virtual void Update(float) override;
	virtual void OnHit(const ActorPtr&, const glm::vec3&);
	void SetBoardingActor(ActorPtr);
	const ActorPtr& GetAttackCollision() const { return attackCollision; }

	float horizontalSpeed = velocity.x * velocity.x + velocity.z * velocity.z;
	void TargetActor(const ActorPtr& target);
	void ObjectActor(const StaticMeshActorPtr object);
	void ObstacleActor(const ActorList& obstacleis);

	bool Wait(float);
	bool Patrol();
	bool Round();
	bool Alert();
	bool Approach();
	bool Attack(float);
	bool NeraEquivalent(const glm::vec3, const glm::vec3, float);
	bool SeenTo(float, float);
	bool NoticeTo(float, float);
	void TaskForcedTermination()
	{
		taskStart = true;
		taskEnd = true;
		isAnimation = false;
	}
	
private:
	void CheckRun();
	void CheckAttack();

	
	std::mt19937 rand;
	ActorPtr boardingActor;    // 乗っているアクター
	float moveSpeed = 5.0f;    // 移動速度
	ActorPtr attackCollision;  // 攻撃判定
	float attackTimer = 0;     // 攻撃時間

	ActorPtr targetActor;
	ActorList obstacle;
	ActorPtr objects;

	glm::vec3 forward;
	glm::vec3 targetVector;
	glm::vec3 targetNormalize;
	float targetLength;
	float targetDot;
	float targetRadian;
	float myDot;

	bool taskStart = false;
	bool taskEnd = false;
	float waitTimer = 0;

	bool isAnimation = false;

	float nearPlayer = 1.5f;
	float nearGoal = 0.5f;

	glm::vec3 patrolGoalPos;
	float patrolX;
	float patrolZ;

	glm::vec3 wait = glm::vec3(0);

	const Terrain::HeightMap* heightMap = nullptr;

	const int width = 25;
	const int height = 10;

	const float rightWall = 115.0f;
	const float leftWall = 85.0f;
	const float forwordWall = 100.0f;
	const float backWall = 70.0f;

public:
	int obstacleLength;
	glm::vec3 map[200][200];
	glm::vec3 goalPos;
	glm::vec3 startPos;
	glm::vec3 nodePos[200];
	int nodePoint = 0;
	int moveCount = 0;
	float heightMap_y;
	int x = 0;
	int z = 0;
	int loop = 0;
	glm::vec3 o[100];

	const glm::vec3 roundPoints[4]
	{
		glm::vec3(110, 0, 80),
		glm::vec3(100, 0, 75),
		glm::vec3(90, 0, 80),
		glm::vec3(100, 0, 85),
	};
	int roundPoint;

	// アニメーションの状態
	enum class State
	{
		wait, // 停止
		patrol,  // 巡回(ランダム)
		round, // 巡回(決められてる)
		attack, //攻撃
		approach, // 近づく
		alert, // 警報（周囲に知らせる）
	};
	State state = State::patrol; // 現在のアニメーションの状態

	// タスクのステート
	enum class Task
	{
		reserve, //準備
		start,     //開始
		end,       //終了
	};
	Task task = Task::reserve;
	
	typedef struct
	{
		int x;
		int y;
		int px;
		int py;
		int cost;
	} anode;

	typedef struct
	{
		glm::vec3 massPos = glm::vec3(0);
		glm::vec3 size = glm::vec3(1, 0, 1);
		int x;
		int y;
		int noSize = 0;
		int No = 0;
	} mass[900];
	int mapSize = 0;
	
	bool MapInitilize = false;
	
	std::map<int, anode> mapOpen;
	std::map<int, anode> mapClose;
};
using EnemyActorPtr = std::shared_ptr<EnemyActor>;

#endif // !ENEMY_H_INCLUDED

