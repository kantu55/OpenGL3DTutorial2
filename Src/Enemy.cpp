/*
@file Enemy.cpp
*/
#include "Enemy.h"
#include "PlayerActor.h"
#include "SkeletalMesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include "Astar.h"

#define ARRAY_NUM(a) (sizeof(a)/sizeof(a[0]))

/*
コンストラクタ
*/
EnemyActor::EnemyActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
	const glm::vec3& pos, const glm::vec3& rot)
	: SkeletalMeshActor(buffer.GetSkeletalMesh("oni_small"), "Enemy", 13, pos, rot), heightMap(hm)
{
	colLocal = Collision::CreateSphere(glm::vec3(0, 0.7f, 0), 0.8f);
}

void EnemyActor::TargetActor(const ActorPtr& target)
{
	targetActor = target;
	return;
}

void EnemyActor::ObstacleActor(const ActorList& obstacleis)
{
	obstacle = obstacleis;
	return;
}

void EnemyActor::ObjectActor(const StaticMeshActorPtr object)
{
	objects = object;
	return;
}

bool EnemyActor::NearPlayer()
{
	if (targetLength <= 1.5f)
	{
		return true;
	}
	return false;
}

bool EnemyActor::RayChack(glm::vec3 front, int seenLength)
{
	float objectMinScale = 10.0f;
	for (int i = 0; i < 3; i++)
	{
		float RotationY = rotation.y;
		float x = -1;
		x += i;
		bool wallFrag = false;
		front = glm::rotate(glm::mat4(1), RotationY,
			glm::vec3(0, 1, 0)) * glm::vec4(x, 0, 1, 1);
		for (int j = 0; j < seenLength; j++)
		{
			front = glm::rotate(glm::mat4(1), RotationY,
				glm::vec3(0, 1, 0)) * glm::vec4(x, 0, j, 1);
			front += position;
			front.y = heightMap->Height(front);
			for (auto& object : obstacle)
			{
				float targetDistance = glm::length(targetActor->position - front);
				const glm::vec3 distance = ObjectChack(object, front);
				if (glm::length(distance) <= 0.5f)
				{
					wallFrag = true;
				}
				else if (targetDistance < 1.0f && !wallFrag)
				{
					seenPos = targetActor->position;
					return true;
				}
			}
		}
	}
	return false;
}

void EnemyActor::PlayerInfo()
{
	// targetはプレイヤー
	targetVector = targetActor->position - position; // ベクトル
	targetNormalize = glm::normalize(targetVector);  // ベクトルを正規化
	targetLength = glm::length(targetVector); // ベクトルを長さに変更（コリジョンで1.45以内はいけない）
	targetDot = glm::dot(glm::normalize(position), glm::normalize(targetVector)); // プレイヤーと敵(自身)の内積
	targetRadian = std::atan2(-targetNormalize.x, targetNormalize.z); // プレイヤーと敵(自身)の角度
}

void EnemyActor::Outside()
{
	// 見えない壁判定
	if (position.x > rightWall || position.x < leftWall || position.z > forwordWall || position.z < backWall)
	{
		velocity = glm::vec3(0);
		if (position.x > rightWall)
		{
			position.x = rightWall;
		}
		else if (position.x < leftWall)
		{
			position.x = leftWall;
		}
		if (position.z > forwordWall)
		{
			position.z = forwordWall;
		}
		else if (position.z < backWall)
		{
			position.z = backWall;
		}
	}
}

/*
更新

@param deltaTime 経過時間
*/
void EnemyActor::Update(float deltaTime)
{
	if (health <= 0)
		return;
	PlayerInfo();

	front = glm::rotate(glm::mat4(1), rotation.y,
		glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1, 1);
	
	// 座標の更新
	SkeletalMeshActor::Update(deltaTime);
	
	if (attackCollision)
	{
		attackCollision->Update(deltaTime);
	}

	
}

/*
待機
*/
bool EnemyActor::Wait(float deltaTime)
{
	// タスクの初期化
	if (waitTimer > 0)
	{
		task = Task::start;
		//アニメションの変更
		if (!isAnimation)
		{
			GetMesh()->Play("Wait");
			isAnimation = true;
		}
		waitTimer -= deltaTime;
	}

	// 視界にプレイヤーが見えたらタスク終了
	if (RayChack(front, 6) && !vigilanceMode)
	{
		vigilanceMode = false;
		discovery = true;
		velocity = glm::vec3(0);
		state = State::vigilance;
		task = Task::reserve;
		isAnimation = false;
		return false;
	}

	if (NearPlayer())
	{
		vigilanceMode = false;
		discovery = true;
		state = State::attack;
		nodePoint = 0;
		moveCount = 0;
		isAnimation = false;
		task = Task::end;
		return false;
	}

	// 待機時間が終了したらタスク終了
	if (waitTimer <= 0 && task == Task::start)
	{
		task = Task::end;
		isAnimation = false;
		return true;
	}
	return false;
}

