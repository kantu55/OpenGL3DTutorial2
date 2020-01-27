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
�R���X�g���N�^
*/
EnemyActor::EnemyActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
	const glm::vec3& pos, const glm::vec3& rot)
	: SkeletalMeshActor(buffer.GetSkeletalMesh("oni_small"), "Enemy", 13, pos, rot), heightMap(hm)
{
	colLocal = Collision::CreateSphere(glm::vec3(0, 0.7f, 0), 0.7f);
	mass m;
	const glm::vec3 massStartPos = glm::vec3(leftWall, 0, backWall);
	const float rightWall = 115.0f;
	const float leftWall = 85.0f;
	const float forwordWall = 100.0f;
	const float backWall = 70.0f;
	const float mapSizeX_min = rightWall - leftWall;
	const float mapSizeZ_min = forwordWall - backWall;
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

/*
�X�V

@param deltaTime �o�ߎ���
*/
void EnemyActor::Update(float deltaTime)
{
	// target�̓v���C���[
	targetVector = targetActor->position - position; // �x�N�g��
	targetNormalize = glm::normalize(targetVector);  // �x�N�g���𐳋K��
	targetLength = glm::length(targetVector); // �x�N�g���𒷂��ɕύX�i�R���W������1.45�ȓ��͂����Ȃ��j
	targetDot = glm::dot(glm::normalize(position), glm::normalize(targetVector)); // �v���C���[�ƓG(���g)�̓���
	targetRadian = std::atan2(-targetNormalize.x, targetNormalize.z); // �v���C���[�ƓG(���g)�̊p�x
	//target = targetDot * (180 / 3.14);
	
	// ���W�̍X�V
	SkeletalMeshActor::Update(deltaTime);
	
	if (attackCollision)
	{
		attackCollision->Update(deltaTime);
	}

	// �����Ȃ��ǔ���
	if (position.x > rightWall || position.x < leftWall || position.z > forwordWall || position.z < backWall)
	{
		velocity = wait;
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
		// �X�e�[�g��ҋ@�ɋ����ύX
		state = State::wait;
		waitTimer = 2.0f;
		isAnimation = false;
		task = Task::end;
	}

	// �ڒn����
	static const float gravity = 0.0f;
	const float groundHeight = heightMap->Height(position);
	if (position.y <= groundHeight)
	{
		position.y = groundHeight;
		velocity.y = 0;
	}
	else if (position.y > groundHeight)
	{
		// ����Ă��镨�̂��痣�ꂽ��󒆔���ɂ���
		if (boardingActor)
		{
			Collision::Shape col = colWorld;
			col.s.r += 0.1f; // �Փ˔���������傫������
			glm::vec3 pa, pb;
			if (!Collision::TestShapeShape(col, boardingActor->colWorld, &pa, &pb))
			{
				boardingActor.reset();
			}
		}
		// ��������
		const bool isFloating = position.y > groundHeight + 0.1f; // �n�ʂ��畂���Ă��邩
	}

	if (health <= 0)
		return;

	// �^�X�N�̍X�V
	switch (state)
	{
	case EnemyActor::State::wait:

		if (Wait(deltaTime))
		{
			//�v���C���[�����F�ł��Ă��邩�Ń^�X�N���؂�ւ��
			if (task == Task::end)
			{
				task = Task::reserve;
				if (!SeenTo(targetLength, targetRadian))
				{
					state = State::patrol;
				}
				if (SeenTo(targetLength, targetRadian))
				{
					state = State::patrol;
				}
			}
		}
		break;

	case EnemyActor::State::round:
		if (Round())
		{
			state = State::wait;
		}
		break;

	case EnemyActor::State::patrol:
		if (Patrol())
		{
			state = State::wait;
		}
		break;

	case EnemyActor::State::approach:
		if (Approach())
		{
			state = State::attack;
		}
		break;

	case EnemyActor::State::attack:
		attackTimer += deltaTime;
		
		if (Attack(deltaTime))
		{
			state = State::wait;
		}
		break;
	}
}

/*
�ҋ@
*/
bool EnemyActor::Wait(float deltaTime)
{
	// �^�X�N�̏�����
	if (waitTimer > 0)
	{
		task = Task::start;
		//�A�j���V�����̕ύX
		if (!isAnimation)
		{
			GetMesh()->Play("Wait");
			isAnimation = true;
		}
		waitTimer -= deltaTime;
	}

	// ���E�Ƀv���C���[����������^�X�N�I��
	if (SeenTo(targetLength, targetRadian))
	{
		state = State::approach;
		task = Task::reserve;
		return false;
	}
	
	// �ҋ@���Ԃ��I��������^�X�N�I��
	if (waitTimer <= 0 && task == Task::start)
	{
		task = Task::end;
		isAnimation = false;
		return true;
	}
	return false;
}

/*
����
*/
bool EnemyActor::Round()
{
	// �^�X�N�̏�����
	if (task == Task::reserve)
	{
		roundPoint = 0;
		patrolGoalPos = roundPoints[roundPoint];
		patrolGoalPos.y = position.y;
		task = Task::start;
	}
	// �A�j���[�V�����̕ύX
	if (!isAnimation)
	{
		GetMesh()->Play("Run");
		isAnimation = true;
	}

	// ���E�Ƀv���[���[����������^�X�N�I��
	if (SeenTo(targetLength, targetRadian))
	{
		state = State::approach;
		task = Task::reserve;
		return false;
	}
	
	if (roundPoint == 3 &&
		NeraEquivalent(position, patrolGoalPos, nearGoal))
	{
		waitTimer = 2.0f;
		velocity = glm::vec3(0);
		task = Task::end;
		isAnimation = false;
		return true;
	}
	else if (NeraEquivalent(position, patrolGoalPos, nearGoal))
	{
		++roundPoint;
		patrolGoalPos = roundPoints[roundPoint];
		patrolGoalPos.y = position.y;
		return false;
	}
	// �S�[���܂ňړ�
	else
	{
		glm::vec3 move = patrolGoalPos - position;
		if (glm::dot(move, move))
		{
			// �������X�V
			move = glm::normalize(move);
			rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);
		}
		velocity = move * moveSpeed;
		return false;
	}
}

