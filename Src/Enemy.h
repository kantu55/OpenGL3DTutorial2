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
	bool MoveTo(glm::vec3, float);
	bool Attack(float);
	bool Vigilance();
	bool NeraEquivalent(const glm::vec3, const glm::vec3, float);
	bool SeenTo(float, float);
	bool MapCheck(int x, int z);
	void TaskForcedTermination()
	{
		taskStart = true;
		taskEnd = true;
		isAnimation = false;
	}
	void PlayerInfo();
	void Outside();
	virtual bool RayChack(glm::vec3, int);
	virtual const glm::vec3 ObjectChack(ActorPtr, glm::vec3);
	virtual bool NearPlayer();
	
private:
	std::mt19937 rand;
	ActorPtr boardingActor;    // 乗っているアクター
	float moveSpeed = 5.0f;    // 移動速度
	ActorPtr attackCollision;  // 攻撃判定
	

	
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

	const Terrain::HeightMap* heightMap = nullptr;

	const int width = 25;
	const int height = 10;
	const float rightWall = 115.0f;
	const float leftWall = 85.0f;
	const float forwordWall = 100.0f;
	const float backWall = 70.0f;

protected:
	ActorPtr targetActor;

	// タスクのステート
	enum class Task
	{
		reserve,   // 準備
		start,       // 開始
		end,        // 終了
		irregular, // イレギュラー
	};
	Task task = Task::reserve;
	float attackTimer = 0;     // 攻撃時間

public:
	glm::vec3 map[200][200];
	glm::vec3 goalPos;
	glm::vec3 startPos;
	glm::vec3 nodePos[200];
	glm::vec3 o[100];
	int nodePoint = 0;
	int moveCount = 0;
	int maxCount = 0;
	int obstacleLength;
	int x = 0;
	int z = 0;
	int loop = 0;
	float heightMap_y;
	bool costChack = false;
	int roundPoint = 0;
	glm::vec3 front;
	float forgetTimer = 0;
	bool vigilanceMode = false;
	bool discovery = false;
	glm::vec3 seenPos;
	int rotateCount = 0;
	int patrolCount = 0;
	float findRotation;
	float rotationSpeed = 2.0f;

	// 巡回するポイント
	glm::vec3 roundPoints[5]
	{
		glm::vec3(100, 0, 93),
		glm::vec3(89, 0, 87),
		glm::vec3(100, 0, 93),
		glm::vec3(99, 0, 89),
		glm::vec3(100, 0, 93)
	};

	// アニメーションの状態
	enum class State
	{
		wait,  // 停止
		patrol,  // 巡回
		vigilance, // 警戒
		attack, //攻撃
		approach, // 近づく
		overlook, // 見渡す
	};
	State state = State::patrol; // 現在のアニメーションの状態
};
using EnemyActorPtr = std::shared_ptr<EnemyActor>;

class EnemyActorList
{
public:
	using iterator = std::vector<EnemyActorPtr>::iterator;
	using const_iterator = std::vector<EnemyActorPtr>::const_iterator;

	EnemyActorList() = default;
	~EnemyActorList() = default;

	void Reserve(size_t);
	void Add(const EnemyActorPtr&);
	bool Remove(const EnemyActorPtr&);
	void Update(float);
	void UpdateDrawData(float);
	void Draw();
	bool Empty() const { return enemies.empty(); }

	// イテレーターを取得する関数
	iterator begin() { return enemies.begin(); }
	iterator end() { return enemies.end(); }
	const_iterator begin() const { return enemies.begin(); }
	const_iterator end() const { return enemies.end(); }

	std::vector<EnemyActorPtr> FindNearbyActors(const glm::vec3& pos, float maxDistance) const;

private:
	std::vector<EnemyActorPtr> enemies;

	static const int mapGridSizeX = 10;
	static const int mapGridSizeY = 10;
	static const int sepalationSizeX = 20;
	static const int sepalationSizeY = 20;
	std::vector<EnemyActorPtr> grid[sepalationSizeY][sepalationSizeX];
	glm::ivec2 CalcMapIndex(const glm::vec3& pos) const;
};

#endif // !ENEMY_H_INCLUDED

