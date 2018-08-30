#pragma once
#include "Module.h"
#include "Globals.h"
#include "Primitive.h"



class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	void CreateCubePrimitive();

	uint lenna_id;

	

public:
	Cube_P cube;
	
};