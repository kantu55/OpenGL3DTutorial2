/*
@file Scene.h
*/
#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED
#include <memory>
#include <string>

class SceneStack;

/*
シーン基底クラス
*/
class Scene
{
public:
	Scene(const char* name);
	Scene(const Scene&) = delete;
	Scene& operator = (const Scene&) = delete;
	virtual ~Scene();

	virtual bool Initialize() = 0{}
	virtual void ProcessInput() = 0{}
	virtual void Update(float) = 0{}
	virtual void Render() = 0{}
	virtual void Finalize() = 0{}

	virtual void Play();
	virtual void Stop();
	virtual void Show();
	virtual void Hide();

	const std::string& Name() const;
	bool IsActive() const;
	bool IsVisible() const;

private:
	std::string name;
	bool isVisible = true;
	bool isActive = true;
};
using ScenePtr = std::shared_ptr<Scene>;

#endif // SCENE_H_INCLUDED
