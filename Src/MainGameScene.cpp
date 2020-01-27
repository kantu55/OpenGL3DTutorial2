/*
@file MainGameScene.cpp
*/
#include "GLFWEW.h"
#include "MainGameScene.h"
#include "StatusScene.h"
#include "GameOverScene.h"
#include "SkeletalMeshActor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <random>

/*
�Փ˂���������

@param a �Փ˂����A�N�^�[����1
@param b �Փ˂����A�N�^�[����2
@param p �Փˈʒu
*/
void PlayerCollisionHandler(const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
{
	const glm::vec3 v = a->colWorld.s.center - p;
	// �Փˈʒu�Ƌ������߂����Ȃ������ׂ�
	if (dot(v, v) > FLT_EPSILON)
	{
		// a��b���d�Ȃ�Ȃ��ʒu�܂ňړ�
		const glm::vec3 vn = normalize(v);
		float radiusSum = a->colWorld.s.r;
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
		a->position += vn * distance;
		a->colWorld.s.center += vn * distance;
		//�v���C���[���Փ˔���`��̏�ɏ������Y�̈ړ��x�N�g����0�ɂ���
		//����60�x�͈͓̔��������画�肷��
		if (a->velocity.y < 0 && vn.y >= glm::cos(glm::radians(60.0f)))
		{
			a->velocity.y = 0;
		}
	}
	else
	{
		// �ړ���������(�������߂�����ꍇ�̗�O����)
		const float deltaTime = static_cast<float>(GLFWEW::Window::Instance().DeltaTime());
		const glm::vec3 deltaVelocity = a->velocity * deltaTime;
		a->position -= deltaVelocity;
		a->colWorld.s.center -= deltaVelocity;
	}
}

bool MainGameScene::CameraRayChack(glm::mat4 front, int z)
{
	for (int i = 0; i < 5; i++)
	{
		glm::vec3 a;
		if (z == -2)
		{
			glm::vec3 ray = front * glm::vec4(0, 0, i + z, 1);
			a = camera.position + ray;
		}
		else
		{
			glm::vec3 ray = front * glm::vec4(0, 0, i, 1);
			a = camera.position + ray;
		}
		a.y = heightMap.Height(a);
		float length = glm::length(player->position - a);
		for (auto& object : objects)
		{
			glm::vec3 distance = enemy->ObjectChack(object, a);
			if (glm::length(distance) < FLT_EPSILON)
			{
				return true;
			}
		}
	}
	return false;
}

StaticMeshActorPtr MainGameScene::CreateTreeWall(glm::vec3 pos, int axsis, int size)
{
	if (axsis == 0)
	{
		const Mesh::FilePtr meshTree = meshBuffer.GetFile("Res/red_pine_tree.gltf");
		glm::vec3 position = glm::vec3(pos.x + size, pos.y, pos.z);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
			meshTree, "Tree", 100, position, glm::vec3(0));
		p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
			glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(1));
		p->scale = glm::vec3(1.5f);
		return p;
	}
	if (axsis == 1)
	{
		const Mesh::FilePtr meshTree = meshBuffer.GetFile("Res/red_pine_tree.gltf");
		glm::vec3 position = glm::vec3(pos.x, pos.y, pos.z + size);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
			meshTree, "Tree", 100, position, glm::vec3(0));
		p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
			glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(1));
		p->scale = glm::vec3(1.5f);
		return p;
	}
	return NULL;
}


StaticMeshActorPtr MainGameScene::CreateStoneWall(glm::vec3 pos, float rot, int axsis, int size)
{
	size *= 4;
	if (axsis == 0)
	{
		const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/wall_stone.gltf");
		glm::vec3 position = glm::vec3(pos.x + size, 0, pos.z);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
			meshStoneWall, "StoneWall", 100, position, glm::vec3(0));
		p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
			glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(2, 2, 0.5f));
		p->rotation = glm::vec3(0, glm::radians(rot), 0);
		return p;
	}
	if (axsis == 1)
	{
		const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/wall_stone.gltf");
		glm::vec3 position = glm::vec3(pos.x, 0, pos.z + size);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
			meshStoneWall, "StoneWall", 100, position, glm::vec3(0));
		p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
			glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(0.5f, 2, 2));
		p->rotation = glm::vec3(0, glm::radians(rot), 0);
		return p;
	}
	return NULL;
}