/*
����
*/
bool EnemyActor::Patrol()
{
	Astar astar;
	// �^�X�N�̏�����
	if (task == Task::reserve)
	{
		astar.open.index = 0;
		astar.close.index = 0;
		for (x = leftWall; x < ARRAY_NUM(map); x++)
		{
			for (z = backWall; z <= forwordWall; z++)
			{
				const int targetPosition_x = targetActor->position.x;
				const int targetPosition_z = targetActor->position.z;
				if (x == targetPosition_x && z == targetPosition_z)
				{
					map[x][z] = glm::vec3(targetPosition_x, position.y, targetPosition_z);
					startPos = map[x][z];
					astar.s.x = x;
					astar.s.z = z;
					// �X�^�[�g�m�[�h���I�[�v�����X�g�ɒǉ�
					astar.open.node[astar.open.index++] = &astar.s;
					// �N���[�Y���X�g�͋�ɂ���
					astar.close.node[astar.close.index++] = NULL;
					patrolGoalPos = glm::vec3(astar.s.x, position.y, astar.s.z);
				}
				const int Position_x = position.x;
				const int Position_z = position.z;
				if (x == Position_x && z == Position_z)
				{
					map[x][z] = glm::vec3(Position_x, position.y, Position_z);
					goalPos = map[x][z];
					astar.e.x = x;
					astar.e.z = z;
				}
			}
		}
		task = Task::start;
	}
	// �A�j���[�V�����̕ύX
	if (!isAnimation)
	{
		GetMesh()->Play("Run");
		isAnimation = true;
	}
	// ���E�Ƀv���[���[����������^�X�N�I��
	if (SeenTo(targetLength, targetRadian))
	{
		//state = State::approach;
		//task = Task::reserve;
		//return false;
	}
	if (nodePoint == 0)
	{
		while (1)
		{
			Astar::NODE *n = NULL;
			for (x = 0; x < astar.open.index; x++)
			{
				if (astar.open.node[x] != NULL)
				{
					int cost = astar.G(&astar.s, astar.open.node[x]);
					if (n == NULL || n->cost > cost)
					{
						glm::vec3 chackMap = glm::vec3(astar.open.node[x]->x,
							position.y, astar.open.node[x]->z);
						chackMap.y = heightMap->Height(chackMap);
						
						bool chack = false;
						int scale_x = objects->colLocal.obb.e.x;
						int scale_z = objects->colLocal.obb.e.z;
						o[0] = { objects->position + glm::vec3(-scale_x, 0, -scale_z) };
						for (int i = 0; i < (scale_x*2)+1; ++i)
						{
							if (scale_z <= 0)
							{
								o[i] = o[0] + glm::vec3(i, 0, 0);
								o[i].y = heightMap->Height(o[i]);
								if (chackMap == o[i])
								{
									chack = true;
								}
							}
							else
							{
								for (int j = 0; j < (scale_z*2)+1; j++)
								{
									o[i + j] = o[0] + glm::vec3(i, 0, j);
									o[i + j].y = heightMap->Height(o[i + j]);
									if (chackMap == o[i + j])
									{
										chack = true;
									}
								}
							}
						}
						if (!chack)
						{
							// �m�[�h�̒��ň�ԍŏ��̃R�X�g�𓾂�
							n = astar.open.node[x];
						}
						else
						{
							printf("��Q��\n");
						}						
						astar.open.node[x] = NULL;
					}
				}
			}
			// open���烊�X�g���Ȃ��Ȃ����̂ŏI������
			if (n == NULL)
			{
				printf("no goal...\n");
				return true;
			}
			map[n->x][n->z] = glm::vec3(n->x, position.y, n->z);
			//printf("%.0f , %.0f\n", map[n->x][n->z].x, map[n->x][n->z].z);

			if (map[n->x][n->z] == goalPos)
			{
				printf("OK goal!!!\n");
				printf("PlayerPos  : %.0f , %.0f\n", targetActor->position.x, targetActor->position.z);
				//printf("StartNode : %.0f , %.0f\n", startPos.x, startPos.z);
				n = n->parent;
				while (n->parent != NULL)
				{
					nodePos[nodePoint] = map[n->x][n->z];
					nodePoint++;
					printf("%.0f , %.0f\n", map[n->x][n->z].x, map[n->x][n->z].z);
					n = n->parent;
				}
				astar.s = { 0, 0, 0 };
				astar.e = { 0, 0, 0 };
				break;
			}

			// ���݂̃m�[�h���N���[�Y���X�g�Ɋi�[����
			astar.close.node[astar.close.index++] = n;

			// ���̃m�[�h������
			if (n->x >= leftWall && map[n->x - 1][n->z] != objects->position)
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x - 1, n->z, n->cost + 1));
			}

			// �E�̃m�[�h������
			if (n->x <= rightWall && map[n->x + 1][n->z] != objects->position)
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x + 1, n->z, n->cost + 1));
			}

			// �O�̃m�[�h������
			if (n->z <= forwordWall && map[n->x][n->z + 1] != objects->position)
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x, n->z + 1, n->cost + 1));
			}

			// ���̃m�[�h������
			if (n->z >= backWall && map[n->x][n->z - 1] != objects->position)
			{
				astar.SearchNode(&astar.open, &astar.close, &astar.s, &astar.e, n,
					astar.CreateNode(n->x, n->z - 1, n->cost + 1));
			}

			if (loop++ > 1000)
			{
				printf("loop error...\n");
				return false;
			}
		}
	}
	else if (moveCount < nodePoint)
	{
		
		nodePos[moveCount].y = position.y;
		glm::vec3 move = nodePos[moveCount] - position;
		if (glm::dot(move, move))
		{
			// �������X�V
			move = glm::normalize(move);
			rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);
		}
		if (NeraEquivalent(position, nodePos[moveCount], 0.5f))
		{
			nodePos[moveCount] = glm::vec3(0);
			moveCount++;
		}
		else
		{
			velocity = move * (moveSpeed);
		}
		return false;
	}
	else
	{
		waitTimer = 5.0f;
		velocity = glm::vec3(0);
		task = Task::end;
		isAnimation = false;
		moveCount = 0;
		nodePoint = 0;
		loop = 0;
		return true;
	}
	return false;
}

