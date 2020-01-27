#include "LoiteringEnemy.h"

LoiteringEnemy::LoiteringEnemy(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
	const glm::vec3& pos, const glm::vec3& rot)
	:EnemyActor::EnemyActor(hm, buffer, pos, rot)
{
	colLocal = Collision::CreateSphere(glm::vec3(0, 0.7f, 0), 0.8f);
}

void LoiteringEnemy::Update(float deltaTime)
{
	EnemyActor::Update(deltaTime);
	// タスクの更新
	switch (state)
	{
	case EnemyActor::State::wait:

		if (Wait(deltaTime))
		{
			//プレイヤーが視認できているかでタスクが切り替わる
			if (task == Task::end)
			{
				if (EnemyActor::vigilanceMode)
				{
					task = Task::reserve;
					state = State::vigilance;
				}
				else
				{
					task = Task::reserve;
					state = State::patrol;
				}
			}
		}
		break;

	case EnemyActor::State::patrol:

		if (MoveTo(roundPoints[patrolCount], 0.9f))
		{
			if (task == Task::end)
			{
				if (patrolCount == 4)
				{
					state = State::wait;
					patrolCount = 0;
				}
				else
				{
					patrolCount += 1;
					task = Task::reserve;
				}
			}
		}
		break;

	case EnemyActor::State::approach:
		if (MoveTo(targetActor->position, 0.9f))
		{
			if (task == Task::end)
			{
				task = Task::reserve;
				state = State::attack;
			}
		}
		break;

	case EnemyActor::State::attack:
		attackTimer += deltaTime;
		if (Attack(deltaTime))
		{
			if (task == Task::end)
			{
				glm::vec3 v = position - targetActor->position;
				if (glm::length(v) > 8)
				{
					discovery = false;
					vigilanceMode = true;
					state = State::vigilance;
				}
				else
				{
					task = Task::reserve;
					state = State::approach;
				}
			}
		}
		break;

	case EnemyActor::State::vigilance:
		if (MoveTo(seenPos, 0.5f))
		{
			if (task == Task::end)
			{
				findRotation = rotation.y + glm::radians(90.0f);
				state = State::overlook;
			}
		}
		break;

	case EnemyActor::State::overlook:
		if (Vigilance())
		{
			if (task == Task::end)
			{
				vigilanceMode = false;
				state = State::wait;
			}
		}
		break;
	}
}

void LoiteringEnemy::OnHit(const ActorPtr& b, const glm::vec3& p)
{
	EnemyActor::OnHit(b, p);
}

/*
格納可能なアクター数を確保する

@param reserveCount アクター配列の確保数
*/
void LoiteringEnemyList::Reserve(size_t reserveCount)
{
	loiteringEnemies.reserve(reserveCount);
}

/*
アクターを追加する

@param actor 追加するアクター
*/
void LoiteringEnemyList::Add(const LoiteringEnemyPtr& actor)
{
	loiteringEnemies.push_back(actor);
}

/*
アクターを削除する

@param actor 削除するアクター
*/
bool LoiteringEnemyList::Remove(const LoiteringEnemyPtr& actor)
{
	//ポインタが一致するアクタを探して
	//見つかったら消す
	for (auto itr = loiteringEnemies.begin(); itr != loiteringEnemies.end(); ++itr)
	{
		if (*itr == actor)
		{
			loiteringEnemies.erase(itr);
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
glm::ivec2 LoiteringEnemyList::CalcMapIndex(const glm::vec3& pos) const
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
void LoiteringEnemyList::Update(float deltaTime)
{
	//範囲for文
	for (const LoiteringEnemyPtr& e : loiteringEnemies)
	{
		if (e && e->health > 0)
		{
			e->Update(deltaTime);
		}
	}

	for (auto i = loiteringEnemies.begin(); i != loiteringEnemies.end();)
	{
		const LoiteringEnemyPtr& e = *i;
		if (!e || e->health <= 0)
		{
			// erase => vectorから要素を削除するために使う
			// 実行後、次の要素を指す値を返してくれる
			i = loiteringEnemies.erase(i);
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
	for (auto i = loiteringEnemies.begin(); i != loiteringEnemies.end(); ++i)
	{
		const glm::ivec2 mapIndex = CalcMapIndex((*i)->position);
		grid[mapIndex.y][mapIndex.x].push_back(*i);
	}
}



/*
アクターを描画データを更新する
*/
void LoiteringEnemyList::UpdateDrawData(float deltaTime)
{
	for (const LoiteringEnemyPtr& e : loiteringEnemies)
	{
		if (e && e->health > 0) {
			e->UpdateDrawData(deltaTime);
		}
	}
}

/*
アクターを描画する
*/
void LoiteringEnemyList::Draw()
{
	for (const LoiteringEnemyPtr& e : loiteringEnemies)
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
std::vector<LoiteringEnemyPtr> LoiteringEnemyList::FindNearbyActors(
	const glm::vec3& pos, float maxDistance) const
{
	std::vector<std::pair<float, LoiteringEnemyPtr>> buffer;
	buffer.reserve(1000);

	const glm::ivec2 mapIndex = CalcMapIndex(pos);
	const glm::ivec2 min = glm::max(mapIndex - 1, 0);
	const glm::ivec2 max = glm::min(
		mapIndex + 1, glm::ivec2(sepalationSizeX - 1, sepalationSizeY - 1));
	for (int y = min.y; y <= max.y; ++y)
	{
		for (int x = min.x; x <= max.x; ++x)
		{
			const std::vector<LoiteringEnemyPtr>& list = grid[y][x];
			for (auto actor : list)
			{
				const float distance = glm::distance(glm::vec3(actor->position), pos);
				buffer.push_back(std::make_pair(distance, actor));
			}
		}
	}

	std::vector<LoiteringEnemyPtr> result;
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