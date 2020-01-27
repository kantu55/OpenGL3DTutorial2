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

bool MainGameScene::Initialize()
{
	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);
	Sprite spr(Texture::Image2D::Create("Res/Sora.tga"));
	spr.Scale(glm::vec2(2));
	sprites.push_back(spr);

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");

	meshBuffer.Init(1'000'000 * sizeof(Mesh::Vertex), 3'000'000 * sizeof(GLushort));

	meshBuffer.LoadMesh("Res/red_pine_tree.gltf");
	meshBuffer.LoadMesh("Res/wall_stone.gltf");
	meshBuffer.LoadMesh("Res/jizo_statue.gltf");
	meshBuffer.LoadSkeletalMesh("Res/bikuni.gltf");
	meshBuffer.LoadSkeletalMesh("Res/oni_small.gltf");

	// FBO���쐬����
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

	//�n�C�g�}�b�v���쐬����
	if (!heightMap.LoadFromFile("Res/Terrain.tga", 20.0f, 0.5f))
	{
		return false;
	}
	if (!heightMap.CreateMesh(meshBuffer, "Terrain"))
	{
		return false;
	}
	// ���ʂ̍����͗v����
	if (!heightMap.CreaetWaterMesh(meshBuffer, "Water", 8))
	{
		return false;
	}

	lightBuffer.Init(1);
	lightBuffer.BindToShader(meshBuffer.GetStaticMeshShader());
	lightBuffer.BindToShader(meshBuffer.GetTerrainShader());
	lightBuffer.BindToShader(meshBuffer.GetWaterShader());

	glm::vec3 startPos(100, 0, 100);
	startPos.y = heightMap.Height(startPos);
	player = std::make_shared<PlayerActor>( &heightMap, meshBuffer, startPos);

	/*
	camera.target = player->position + glm::vec3(-2.5f, 0, 5);
	camera.position = player->position + glm::vec3(2.5f, 4, -6);
	*/
	
	// ���C�g�̔z�u
	lights.Add(std::make_shared<DirectionalLightActor>(
		"DirectionalLight", glm::vec3(1.0f), glm::normalize(glm::vec3(1, -2, -1))));
	for (int i = 0; i < 50; ++i)
	{
		glm::vec3 color(1, 0.8f, 0.5f);
		glm::vec3 position(0);
		glm::vec3 direction(0,5,0);
		position.x = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.z = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
		position.y = heightMap.Height(position) + 1;
		lights.Add(std::make_shared<PointLightActor>("PointLight", color, position));
		lights.Add(std::make_shared<SpotLightActor>("SpotLight", color, position, direction
			, glm::radians(20.0f), glm::radians(15.0f)));
	}
	lights.Update(0);
	lightBuffer.Update(lights, glm::vec3(0.1f, 0.05f, 0.15f));
	heightMap.UpdateLightIndex(lights);

	// ���n����z�u
	for (int i = 0; i < 4; ++i)
	{
		glm::vec3 position(0);
		position.x = static_cast<float>(std::uniform_int_distribution<>(50, 150)(rand));
		position.z = static_cast<float>(std::uniform_int_distribution<>(50, 150)(rand));
		position.y = heightMap.Height(position);
		glm::vec3 rotation(0);
		rotation.y = std::uniform_real_distribution<float>(0.0f, 3.14f * 2.0f)(rand);
		JizoActorPtr p = std::make_shared<JizoActor>(
			meshBuffer.GetFile("Res/jizo_statue.gltf"), position, i, this);
		p->scale = glm::vec3(3);  // �����₷���悤�Ɋg��
		objects.Add(p);
	}

	rand.seed(0);
	enemy = std::make_shared<EnemyActor>(&heightMap, meshBuffer, startPos + glm::vec3(0, 0, -10));

	// �Εǂ�z�u
	{
		const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/wall_stone.gltf");
		glm::vec3 position = startPos + glm::vec3(0, -50, -5);
		position.y = heightMap.Height(position);
		StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
			meshStoneWall, "StoneWall", 100, position, glm::vec3(0));
		p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
			glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(2, 2, 0.5f));
		objects.Add(p);
		enemy->ObjectActor(p);
	}
	

	// �G��z�u
	{
		const size_t oniCount = 10;
		enemies.Reserve(oniCount);
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

		if (!flag)
		{
			flag = true;
			SceneStack::Instance().Replace(std::make_shared<StatusScene>());
		}
		else
		{
			SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
		}
	}
}

