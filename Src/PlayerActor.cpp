/*
@file PlayerActor.cpp
*/
#include "PlayerActor.h"
#include "SkeletalMesh.h"
#include <glm/gtc/matrix_transform.hpp>

/*
�R���X�g���N�^
*/
PlayerActor::PlayerActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
	const glm::vec3& pos, const glm::vec3& rot)
	: SkeletalMeshActor(buffer.GetSkeletalMesh("Bikuni"), "Player", 13, pos, rot), heightMap(hm)
{
	colLocal = Collision::CreateSphere(glm::vec3(0, 0.7f, 0), 0.7f);
	GetMesh()->Play("Idle");
	state = State::idle;
}

/*
�X�V

@param deltaTime �o�ߎ���
*/
void PlayerActor::Update(float deltaTime)
{
	// ���W�̍X�V
	SkeletalMeshActor::Update(deltaTime);
	if (attackCollision)
	{
		attackCollision->Update(deltaTime);
	}

	// �����Ȃ��ǔ���
	const float rightWall = 115.0f;
	const float leftWall = 85.0f;
	const float forwordWall = 100.0f;
	const float backWall = 70.0f;
	if (position.x >= rightWall)
	{
		position.x = rightWall;
	}
	else if (position.x <= leftWall)
	{
		position.x = leftWall;
	}
	if (position.z >= forwordWall)
	{
		position.z = forwordWall;
	}
	else if (position.z <= backWall)
	{
		position.z = backWall;
	}

	// �ڒn����
	static const float gravity = 9.8f;
	const float groundHeight = heightMap->Height(position);
	if (position.y <= groundHeight)
	{
		position.y = groundHeight;
		velocity.y = 0;
		isInAir = false;
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
		if (!isInAir && isFloating && !boardingActor)
		{
			isInAir = true;
		}

		//�d�͂�������
		if (isInAir)
		{
			velocity.y -= gravity * deltaTime;
		}
	}

	// �A�j���[�V�����̍X�V
	switch (state)
	{
	case PlayerActor::State::idle:
		if (isInAir)
		{
			GetMesh()->Play("Jump");
			state = State::jump;
		}
		else
		{
			const float horizontalSpeed = velocity.x * velocity.x + velocity.z * velocity.z;
			if (horizontalSpeed != 0)
			{
				GetMesh()->Play("Run");
				state = State::run;
			}
		}
		break;

	case PlayerActor::State::run:
		if (isInAir)
		{
			GetMesh()->Play("Jump");
			state = State::jump;
		}
		else
		{
			const float horizontalSpeed = velocity.x * velocity.x + velocity.z * velocity.z;
			if (horizontalSpeed == 0)
			{
				GetMesh()->Play("Idle");
				state = State::idle;
			}
		}
		break;

	case PlayerActor::State::jump:
		if (!isInAir)
		{
			GetMesh()->Play("Idle");
			state = State::idle;
		}
		break;

	case PlayerActor::State::attack:
		attackTimer += deltaTime;
		if (attackTimer > 0.05 && attackTimer < 0.6f)
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
		if (GetMesh()->IsFinished())
		{
			attackCollision.reset();
			GetMesh()->Play("Idle");
			state = State::idle;
		}
		break;
	}
}

/*
�Փ˃n���h��

@param b �Փˑ���̃A�N�^�[
@param p �Փ˂������������W
*/
void PlayerActor::OnHit(const ActorPtr& b, const glm::vec3& p)
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
�W�����v������
*/
void PlayerActor::Jump()
{
	velocity.y = 5.0f;
	boardingActor.reset();
	isInAir = true;
}

/*
���͂���������
*/
void PlayerActor::ProcessInput()
{
	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
	CheckRun(gamepad);
	CheckJump(gamepad);
	CheckAttack(gamepad);
}



/*
�v���C���[������Ă��镨�̂�ݒ肷��

@param p ����Ă��镨��
*/
void PlayerActor::SetBoardingActor(ActorPtr p)
{
	boardingActor = p;
	if (p)
	{
		isInAir = false;
	}
}

/*
�ړ��������������

@param gamepad �Q�[���p�b�h����
*/
void PlayerActor::CheckRun(const GamePad& gamepad)
{
	// �󒆂ɂ���Ƃ��͈ړ��ł��Ȃ�
	if (isInAir)
	{
		return;
	}

	// �����L�[����ړ��������v�Z
	const glm::vec3 front(0, 0, -1);
	const glm::vec3 left(-1, 0, 0);
	glm::vec3 move(0);
	if (gamepad.buttons & GamePad::DPAD_UP)
	{
		move += front;
	}
	else if (gamepad.buttons & GamePad::DPAD_DOWN)
	{
		move -= front;
	}
	if (gamepad.buttons & GamePad::DPAD_LEFT)
	{
		move += left;
	}
	else if (gamepad.buttons & GamePad::DPAD_RIGHT)
	{
		move -= left;
	}

	// �ړ����s���Ă�����A�ړ������ɉ����Č����Ƒ��x���X�V
	if (glm::dot(move, move))
	{
		// �������X�V
		move = glm::normalize(move);
		rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);

		// ���̂ɏ���Ă��Ȃ��Ƃ��͒n�`�̌��z���l�����Ĉړ������𒲐�����
		if (!boardingActor)
		{
			// �ړ������̒n�`�̌��z(radient)���v�Z
			const float minGradient = glm::radians(-60.0f); // �������Ƃ̂ł�����z�̍ŏ��l
			const float maxGradient = glm::radians(60.0f);  // �������Ƃ̂ł�����z�̍ő�l
			const float frontY =
				heightMap->Height(position + move * 0.05f) - position.y - 0.01f;
			const float grasient =
				glm::clamp(std::atan2(frontY, 0.05f), minGradient, maxGradient);

			//�n�`�ɉ����悤�Ɉړ����x��ݒ�
			const glm::vec3 axis = glm::normalize(glm::cross(move, glm::vec3(0, 1, 0)));
			move = glm::rotate(glm::mat4(1), grasient, axis) * glm::vec4(move, 1.0f);
		}
		velocity = move * moveSpeed;
	}
	else
	{
		//�ړ����Ă��Ȃ��̂ő��x��0�ɂ���
		velocity = glm::vec3(0);
	}
}

/*
�W�����v�������������

@param gamepad �Q�[���p�b�h����
*/
void PlayerActor::CheckJump(const GamePad& gamepad)
{
	if (isInAir)
	{
		return;
	}
	if (gamepad.buttonDown & GamePad::B)
	{
		Jump();
	}
}

/*
�U���������������

@param gamepad �Q�[���p�b�h����
*/
void PlayerActor::CheckAttack(const GamePad& gamepad)
{
	if (isInAir)
	{
		return;
	}
	if (gamepad.buttonDown & GamePad::A)
	{
		GetMesh()->Play("Attack.Light", false);
		attackTimer = 0;
		state = State::attack;
	}
}