/*
攻撃
*/
bool EnemyActor::Attack(float delatTime)
{
	task = Task::start;
	velocity = glm::vec3(0);

	// アニメーションの変更
	if (!isAnimation)
	{
		GetMesh()->Play("Attack");
		attackTimer = 0;
		isAnimation = true;
	}
	glm::vec3 move = targetActor->position - position;
	if (glm::dot(move, move))
	{
		// 向きを更新
		move = glm::normalize(move);
		rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);
	}
	// コリジョンを出す
	if (attackTimer > 0.5f && attackTimer < 0.9f)
	{
		if (!attackCollision)
		{
			static const float radian = 0.5f;
			const glm::vec3 front = glm::rotate(glm::mat4(1), rotation.y,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1.0f, 1);
			attackCollision = std::make_shared<Actor>("PlayerAttackCollision", 5,
				position + front + glm::vec3(0, 1, 0), glm::vec3(0), glm::vec3(radian));
			attackCollision->colLocal = Collision::CreateSphere(glm::vec3(0), radian);
		}
	}
	else
	{
		attackCollision.reset();
	}

	// アニメーションが終了(1秒)したらタスク終了
	if (attackTimer >= 1)
	{
		attackTimer = 0;
		waitTimer = 1.0f;
		task = Task::end;
		isAnimation = false;
		return true;
	}
	else
	{
		return false;
	}
}

