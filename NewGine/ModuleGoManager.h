#pragma once
#ifndef _GOMANAGER_H_
#define _GOMANAGER_H_

#include "Globals.h"
#include "Module.h"
#include "GameObject.h"
#include <list>

class CameraComponent;

class ModuleGOManager : public Module
{
public:

	ModuleGOManager(Application* app, bool start_enabled = true);
	~ModuleGOManager();

	bool Init();
	update_status Update(float dt);


	//GameObject Management
	GameObject* CreateEmpty(const char* name);
	GameObject* CreateGameObject(const char* name, GameObject* parent = nullptr);
	bool DeleteGameObject(GameObject* to_delete);
	GameObject* CreateCamera(const char* name);
	GameObject* GetCameraObject();
	CameraComponent* GetCameraComponent();


public:
	std::list<GameObject*> all_gameobjects;
	std::list<GameObject*> objects_to_draw;

private:
	std::list<GameObject*> to_delete_gos;


	GameObject * camera = nullptr;
	CameraComponent* cam_comp = nullptr;
	GameObject* root = nullptr;

};

#endif // _GOMANAGER_H_