/*
�U��
*/
bool EnemyActor::Attack(float delatTime)
{
	task = Task::start;
	velocity = wait;

	// �A�j���[�V�����̕ύX
	if (!isAnimation)
	{
		GetMesh()->Play("Attack");
		attackTimer = 0;
		isAnimation = true;
	}

	// �R���W�������o��
	if (attackTimer > 0.05f && attackTimer < 0.6f)
	{
		if (!attackCollision)
		{
			static const float radian = 1.0f;
			const glm::vec3 front = glm::rotate(glm::mat4(1), rotation.y,
				glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1.5f, 1);
			attackCollision = std::make_shared<Actor>("PlayerAttackCollision", 5,
				position + front + glm::vec3(0, 1, 0), glm::vec3(0), glm::vec3(radian));
			attackCollision->colLocal = Collision::CreateSphere(glm::vec3(0), radian);
		}
	}
	else
	{
		attackCollision.reset();
	}

	// �A�j���[�V�������I��(1�b)������^�X�N�I��
	if (attackTimer >= 1)
	{
		attackTimer = 0;
		waitTimer = 5.0f;
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
�߂Â�
*/
bool EnemyActor::Approach()
{
	// �A�j���[�V�����̕ύX
	if (!isAnimation)
	{
		GetMesh()->Play("Run");
		isAnimation = true;
	}

	// �^�X�N�̏�����
	if (task == Task::reserve)
	{
		patrolGoalPos = targetActor->position;
		task = Task::start;
	}

	// �v���[���[�̋߂��ɍs������^�X�N�I��
	if (NeraEquivalent(position, patrolGoalPos, nearPlayer))
	{
		velocity = glm::vec3(0);
		task = Task::end;
		isAnimation = false;
		return true;
	}
	// �v���[���[�̈ʒu�܂ňړ�
	else
	{
		patrolGoalPos = targetActor->position;
		glm::vec3 move = patrolGoalPos - position;
		if (glm::dot(move, move))
		{
			// �������X�V
			move = glm::normalize(move);
			rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);
		}
		velocity = move * moveSpeed;
		return false;
	}
}

// �v���C���[�����F�ł���͈�
bool EnemyActor::SeenTo(float length, float angle)
{
	if (length <= 5.0f && angle >= -1.0f && angle <= 1.0f)
	{
		return true;
	}
	return false;
}

// �S�[�����Ɣ��f�ł���͈�
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
�Փ˃n���h��

@param b �Փˑ���̃A�N�^�[
@param p �Փ˂������������W
*/
void EnemyActor::OnHit(const ActorPtr& b, const glm::vec3& p)
{
	const glm::vec3 v = colWorld.s.center - p;
	// �Փˈʒu�Ƃ̋������߂����Ȃ������ׂ�
	if (dot(v, v) > FLT_EPSILON)
	{
		// this��b�ɏd�Ȃ�Ȃ��ʒu�܂ňړ�
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
		// �ړ���������(�������߂�����ꍇ�̗�O����)
		const float deltaTime = static_cast<float>(GLFWEW::Window::Instance().DeltaTime());
		const glm::vec3 deltaVelocity = velocity * deltaTime;
		position -= deltaVelocity;
		colWorld.s.center -= deltaVelocity;
	}
	SetBoardingActor(b);
}

/*
�G�l�~�[������Ă��镨�̂�ݒ肷��

@param p ����Ă��镨��
*/
void EnemyActor::SetBoardingActor(ActorPtr p)
{
	boardingActor = p;
}