/*
移動
*/
bool EnemyActor::MoveTo(glm::vec3 target, float length)
{
	// アニメーションの変更
	if (!isAnimation)
	{
		GetMesh()->Play("Run");
		isAnimation = true;
	}
	
	Astar astar;
	// タスクの初期化
	if (task == Task::reserve)
	{
		int goalPos_x = target.x;
		int goalPos_z = target.z;
		if ((target.x - goalPos_x) >= 0.5f)
		{
			goalPos_x += 1;
		}
		if ((target.z - goalPos_z) >= 0.5f)
		{
			goalPos_z += 1;
		}

		patrolGoalPos.y = position.y;
		costChack = false;
		loop = 0;
		nodePoint = 0;
		moveCount = 0;
		astar.open.index = 0;
		astar.close.index = 0;
		for (x = leftWall; x < ARRAY_NUM(map); x++)
		{
			for (z = backWall; z <= forwordWall; z++)
			{
				// ノードのスタート地点を設定
				if (x == goalPos_x && z == goalPos_z)
				{
					map[x][z] = glm::vec3(goalPos_x, position.y, goalPos_z);
					map[x][z].y = heightMap->Height(map[x][z]);
					astar.s.x = x;
					astar.s.z = z;
					// スタートノードをオープンリストに追加
					astar.open.node[astar.open.index++] = &astar.s;
					// クローズリストは空にする
					astar.close.node[astar.close.index++] = NULL;
				}

				int position_x = position.x;
				int position_z = position.z;

				// ノードのゴール地点を設定
				if (x == position_x && z == position_z)
				{
					map[x][z] = glm::vec3(x, 0, z);
					map[x][z].y = heightMap->Height(map[x][z]);
					goalPos = map[x][z]; // ゴール地点として変数に格納
					astar.e.x = x;
					astar.e.z = z;
				}
			}
		}
		task = Task::start;
	}

	// 視界にプレイヤーが見えたらタスク終了
	if (RayChack(front, 6) && !vigilanceMode
		&& !discovery)
	{
		vigilanceMode = true;
		velocity = glm::vec3(0);
		state = State::wait;
		waitTimer = 1.0f;
		task = Task::reserve;
		isAnimation = false;
		return false;
	}
	
	if (NearPlayer())
	{
		vigilanceMode = false;
		discovery = true;
		state = State::attack;
		nodePoint = 0;
		moveCount = 0;
		isAnimation = false;
		task = Task::end;
		return false;
	}

	if (nodePoint == 0)
	{
		while (1)
		{
			Astar::NODE* n = NULL;
			n = astar.GetMinCost(n);
			// openからリストがなくなったので終了する
			if (n == NULL)
			{
				printf("no goal...\n");
				return true;
			}
			map[n->x][n->z] = glm::vec3(n->x, position.y, n->z);
			glm::vec3 v = map[n->x][n->z] - goalPos;
			// 計算中にゴール地点に着いたら計算終了
			if (dot(v,v) <= 1)
			{
				if (n != NULL && n->parent == NULL)
				{
					nodePos[nodePoint] = glm::vec3(n->x, 0, n->z);
					nodePos[nodePoint].y = heightMap->Height(nodePos[nodePoint]);
					nodePoint++;
				}

				// スタート地点のノードが無くなるまで各ノードの座標を配列に格納
				while (n->parent != NULL)
				{
					if (n->cost >= n->parent->cost)
					{
						n = n->parent;
						nodePos[nodePoint] = glm::vec3(n->x, 0, n->z);
						nodePos[nodePoint].y = heightMap->Height(nodePos[nodePoint]);
						nodePoint++;
						if (nodePoint > 55)
						{
							printf("コストが大きすぎるため再度計算\n");
							// 親ノードの空にする
							n->parent = NULL;
							costChack = true;
						}
					}

					// 親ノードのコストが大きかった場合の例外処理
					else if (n->cost < n->parent->cost)
					{
						printf("コストエラー\n");
						// 親ノードの空にする
						n->parent = NULL;
						costChack = true;
					}
				}
				n = NULL;
				astar.s = { 0, 0, 0 };
				astar.e = { 0, 0, 0 };
				break;
			}

			// 現在のノードをクローズリストに格納する
			astar.close.node[astar.close.index++] = n;

			// 左のノードを検索
			if (n->x > leftWall && MapCheck(n->x - 1, n->z))
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x - 1, n->z, n->cost + 1));
			}

			// 右のノードを検索
			if (n->x < rightWall && MapCheck(n->x + 1, n->z))
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x + 1, n->z, n->cost + 1));
			}

			// 前のノードを検索
			if (n->z < forwordWall && MapCheck(n->x, n->z + 1))
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x, n->z + 1, n->cost + 1));
			}

			// 後ろのノードを検索
			if (n->z > backWall && MapCheck(n->x, n->z - 1))
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x, n->z - 1, n->cost + 1));
			}
			// 左前のノードを検索
			if (n->x > leftWall && n->z < forwordWall && MapCheck(n->x - 1, n->z + 1))
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x - 1, n->z + 1, n->cost + 1));
			}

			// 右前のノードを検索
			if (n->x < leftWall && n->z < forwordWall && MapCheck(n->x + 1, n->z + 1))
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x + 1, n->z + 1, n->cost + 1));
			}

			// 左後ろのノードを検索
			if (n->x > leftWall && n->z > backWall && MapCheck(n->x - 1, n->z - 1))
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x - 1, n->z - 1, n->cost + 1));
			}

			// 右後ろのノードを検索
			if (n->x < leftWall && n->z > backWall && MapCheck(n->x + 1, n->z - 1))
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x + 1, n->z - 1, n->cost + 1));
			}

			// ゴールにたどり着かず、永遠にノードの計算を行った場合の例外処理
			if (loop++ > 1000)
			{
				printf("loop error...\n");
				return true;
			}
		}
	}
	// 各ノードを辿るようにしてAIを動かす
	if (moveCount < nodePoint)
	{
		nodePos[moveCount].y = position.y;
		glm::vec3 move = nodePos[moveCount] - position;
		if (glm::dot(move, move))
		{
			// 向きを更新
			move = glm::normalize(move);
			rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);
		}
		// ある一定の座標に着いたら次のノードの座標に移る
		if (NeraEquivalent(position, nodePos[moveCount], 0.5f))
		{
			moveCount++;
		}
		else
		{
			// 移動処理
			velocity = move * (moveSpeed);
		}
	}
	// 格納したノードの数の分進んだらタスク終了
	if (moveCount >= nodePoint)
	{
		// 親のコストが高かった場合の例外処理に引っかかった場合、タスクを終了せずノードの再計算を行う
		if (costChack)
		{
			velocity = glm::vec3(0);
			position = nodePos[nodePoint - 1];
			task = Task::reserve;
			return false;
		}
		if (glm::length(target - nodePos[moveCount]) > length && state == State::approach)
		{
			velocity = glm::vec3(0);
			position = nodePos[nodePoint - 1];
			task = Task::reserve;
			return false;
		}
		roundPoint = 0;
		velocity = glm::vec3(0);
		task = Task::end;
		waitTimer = 2.0f;
		isAnimation = false;
		return true;
	}
	return false;
}