/*
�V�[�����X�V����

@param deltaTime �O��̍X�V����̌o�ߎ��ԁi�b�j
*/
void MainGameScene::Update(float deltaTime)
{
	//�J�����̏�Ԃ��X�V����
	{
		camera.target = player->position + glm::vec3(0, 3, 0);
		camera.position = player->position + glm::vec3(0, 7, 10);

		/*
		const glm::mat4 CameraRotY = glm::rotate(glm::mat4(1), player->position.y, glm::vec3(0, 1, 0));

		//�J����
		vCamera = camera.position - player->position;   //�Ȃ���O�̃v���C���[�ƃJ�����̃x�N�g��
		vCameraTraget = CameraRotY * glm::vec4(2.5f, 4, -6, 1); //�Ȃ�������̃v���C���[�ƃJ�����x�N�g��
		cameraVelocity = vCameraTraget - vCamera; //�Ȃ���O�ƋȂ�������̃x�N�g��
		vCamera += cameraVelocity * (deltaTime * 5);    //���̃x�N�g�����AdeltaTime���i�߂���

		//�����_
		vTraget = camera.target - player->position; //�v���C���[�ƒ����_�̃x�N�g��
		vTragetoffset = CameraRotY * glm::vec4(-2.5f, 0, 5, 1);
		TragetVelocity = vTragetoffset - vTraget;
		vTraget += TragetVelocity * (deltaTime * 5);
		camera.position = player->position + vCamera;   //�J�����̈ʒu���X�V
		camera.target = player->position + vTraget; //�����_�̈ʒu���X�V
		*/
	}
	
	enemy->TargetActor(player);

	player->Update(deltaTime);
	enemy->Update(deltaTime);
	enemies.Update(deltaTime);
	objects.Update(deltaTime);
	lights.Update(deltaTime);

	DetectCollision(player, enemies);
	DetectCollision(player, enemy);
	DetectCollision(player, objects);

	// �v���[���[�̍U������
	ActorPtr attackCollision = player->GetAttackCollision();
	if (attackCollision)
	{
		bool hit = false;
		DetectCollision(attackCollision, enemies,
			[this, &hit](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			SkeletalMeshActorPtr bb = std::static_pointer_cast<SkeletalMeshActor>(b);
			bb->health -= a->health;
			if (bb->health <= 0)
			{
				bb->colLocal = Collision::Shape{};
				bb->health = 1;
				bb->GetMesh()->Play("Down", false);
			}
			else
			{
				bb->GetMesh()->Play("Hit", false);
			}
			hit = true;
		}
		);
		if (hit)
		{
			attackCollision->health = 0;
		}

		DetectCollision(attackCollision, enemy,
			[this, &hit](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			EnemyActorPtr bb = std::static_pointer_cast<EnemyActor>(b);
			bb->health -= a->health;
			if (bb->health <= 0)
			{
				bb->colLocal = Collision::Shape{};
				bb->health = 1;
				bb->GetMesh()->Play("Down", false);
			}
			else
			{
				bb->GetMesh()->Play("Hit", false);
			}
			hit = true;
		}
		);
		if (hit)
		{
			attackCollision->health = 0;
		}
	}

	// ���S�A�j���V�����̏I������G������
	for (auto& e : enemies)
	{
		EnemyActorPtr enemy = std::static_pointer_cast<EnemyActor>(e);
		Mesh::SkeletalMeshPtr mesh = enemy->GetMesh();
		if (mesh->IsFinished())
		{
			if (mesh->GetAnimation() == "Down")
			{
				enemy->health = 0;
			}
			else
			{
				mesh->Play("Wait");
			}
		}
	}

	if (enemy->GetMesh()->IsFinished())
	{
		if (enemy->GetMesh()->GetAnimation() == "Down")
		{
			enemy->health = 0;
		}
		else
		{
			enemy->GetMesh()->Play("Wait");
		}
	}

	// ���C�g�̍X�V
	glm::vec3 ambientColor(0.1f, 0.05f, 0.15f);
	lightBuffer.Update(lights, ambientColor);
	/*
	for (auto e : enemies)
	{
		const std::vector<ActorPtr> neighborhood = lights.FindNearbyActors(e->position, 20);
		std::vector<int> pointLightIndex;
		std::vector<int> spotLightIndex;
		pointLightIndex.reserve(neighborhood.size());
		spotLightIndex.reserve(neighborhood.size());
		for (auto light : neighborhood)
		{
			if (PointLightActorPtr p = std::dynamic_pointer_cast<PointLightActor>(light))
			{
				if (pointLightIndex.size() < 8)
				{
					pointLightIndex.push_back(p->index);
				}
			}
			else if (SpotLightActorPtr p = std::dynamic_pointer_cast<SpotLightActor>(light))
			{
				if (spotLightIndex.size() < 8)
				{
					spotLightIndex.push_back(p->index);
				}
			}
		}
		StaticMeshActorPtr p = std::static_pointer_cast<StaticMeshActor>(e);
		p->SetPointLightList(pointLightIndex);
		p->SetSpotLightList(spotLightIndex);
	}
	*/

	// �G��S�ł�������ړI�B���t���O��true�ɂ���
	if (jizoId >= 0)
	{
		if (enemies.Empty())
		{
			achivements[jizoId] = true;
			jizoId = -1;
		}
	}

	player->UpdateDrawData(deltaTime);
	enemy->UpdateDrawData(deltaTime);
	enemies.UpdateDrawData(deltaTime);
	objects.UpdateDrawData(deltaTime);

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
	
	if (enemy->state == EnemyActor::State::wait)
	{
		fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHenght), L"�ҋ@");
	}
	else if (enemy->state == EnemyActor::State::patrol)
	{
		fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHenght), L"����");
	}
	else if (enemy->state == EnemyActor::State::approach)
	{
		fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHenght), L"�߂Â�");
	}
	else if (enemy->state == EnemyActor::State::attack)
	{
		fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHenght), L"�U��");
	}
	else if (enemy->state == EnemyActor::State::round)
	{
		fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHenght), L"����");
	}
	fontRenderer.EndUpdate();
}

