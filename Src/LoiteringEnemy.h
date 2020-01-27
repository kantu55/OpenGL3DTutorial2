#ifndef LOITERINGENEMY_H_INCLUDED
#define LOITERINGENEMY_H_INCLUDED
#include "Enemy.h"

class LoiteringEnemy : public EnemyActor
{
public:
	LoiteringEnemy(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
		const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0));
	virtual ~LoiteringEnemy() = default;
	virtual void Update(float) override;
	virtual void OnHit(const ActorPtr&, const glm::vec3&);
	
};
using LoiteringEnemyPtr = std::shared_ptr<LoiteringEnemy>;

class LoiteringEnemyList
{
public:
	using iterator = std::vector<LoiteringEnemyPtr>::iterator;
	using const_iterator = std::vector<LoiteringEnemyPtr>::const_iterator;

	LoiteringEnemyList() = default;
	~LoiteringEnemyList() = default;

	void Reserve(size_t);
	void Add(const LoiteringEnemyPtr&);
	bool Remove(const LoiteringEnemyPtr&);
	void Update(float);
	void UpdateDrawData(float);
	void Draw();
	bool Empty() const { return loiteringEnemies.empty(); }

	// イテレーターを取得する関数
	iterator begin() { return loiteringEnemies.begin(); }
	iterator end() { return loiteringEnemies.end(); }
	const_iterator begin() const { return loiteringEnemies.begin(); }
	const_iterator end() const { return loiteringEnemies.end(); }

	std::vector<LoiteringEnemyPtr> FindNearbyActors(const glm::vec3& pos, float maxDistance) const;

private:
	std::vector<LoiteringEnemyPtr> loiteringEnemies;

	static const int mapGridSizeX = 10;
	static const int mapGridSizeY = 10;
	static const int sepalationSizeX = 20;
	static const int sepalationSizeY = 20;
	std::vector<LoiteringEnemyPtr> grid[sepalationSizeY][sepalationSizeX];
	glm::ivec2 CalcMapIndex(const glm::vec3& pos) const;
};

#endif LOITERINGENEMY_H_INCLUDED