bool EnemyActor::Vigilance()
{
	if (RayChack(front, 12))
	{
		vigilanceMode = false;
		discovery = true;
		rotateCount = 0;
		state = State::approach;
		task = Task::end;
		isAnimation = false;
		return false;
	}

	if (task == Task::reserve)
	{
		findRotation = rotation.y + glm::radians(90.0f);
		task = Task::start;
	}

	if (rotateCount == 3)
	{
		rotateCount = 0;
		return true;
	}
	else
	{
		if (rotateCount ==  0)
		{
			if ((findRotation - rotation.y) > FLT_EPSILON)
			{
				rotation.y += glm::radians(rotationSpeed);
			}
			else
			{
				rotateCount += 1;
				findRotation = rotation.y + glm::radians(-180.0f);
			}
		}
		else if (rotateCount == 1)
		{
			if ((findRotation - rotation.y) < FLT_EPSILON)
			{
				rotation.y -= glm::radians(rotationSpeed);
			}
			else
			{
				rotateCount += 1;
				findRotation += glm::radians(90.0f);
			}
		}
		else if (rotateCount == 2)
		{
			if ((findRotation - rotation.y) > FLT_EPSILON)
			{
				rotation.y += glm::radians(rotationSpeed);
			}
			else
			{
				rotateCount += 1;
			}
		}
	}
	return false;
}

// プレイヤーを視認できる範囲
bool EnemyActor::SeenTo(float length, float angle)
{
	if (length <= 5.0f && angle >= -1.0f && angle <= 1.0f)
	{
		return true;
	}
	return false;
}