/*
�V�[����`�悷��
*/
void MainGameScene::Render()
{
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	
	spriteRenderer.Draw(screenSize);
	glEnable(GL_DEPTH_TEST);

	lightBuffer.Upload();
	lightBuffer.Bind();

	// FBO�ɕ`��
	glBindFramebuffer(GL_FRAMEBUFFER, fboMain->GetFramebuffer());
	glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

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

	player->Draw();
	enemy->Draw();
	enemies.Draw();
	objects.Draw();

	glm::vec3 treePos(110, 0, 110);
	treePos.y = heightMap.Height(treePos);
	const glm::mat4 matTreeModel =
		glm::translate(glm::mat4(1), treePos) * glm::scale(glm::mat4(1), glm::vec3(1));
	Mesh::Draw(meshBuffer.GetFile("Res/red_pine_tree.gltf"), matTreeModel);
	Mesh::Draw(meshBuffer.GetFile("Water"), glm::mat4(1));

	// �f�t�H���g�̃t���[���o�b�t�@�ɕ`��
	{
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
	jizoId = id;
	const size_t oniCount = 8;
	for (size_t i = 0; i < oniCount; i++)
	{
		glm::vec3 position(pos);
		position.x += std::uniform_real_distribution<float>(-15, 15)(rand);
		position.z += std::uniform_real_distribution<float>(-15, 15)(rand);
		position.y = heightMap.Height(position);
		//�G�̌����������_���ɑI��
		glm::vec3 rotation(0);
		rotation.y = std::uniform_real_distribution<float>(0, glm::pi<float>() * 2)(rand);
		EnemyActorPtr p =std::make_shared<EnemyActor>(&heightMap, meshBuffer,position);

		p->colLocal = Collision::CreateCapsule(
			glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
		enemies.Add(p);
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