bool MainGameScene::Initialize()
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);
	Sprite spr(Texture::Image2D::Create("Res/Sora.tga"));
	spr.Position(glm::vec3(0, 0, 500.0f));
	spr.Scale(glm::vec2(2));
	sprites.push_back(spr);
	map.Texture(Texture::Image2D::Create("Res/Map.tga"));
	map.Position(glm::vec3(750, 325, 0.0f));
	map.Scale(glm::vec2(1.5f, 1.725f));
	sprites.push_back(map);
	sprPlayer.Texture(Texture::Image2D::Create("Res/Player.tga"));
	sprPlayer.Scale(glm::vec2(0.2f));
	sprSikaku.Texture(Texture::Image2D::Create("Res/Sikaku.tga"));
	sprSikaku.Scale(glm::vec2(0.25f, 0.5f));
	sprWall.Texture(Texture::Image2D::Create("Res/wall.tga"));
	sprWall.Scale(glm::vec2(1,1));
	sprJizo.Texture(Texture::Image2D::Create("Res/Jizo.tga"));
	sprJizo.Scale(glm::vec2(0.05f));
	sprGoal.Texture(Texture::Image2D::Create("Res/Goal.tga"));
	sprGoal.Scale(glm::vec2(0.05f));
	sprGoal.Position(glm::vec3(870, 490, 0));
	sprites.push_back(sprGoal);
	sprDiscovery.Texture(Texture::Image2D::Create("Res/Hakken.tga"));
	sprDiscovery.Scale(glm::vec2(0.1f));
	sprVigilance.Texture(Texture::Image2D::Create("Res/Keikai.tga"));
	sprVigilance.Scale(glm::vec2(0.1f));

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");
	fntJizo.Init(1000);
	fntJizo.LoadFromFile("Res/font.fnt");

	meshBuffer.Init(1'000'000 * sizeof(Mesh::Vertex), 3'000'000 * sizeof(GLushort));

	meshBuffer.LoadMesh("Res/red_pine_tree.gltf");
	meshBuffer.LoadMesh("Res/wall_stone.gltf");
	meshBuffer.LoadMesh("Res/jizo_statue.gltf");
	meshBuffer.LoadSkeletalMesh("Res/bikuni.gltf");
	meshBuffer.LoadSkeletalMesh("Res/oni_small.gltf");

	// FBO���쐬����
	/*
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	fboMain = FramebufferObject::Create(window.Width(), window.Height());
	Mesh::FilePtr rt = meshBuffer.AddPlane("RenderTarget");
	if (rt)
	{
		rt->material[0].program = Shader::Program::Create(
			"Res/DepthOfField.vert", "Res/DepthOfField.frag");
		rt->material[0].texture[0] = fboMain->GetColorTexture();
		rt->material[0].texture[1] = fboMain->GetDepthTexture();
	}
	if (!rt || !rt->material[0].program)
	{
		return false;
	}
	*/

	//�n�C�g�}�b�v���쐬����
	if (!heightMap.LoadFromFile("Res/Terrain.tga", 20.0f, 0.5f))
	{
		return false;
	}
	if (!heightMap.CreateMesh(meshBuffer, "Terrain"))
	{
		return false;
	}
	/*
	// ���ʂ̍����͗v����
	if (!heightMap.CreaetWaterMesh(meshBuffer, "Water", 8))
	{
		return false;
	}
	*/
	lightBuffer.Init(1);
	lightBuffer.BindToShader(meshBuffer.GetStaticMeshShader());
	lightBuffer.BindToShader(meshBuffer.GetTerrainShader());
	/*
	lightBuffer.BindToShader(meshBuffer.GetWaterShader());
	*/
	glm::vec3 startPos(89, 0, 97);
	startPos.y = heightMap.Height(startPos);
	player = std::make_shared<PlayerActor>( &heightMap, meshBuffer, startPos);
	cameraRadius = glm::radians(180.0f);
	cameraRotate = player->rotation.y;
	
	objects.Reserve(500);

	rand.seed(0);

	// ���n����z�u
	{
		glm::vec3 position(97, 0, 85);
		position.y = heightMap.Height(position);
		glm::vec3 rotation(0);
		rotation.y = std::uniform_real_distribution<float>(0.0f, 3.14f * 2.0f)(rand);
		JizoActorPtr p = std::make_shared<JizoActor>(
			meshBuffer.GetFile("Res/jizo_statue.gltf"), position, 0, this);
		p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
			glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(1, 1, 0.5f));
		p->scale = glm::vec3(3);  // �����₷���悤�Ɋg��
		objects.Add(p);
		sprJizo.Position(glm::vec3(740,320,0));
		sprites.push_back(sprJizo);
	}

	// �O�ǂ�z�u
	{
		size_t outerWallCount = 5;
		const size_t wallDirectoin = 1;
		objects.Reserve(outerWallCount);
		float sprSlide = 0;
		for (size_t east = 0; east < outerWallCount; ++east)
		{
			glm::vec3 position = glm::vec3(88.5f, 0, 70.5f);
			position.y = heightMap.Height(position);
			StaticMeshActorPtr p = CreateStoneWall(position, 0, x, east);
			objects.Add(p);
			sprWall.Position(glm::vec3(550, 425.5f, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}

		
		sprSlide = 0;
		for (size_t west = 0; west < outerWallCount; ++west)
		{
			glm::vec3 position = glm::vec3(92.5f, 0, 99.0f);
			position.y = heightMap.Height(position);
			StaticMeshActorPtr p = CreateStoneWall(position, 0, x, west);
			objects.Add(p);
			sprWall.Position(glm::vec3(590, 60, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}

		sprSlide = 0;
		outerWallCount = 7;
		for (size_t north = 0; north < outerWallCount; ++north)
		{
			glm::vec3 position = glm::vec3(87.0f, 0, 72.5f);
			position.y = heightMap.Height(position);
			StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, north);
			objects.Add(p);
			sprWall.Position(glm::vec3(535, 110, 0.0f) + glm::vec3(p->position.x, p->position.z + sprSlide, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
		sprSlide = 0;
		for (size_t south = 0; south < outerWallCount; ++south)
		{
			glm::vec3 position = glm::vec3(110.0f, 0, 72.5f);
			position.y = heightMap.Height(position);
			StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, south);
			objects.Add(p);
			sprWall.Position(glm::vec3(785, 110, 0.0f) + glm::vec3(p->position.x, p->position.z + sprSlide, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
	}

	// ����(���p�|�C���g)�̔z�u
	{
		const size_t innerWallCount = 1;
		const size_t wallDirectoin = 1;
		objects.Reserve(innerWallCount);
		float sprSlide = 0;
		for (size_t east = 0; east < innerWallCount; ++east)
		{
			glm::vec3 position = glm::vec3(96.5f, 0, 83.5f);
			position.y = heightMap.Height(position);
			StaticMeshActorPtr p = CreateStoneWall(position, 0.0f, x, east);
			objects.Add(p);
			sprWall.Position(glm::vec3(640, 260, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}

		sprSlide = 0;
		for (size_t west = 0; west < innerWallCount; ++west)
		{
			glm::vec3 position = glm::vec3(100.5f, 0, 87.5f);
			position.y = heightMap.Height(position);
			StaticMeshActorPtr p = CreateStoneWall(position, 0.0f, x, west);
			objects.Add(p);
			sprWall.Position(glm::vec3(680, 205, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
		sprSlide = 0;
		for (size_t north = 0; north < innerWallCount; ++north)
		{
			glm::vec3 position = glm::vec3(95.0f, 0, 86.0f);
			position.y = heightMap.Height(position);
			StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, north);
			objects.Add(p);
			sprWall.Position(glm::vec3(625, 230, 0.0f) + glm::vec3(p->position.x, p->position.z + sprSlide, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
		sprSlide = 0;
		for (size_t south = 0; south < innerWallCount; ++south)
		{
			glm::vec3 position = glm::vec3(102.0f, 0, 85.0f);
			position.y = heightMap.Height(position);
			StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, south);
			objects.Add(p);
			sprWall.Position(glm::vec3(695, 235, 0.0f) + glm::vec3(p->position.x, p->position.z + sprSlide, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
	}

	// �O���ƌ㔼�𕪂�����ǔz�u(��)
	{
		size_t left = 2;
		size_t right = 2;
		float sprSlide = 0;
		glm::vec3 position = glm::vec3(89.0f, 0, 85.0f);
		position.y = heightMap.Height(position);
		for (size_t i = 0; i < left; i++)
		{
			StaticMeshActorPtr p = CreateStoneWall(position, 0, x, i);
			objects.Add(p);
			sprWall.Position(glm::vec3(555, 240, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
		position = glm::vec3(104.0f, 0, 85.0f);
		position.y = heightMap.Height(position);
		sprSlide = 0;
		for (size_t j = 0; j < right; j++)
		{
			StaticMeshActorPtr p = CreateStoneWall(position, 0, x, j);
			objects.Add(p);
			sprWall.Position(glm::vec3(720, 240, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
	}

	// �O���̓��ǔz�u(��)
	{
		float sprSlide = 0;
		glm::vec3 position = glm::vec3(96.5f, 0, 91.5f);
		position.y = heightMap.Height(position);
		for (int i = 0; i < 3; i++)
		{
			StaticMeshActorPtr p = CreateStoneWall(position, 0, x, i);
			objects.Add(p);
			sprWall.Position(glm::vec3(640, 155, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
		position = glm::vec3(106, 0, 90);
		position.y = heightMap.Height(position);
		sprSlide = 0;
		{
			StaticMeshActorPtr p = CreateStoneWall(position, 90, z, 0);
			objects.Add(p);
			sprWall.Position(glm::vec3(740, 180, 0.0f) + glm::vec3(p->position.x, p->position.z + sprSlide, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
		
		position = glm::vec3(91, 0,90);
		position.y = heightMap.Height(position);
		sprSlide = 0;
		{
			StaticMeshActorPtr p = CreateStoneWall(position, 90, z, 0);
			objects.Add(p);
			sprWall.Position(glm::vec3(575, 180, 0.0f) + glm::vec3(p->position.x, p->position.z + sprSlide, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
		position = glm::vec3(95, 0, 90);
		position.y = heightMap.Height(position);
		sprSlide = 0;
		{
			StaticMeshActorPtr p = CreateStoneWall(position, 90, z, 0);
			objects.Add(p);
			sprWall.Position(glm::vec3(625, 180, 0.0f) + glm::vec3(p->position.x, p->position.z + sprSlide, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
		
		position = glm::vec3(89, 0, 95);
		position.y = heightMap.Height(position);
		sprSlide = 0;
		for (int i = 0; i < 3; i++)
		{
			StaticMeshActorPtr p = CreateStoneWall(position, 0, x, i);
			objects.Add(p);
			sprWall.Position(glm::vec3(550, 110, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}

		position = glm::vec3(104, 0, 95);
		position.y = heightMap.Height(position);
		sprSlide = 0;
		for (int i = 0; i < 2; i++)
		{
			StaticMeshActorPtr p = CreateStoneWall(position, 0, x, i);
			objects.Add(p);
			sprWall.Position(glm::vec3(725, 110, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
			sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
			sprites.push_back(sprWall);
			sprSlide += 42.5f;
		}
	}

	// �㔼�̓��ǂ̔z�u
	{
		glm::vec3 position = glm::vec3(106.0f, 0, 72.0f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(650, 325, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}
	
	{
		glm::vec3 position = glm::vec3(106.0f, 0, 75.5f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(650, 350, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}
	
	{
		glm::vec3 position = glm::vec3(92.5f, 0, 80.5f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 0.0f, x, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(590, 300, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}
	
	float sprSlide = 0;
	for(int i = 0; i < 2; i++)
	{
		glm::vec3 position = glm::vec3(100.5f, 0, 80.5f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 0.0f, x, i);
		objects.Add(p);
		sprWall.Position(glm::vec3(680, 300, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
		sprSlide += 42.5f;
	}
	
	{
		glm::vec3 position = glm::vec3(92.5f, 0, 74.5f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 0.0f, x, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(590, 370, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}
	
	{
		glm::vec3 position = glm::vec3(100.0f, 0, 74.0f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 0.0f, x, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(672.5f, 375, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}

	sprSlide = 0;
	for (size_t i = 0; i < 2; ++i)
	{
		glm::vec3 position = glm::vec3(96.5f, 0, 77.5f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 0.0f, x, i);
		objects.Add(p);
		sprWall.Position(glm::vec3(635, 335, 0.0f) + glm::vec3(p->position.x + sprSlide, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
		sprSlide += 42.5f;
	}
	
	{
		glm::vec3 position = glm::vec3(91.0f, 0, 79.0f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(567.5f, 317.5f, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}
	
	{
		glm::vec3 position = glm::vec3(98.5f, 0, 76.0f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(745, 400, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}
	
	{
		glm::vec3 position = glm::vec3(98.5f, 0, 79.0f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(745, 350, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}

	{
		glm::vec3 position = glm::vec3(94.5f, 0, 76.0f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(610, 352.5f, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}
	
	{
		glm::vec3 position = glm::vec3(102.0f, 0, 82.0f);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = CreateStoneWall(position, 90.0f, z, 0);
		objects.Add(p);
		sprWall.Position(glm::vec3(695, 280, 0.0f) + glm::vec3(p->position.x, p->position.z, 0));
		sprWall.Rotation(p->rotation.y + glm::radians(90.0f));
		sprites.push_back(sprWall);
	}


	// ���C�g�̔z�u
	lights.Add(std::make_shared<DirectionalLightActor>(
		"DirectionalLight", glm::vec3(1.0f), glm::normalize(glm::vec3(1, -2, -1))));

	glm::vec3 color(5, 0.8f, 0.5f);
	glm::vec3 position(0);
	glm::vec3 direction(0, 5, 0);

	// �G��z�u
	{
		/*
		const size_t oniCount = 1;
		enemies.Reserve(5);
		for (size_t i = 0; i < oniCount; ++i)
		{
			EnemyActorPtr p;
			p = std::make_shared<EnemyActor>(&heightMap, meshBuffer, glm::vec3(97, 0, 89));
			p->position.y = heightMap.Height(p->position);
			p->ObstacleActor(objects);
			sprEnemy.Texture(Texture::Image2D::Create("Res/Oni.tga"));
			sprEnemy.Scale(glm::vec2(0.03f));
			enemies.Add(p);
		}
		*/
		{
			const size_t oniCount = 1;
			L_enemies.Reserve(5);
			for (size_t i = 0; i < oniCount; ++i)
			{
				LoiteringEnemyPtr p;
				p = std::make_shared<LoiteringEnemy>(&heightMap, meshBuffer, glm::vec3(97, 0, 89));
				p->position.y = heightMap.Height(p->position);
				p->ObstacleActor(objects);
				sprEnemy.Texture(Texture::Image2D::Create("Res/Oni.tga"));
				sprEnemy.Scale(glm::vec2(0.03f));
				enemies.Add(p);
			}
		}
		lights.Update(0);
		lightBuffer.Update(lights, glm::vec3(0.1f, 0.05f, 0.15f));
		heightMap.UpdateLightIndex(lights);
		
#if 0
		for (size_t i = 0; i < oniCount; ++i)
		{
			//�G�̈ʒu��(50,50)-(150,150)�͈̔͂��烉���_���ɑI��
			glm::vec3 position(0);
			position.x = std::uniform_real_distribution<float>(85, 115)(rand);
			position.z = std::uniform_real_distribution<float>(70, 100)(rand);
			position.y = heightMap.Height(position);
			//�G�̌����������_���ɑI��
			glm::vec3 rotation(0);
			rotation.y = std::uniform_real_distribution<float>(0, glm::pi<float>() * 2)(rand);
			const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
			SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
				mesh, "Kooni", 13, position, rotation);
			p->GetMesh()->Play("Wait");

			p->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
			enemies.Add(p);
		}
#endif
	}
	window.CenterCursor();
	return true;
}
/*
�v���C���[�̓��͂���������
*/
void MainGameScene::ProcessInput()
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();

	player->ProcessInput();

	if (window.GetGamePad().buttonDown & GamePad::START)
	{
		if (state == GameState::play)
		{
			if (c_state == CameraState::playcamera)
			{
				c_state = CameraState::debug;
			}
			else if (c_state == CameraState::debug)
			{
				c_state = CameraState::playcamera;
				camera.target = player->position + glm::vec3(0, 0, -5);
				camera.position = player->position + glm::vec3(0, 4, 5);
			}
		}
		else if (state == GameState::clear || state == GameState::over)
		{
			SceneStack::Instance().Replace(std::make_shared<StatusScene>());
		}	
	}

}

/*
�V�[�����X�V����

@param deltaTime �O��̍X�V����̌o�ߎ��ԁi�b�j
*/
void MainGameScene::Update(float deltaTime)
{
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const float w = window.Width();
	const float h = window.Height();
	const float lineHenght = fontRenderer.LineHeight();
	fontRenderer.BeginUpdate();

	fntJizo.BeginUpdate();
	if (!relayFrag)
	{
		fntJizo.AddString(glm::vec2(580, 90), L"�ړI�F���n���ɐG���");
	}
	else
	{
		fntJizo.AddString(glm::vec2(580, 90), L"�ړI�F�S�[����ڎw��");
	}
	fntJizo.EndUpdate();
	
	if (state == GameState::clear)
	{
		fontRenderer.AddString(glm::vec2(-75, 25), L"GAMECLEAR");
		fontRenderer.AddString(glm::vec2(-105, -75), L"Enter�L�[�ōĊJ");
		fontRenderer.AddString(glm::vec2(-105, -125), L"Esc�L�[�ŏI��");
		fontRenderer.EndUpdate();
		return;
	}

	if (state == GameState::over)
	{
		fontRenderer.AddString(glm::vec2(-75, 0), L"GAMEOVER");
		fontRenderer.AddString(glm::vec2(-105, -75), L"Enter�L�[�ōĊJ");
		fontRenderer.AddString(glm::vec2(-105, -125), L"Esc�L�[�ŏI��");
		fontRenderer.EndUpdate();
		return;
	}
	
	//�J�����̏�Ԃ��X�V����
	if(c_state == CameraState::playcamera)
	{
		cameraRadius = -window.CursorPos().x;
		float cameraRotate = glm::radians(cameraRadius);
		const glm::mat4 CameraRotY = glm::rotate(glm::mat4(1), cameraRotate, glm::vec3(0, 1, 0));
		player->cameraPosition = camera.position;
		player->cameraRotate = cameraRadius;
		player->velocityRotY = CameraRotY;
		player->debugFrag = false;
		
		//�J����
		vCamera = camera.position - player->position;   //�Ȃ���O�̃v���C���[�ƃJ�����̃x�N�g��
		vCameraTraget = CameraRotY * glm::vec4(0, 2.5f, -3, 1); //�Ȃ�������̃v���C���[�ƃJ�����x�N�g��
		cameraVelocity = vCameraTraget - vCamera; //�Ȃ���O�ƋȂ�������̃x�N�g��
		vCamera += cameraVelocity * 0.5f;

		//�����_
		vTraget = camera.target - player->position; //�v���C���[�ƒ����_�̃x�N�g��
		vTragetoffset = CameraRotY * glm::vec4(0, 0, 5, 1);
		TragetVelocity = vTragetoffset - vTraget;
		vTraget += TragetVelocity * 0.5f;

		camera.position = player->position + vCamera;   //�J�����̈ʒu���X�V
		camera.target = player->position + vTraget;       //�����_�̈ʒu���X�V
	}

	if (c_state == CameraState::debug)
	{
		player->debugFrag = true;
		camera.target = glm::vec3(100, 0, 85);
		camera.position = glm::vec3(100, 50, 86);
		camera.target.y =heightMap.Height(camera.target) + 3.0f;
		camera.position.y = heightMap.Height(camera.position) + 60.0f;
	}
	
	for (auto& e : enemies)
	{
		e->TargetActor(player);
		DetectCollision(player, e);
		DetectCollision(e, objects);
		ActorPtr enemyAttackCollision = e->GetAttackCollision();
		if (enemyAttackCollision)
		{
			bool hit = false;
			DetectCollision(enemyAttackCollision, player,
				[this, &hit](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
			{
				state = GameState::over;
			}
			);
		}
	}
	
	objects.Update(deltaTime);
	player->Update(deltaTime);
	enemies.Update(deltaTime);
	L_enemies.Update(deltaTime);
	lights.Update(deltaTime);
	
	DetectCollision(player, objects);

	glm::vec3 goalPos = glm::vec3(108, 0, 69);
	goalPos.y = heightMap.Height(goalPos);
	if (ClearPositionFrag(player->position, goalPos) && relayFrag)
	{
		state = GameState::clear;
	}

	// ���C�g�̍X�V
	glm::vec3 ambientColor(0.5f, 0.25f, 0.45f);
	lightBuffer.Update(lights, ambientColor);
	
	player->UpdateDrawData(deltaTime);
	enemies.UpdateDrawData(deltaTime);
	L_enemies.UpdateDrawData(deltaTime);
	objects.UpdateDrawData(deltaTime);
	lights.UpdateDrawData(deltaTime);

	spriteRenderer.BeginUpdate();
	
	for (const Sprite& e : sprites)
	{
		spriteRenderer.AddVertices(e);
	}
	
	for (auto& e : enemies)
	{
		glm::vec3 position = glm::rotate(glm::mat4(1), e->rotation.y,
			glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 2.5f, 1);
		position += e->position;
		sprEnemy.Rotation(e->rotation.y);
		sprEnemy.Position(sprEnemyPos + glm::vec3(e->position.x * 12, e->position.z * -12, 0.0f));
		spriteRenderer.AddVertices(sprEnemy);
		if (e->state == EnemyActor::State::vigilance ||
			e->state == EnemyActor::State::overlook)
		{
			sprVigilance.Position(sprEnemyPos + glm::vec3(e->position.x * 12, e->position.z * -11.8f, 0.0f));
			spriteRenderer.AddVertices(sprVigilance);
		}
		else if (e->state == EnemyActor::State::approach ||
				e->state == EnemyActor::State::attack)
		{
			sprDiscovery.Position(sprEnemyPos + glm::vec3(e->position.x * 12, e->position.z * -11.8f, 0.0f));
			spriteRenderer.AddVertices(sprDiscovery);
		}
	}
	sprPlayer.Position(sprEnemyPos + glm::vec3(player->position.x * 12, player->position.z * -12, 0));
	spriteRenderer.AddVertices(sprPlayer);
	spriteRenderer.EndUpdate();
	fontRenderer.EndUpdate();
}

/*
�V�[����`�悷��
*/
void MainGameScene::Render()
{
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	
	
	glEnable(GL_DEPTH_TEST);

	lightBuffer.Upload();
	lightBuffer.Bind();

	// FBO�ɕ`��
	/*
	glBindFramebuffer(GL_FRAMEBUFFER, fboMain->GetFramebuffer());
	glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	*/

	const glm::mat4 matView = glm::lookAt(camera.position, camera.target, camera.up);
	const float aspectRatio =
		static_cast<float>(window.Width()) / static_cast<float>(window.Height());
	const glm::mat4 matProj =
		glm::perspective(camera.fov * 0.5f, aspectRatio, camera.near, camera.far);
	
	glm::vec3 cubePos(100, 0, 100);
	cubePos.y = heightMap.Height(cubePos);
	
	const glm::mat4 matModel = glm::translate(glm::mat4(1), cubePos);
	meshBuffer.SetViewProjectionMatrix(matProj * matView);
	meshBuffer.SetCameraPosition(camera.position);
	meshBuffer.SetTime(window.Time());
	Mesh::Draw(meshBuffer.GetFile("Terrain"), glm::mat4(1));

	objects.Draw();
	enemies.Draw();
	L_enemies.Draw();
	player->Draw();
	spriteRenderer.Draw(screenSize);
	fntJizo.Draw(screenSize);
	
	
	

	/*
	Mesh::Draw(meshBuffer.GetFile("Water"), glm::mat4(1));
	*/
	// �f�t�H���g�̃t���[���o�b�t�@�ɕ`��
	{
		/*
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);

		camera.Update(matView);

		Mesh::FilePtr mesh = meshBuffer.GetFile("RenderTarget");
		Shader::ProgramPtr prog = mesh->material[0].program;
		prog->Use();
		prog->SetViewInfo(static_cast<float>(window.Width()), 
			static_cast<float>(window.Height()), camera.near, camera.far);
		prog->SetCameraInfo(camera.focalPlane, camera.focalLength,
			camera.apertrue, camera.sensorSize);
		Mesh::Draw(mesh, glm::mat4(1));
		*/
		fontRenderer.Draw(screenSize);
	}
}

/*
���n���l�ɐG�ꂽ�Ƃ��̏���

@param id  ���n���l�̔ԍ�
@param pos ���n���l�̍��W

@retval true  ��������
@retval false ���łɐ퓬���̂��ߏ������Ȃ�
*/
bool MainGameScene::HandleJizoEffects(int id, const glm::vec3& pos)
{
	if (jizoId >= 0)
	{
		return false;
	}
	for (auto& e : enemies)
	{
		e->health = 0;
	}

	relayFrag = true;
	
	LoiteringEnemyPtr p;
	for (int i = 0; i < 3; i++)
	{
		if (i == 0)
		{
			p = std::make_shared<LoiteringEnemy>(&heightMap, meshBuffer, glm::vec3(96, 0, 82));
			p->roundPoints[0] = glm::vec3(89, 0, 82);
			p->roundPoints[1] = glm::vec3(89, 0, 78);
			p->roundPoints[2] = glm::vec3(94, 0, 79);
			p->roundPoints[3] = glm::vec3(96, 0, 82);
			p->roundPoints[4] = glm::vec3(94, 0, 82);
			p->position.y = heightMap.Height(p->position);
			p->ObstacleActor(objects);
			enemies.Add(p);
		}
		else if (i == 1)
		{
			p = std::make_shared<LoiteringEnemy>(&heightMap, meshBuffer, glm::vec3(100, 0, 76));
			p->roundPoints[0] = glm::vec3(100, 0, 79);
			p->roundPoints[1] = glm::vec3(100, 0, 72);
			p->roundPoints[2] = glm::vec3(103, 0, 76);
			p->roundPoints[3] = glm::vec3(100, 0, 79);
			p->roundPoints[4] = glm::vec3(100, 0, 72);
			p->position.y = heightMap.Height(p->position);
			p->ObstacleActor(objects);
			enemies.Add(p);
		}
		else if (i == 2)
		{
			p = std::make_shared<LoiteringEnemy>(&heightMap, meshBuffer, glm::vec3(108, 0, 73));
			p->roundPoints[0] = glm::vec3(104, 0, 82);
			p->roundPoints[1] = glm::vec3(108, 0, 73);
			p->roundPoints[2] = glm::vec3(104, 0, 82);
			p->roundPoints[3] = glm::vec3(108, 0, 73);
			p->roundPoints[4] = glm::vec3(108, 0, 75);
			p->position.y = heightMap.Height(p->position);
			p->ObstacleActor(objects);
			enemies.Add(p);
		}
	}
	return true;
}

/*
�J�����̃p�����[�^���X�V����

@param matView �X�V�Ɏg�p����r���[�s��
*/
void MainGameScene::Camera::Update(const glm::mat4& matView)
{
	const glm::vec4 pos = matView * glm::vec4(target, 1);
	// �Q�[�����̒P�ʂ̓��[�g���P�ʂȂ̂�1000�{����K�v������
	focalPlane = pos.z * -1000.0f; 

	// ���a���甼�a�ɕς���
	const float imageDistance = sensorSize * 0.5f / glm::tan(fov * 0.5f);
	focalLength = 1.0f / ((1.0f / focalPlane) + (1.0f / imageDistance));
	apertrue = focalLength / fNumber;
}

bool MainGameScene::ClearPositionFrag(glm::vec3 playerPos, glm::vec3 pos)
{
	float length = glm::length(pos - playerPos);
	if (length <= 2.5f && length >= -2.5f)
	{
		return true;
	}
	return false;
}