// ゴールだと判断できる範囲
bool EnemyActor::NeraEquivalent(const glm::vec3 left, const glm::vec3 right, float acceptable)
{
	if (glm::length(right - left) <= acceptable)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
衝突ハンドラ

@param b 衝突相手のアクター
@param p 衝突が発生した座標
*/
void EnemyActor::OnHit(const ActorPtr& b, const glm::vec3& p)
{
	const glm::vec3 v = colWorld.s.center - p;
	// 衝突位置との距離が近すぎないか調べる
	if (dot(v, v) > FLT_EPSILON)
	{
		// thisをbに重ならない位置まで移動
		const glm::vec3 vn = normalize(v);
		float radiusSum = colWorld.s.r;
		switch (b->colWorld.type)
		{
		case Collision::Shape::Type::sphere:
			radiusSum += b->colWorld.s.r;
			break;
		case Collision::Shape::Type::capsule:
			radiusSum += b->colWorld.c.r;
			break;
		}
		const float distance = radiusSum - glm::length(v) + 0.01f;
		position += vn * distance;
		colWorld.s.center += vn * distance;
	}
	else
	{
		// 移動を取り消す(距離が近すぎる場合の例外処理)
		const float deltaTime = static_cast<float>(GLFWEW::Window::Instance().DeltaTime());
		const glm::vec3 deltaVelocity = velocity * deltaTime;
		position -= deltaVelocity;
		colWorld.s.center -= deltaVelocity;
	}
	SetBoardingActor(b);
}

/*
エネミーが乗っている物体を設定する

@param p 乗っている物体
*/
void EnemyActor::SetBoardingActor(ActorPtr p)
{
	boardingActor = p;
}

/*
ノードの検索先に障害物があるかチェックする

@param x 検索先のX座標
@param z 検索先のZ座標

@retrun true  障害物なし
@param false 障害物あり
*/
bool EnemyActor::MapCheck(int x, int z)
{
	glm::vec3 map = glm::vec3(x , 0, z);
	map.y = heightMap->Height(map);
	for (auto& object : obstacle)
	{
		// オブジェクトとノードの距離を取得する
		const glm::vec3 distance = ObjectChack(object, map); 

		// オブジェクトと指定した座標の距離が重なっているか確認する
		if (glm::length(distance) <= 0.8f)
		{
			return false;
		}
	}
	return true;
}

/*
障害物のオブジェクトと指定した座標の距離（ベクトル）を調べる

@param o 障害物（リスト）
@param m 指定した座標(ノード、レイ)

@return オブジェクトと指定した座標の距離（ベクトル）
*/
const glm::vec3 EnemyActor::ObjectChack(ActorPtr o, glm::vec3 m)
{
	// リストから一つのオブジェクトを取得
	StaticMeshActorPtr p = std::static_pointer_cast<StaticMeshActor>(o);
	// 指定した座標と障害物の中心点の距離(ベクトル)
	const glm::vec3 d = m - p->colWorld.obb.center;
	// 障害物の中心点の座標
	glm::vec3 q = p->colWorld.obb.center;
	// x,y,zの3軸を調べる
	for (int i = 0; i < 3; i++)
	{
		// ベクトルと各軸の内積を調べる
		float distance = dot(d, p->colWorld.obb.axis[i]);
		if (distance >= p->colWorld.obb.e[i]) {
			distance = p->colWorld.obb.e[i];
		}
		else if (distance <= -p->colWorld.obb.e[i]) {
			distance = -p->colWorld.obb.e[i];
		}
		q += distance * p->colWorld.obb.axis[i];
	}
	return m - q;
}

/*
格納可能なアクター数を確保する

@param reserveCount アクター配列の確保数
*/
void EnemyActorList::Reserve(size_t reserveCount)
{
	enemies.reserve(reserveCount);
}

/*
アクターを追加する

@param actor 追加するアクター
*/
void EnemyActorList::Add(const EnemyActorPtr& actor)
{
	enemies.push_back(actor);
}

/*
アクターを削除する

@param actor 削除するアクター
*/
bool EnemyActorList::Remove(const EnemyActorPtr& actor)
{
	//ポインタが一致するアクタを探して
	//見つかったら消す
	for (auto itr = enemies.begin(); itr != enemies.end(); ++itr)
	{
		if (*itr == actor)
		{
			enemies.erase(itr);
			return true;
		}
	}
	return false;
}

/*
指定された座標に対応する格子のインデックスを取得する

@param pos インデックスの元になる位置

@retun posに対応する格子のインデックス
*/
glm::ivec2 EnemyActorList::CalcMapIndex(const glm::vec3& pos) const
{
	const int x = std::max(0,
		std::min(sepalationSizeX - 1, static_cast<int>(pos.x / mapGridSizeX)));
	const int y = std::max(0,
		std::min(sepalationSizeY - 1, static_cast<int>(pos.z / mapGridSizeY)));
	return glm::ivec2(x, y);
}

/*
アクターの状態を更新する

@param deltaTime 前回の更新からの経過時間
*/
void EnemyActorList::Update(float deltaTime)
{
	//範囲for文
	for (const EnemyActorPtr& e : enemies)
	{
		if (e && e->health > 0)
		{
			e->Update(deltaTime);
		}
	}

	for (auto i = enemies.begin(); i != enemies.end();)
	{
		const EnemyActorPtr& e = *i;
		if (!e || e->health <= 0)
		{
			// erase => vectorから要素を削除するために使う
			// 実行後、次の要素を指す値を返してくれる
			i = enemies.erase(i);
		}
		else
		{
			++i;
		}
	}

	// 格子空間にアクターを割り当てる
	for (int y = 0; y < sepalationSizeY; ++y)
	{
		for (int x = 0; x < sepalationSizeX; ++x)
		{
			grid[y][x].clear();
		}
	}
	for (auto i = enemies.begin(); i != enemies.end(); ++i)
	{
		const glm::ivec2 mapIndex = CalcMapIndex((*i)->position);
		grid[mapIndex.y][mapIndex.x].push_back(*i);
	}
}



/*
アクターを描画データを更新する
*/
void EnemyActorList::UpdateDrawData(float deltaTime)
{
	for (const EnemyActorPtr& e : enemies)
	{
		if (e && e->health > 0) {
			e->UpdateDrawData(deltaTime);
		}
	}
}

/*
アクターを描画する
*/
void EnemyActorList::Draw()
{
	for (const EnemyActorPtr& e : enemies)
	{
		if (e && e->health > 0) {
			e->Draw();
		}
	}
}

/*
指定された座標の近傍にあるアクターのリストを取得する

@param pos         検索の基点となる座標
@param maxDistance 近傍とみなす最大距離(ｍ)

@return Actor::positionがposから半径maxDistance以内にあるアクターの配列
*/
std::vector<EnemyActorPtr> EnemyActorList::FindNearbyActors(
	const glm::vec3& pos, float maxDistance) const
{
	std::vector<std::pair<float, EnemyActorPtr>> buffer;
	buffer.reserve(1000);

	const glm::ivec2 mapIndex = CalcMapIndex(pos);
	const glm::ivec2 min = glm::max(mapIndex - 1, 0);
	const glm::ivec2 max = glm::min(
		mapIndex + 1, glm::ivec2(sepalationSizeX - 1, sepalationSizeY - 1));
	for (int y = min.y; y <= max.y; ++y)
	{
		for (int x = min.x; x <= max.x; ++x)
		{
			const std::vector<EnemyActorPtr>& list = grid[y][x];
			for (auto actor : list)
			{
				const float distance = glm::distance(glm::vec3(actor->position), pos);
				buffer.push_back(std::make_pair(distance, actor));
			}
		}
	}

	std::vector<EnemyActorPtr> result;
	result.reserve(100);
	for (const auto& e : buffer)
	{
		if (e.first <= maxDistance)
		{
			result.push_back(e.second);
		}
	}
	return result;
}