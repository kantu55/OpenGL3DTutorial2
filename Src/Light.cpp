/*
@file Light.cpp
*/
#include "Light.h"
#include <iostream>

namespace /* unnamed */
{
	const char UniformBlockName[] = "LightUniformBlock";
} // unnamed namespace

/*
ライト用ユニフォームバッファを作成する

@param bindingPoint ユニフォームバッファに割り当てる管理番号
*/
bool LightBuffer::Init(GLuint bindingPoint)
{
	/*
	ダブルバッファ
	GPUとCPUが見ているUBOを分離させる
	*/
	ubo[0] = UniformBuffer::Create(sizeof(data), bindingPoint, UniformBlockName);
	ubo[1] = UniformBuffer::Create(sizeof(data), bindingPoint, UniformBlockName);
	currentUboIndex = 0;

	return ubo[0] && ubo[1];
}

/*
シェーダープログラムにスケルタルメッシュの制御用データのバインディング・ポイントを設定する

@param program バインディング・ポイント設定先のシェーダープログラム

@return true  設定成功
@return false 設定失敗
*/
bool LightBuffer::BindToShader(const Shader::ProgramPtr& program)
{
	const GLuint id = program->Get();
	// UniformBlockの位置を取得する
	const GLuint blockIndex = glGetUniformBlockIndex(id, UniformBlockName);
	if (blockIndex == GL_INVALID_INDEX)
	{
		std::cerr << "[エラー] Uniformブロック'" << UniformBlockName << "'が見つかりません\n";
		return false;
	}
	// バインディング・ポイントにインタフェースブロックを結びつける
	glUniformBlockBinding(id, blockIndex, ubo[0]->BindingPoint());
	const GLenum result = glGetError();
	if (result != GL_NO_ERROR)
	{
		std::cerr << "[エラー] Uniformブロック'" << UniformBlockName << "'のバインドに失敗\n";
		return false;
	}
	return true;
}

/*
GPUへ転送するライトデータを更新する

@param al           // ライトのアクターリスト
@param ambientColor // 環境光の明るさ
*/
void LightBuffer::Update(const ActorList& al, const glm::vec3& ambientLight)
{
	data.ambientLight.color = glm::vec4(ambientLight, 0);

	int pointLightCount = 0;
	int spotLightCount = 0;
	for (auto i = al.begin(); i != al.end(); ++i)
	{
		/*
		[*i]がLightActorを指していたら成功、StaticActorを指していたら失敗
		*/
		if (DirectionalLightActorPtr p =
			std::dynamic_pointer_cast<DirectionalLightActor>(*i))
		{
			data.directionalLight.color = glm::vec4(p->color, 0);
			data.directionalLight.direction = glm::vec4(p->direction, 0);
		}
		else if (PointLightActorPtr p = std::dynamic_pointer_cast<PointLightActor>(*i))
		{
			if (pointLightCount < 100)
			{
				p->index = pointLightCount;
				PointLight& light = data.pointLight[pointLightCount];
				light.color = glm::vec4(p->color, 1);
				light.position = glm::vec4(p->position, 1);
				++pointLightCount;
			}
		}
		else if (SpotLightActorPtr p = std::dynamic_pointer_cast<SpotLightActor>(*i))
		{
			if (spotLightCount < 100)
			{
				p->index = spotLightCount;
				SpotLight& light = data.spotLight[spotLightCount];
				light.color = glm::vec4(p->color, 1);
				light.dirAndCutOff = glm::vec4(p->direction, p->cutOff);
				light.posAndInnerCutOff = glm::vec4(p->position, p->innerCutOff);
				++spotLightCount;
			}
		}
	}
}

/*
ライトデータをGPUメモリに転送する
*/
void LightBuffer::Upload()
{
	UniformBufferPtr pUbo = ubo[currentUboIndex];
	pUbo->BufferSubData(&data, 0, sizeof(data));
	// 「！演算子」でcurrentUboIndexの値を反転させる
	currentUboIndex = !currentUboIndex;
}

/*
ライト用UBOをGLコンテキストのUBO用バインディング・ポイントに割り当てる
*/
void LightBuffer::Bind()
{
	ubo[!currentUboIndex]->BindBufferRange(0, sizeof(data));
}