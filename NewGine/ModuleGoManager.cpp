
#include"ModuleGOManager.h"
#include "GameObject.h"
#include "Application.h"
#include "CameraComponent.h"
#include "MeshComponent.h"
#include "JSONWrapper.h"
#include "GlobalFunctions.h"


ModuleGOManager::ModuleGOManager(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	quadtree = new Quadtree(float3(WORLD_WIDTH / -2, WORLD_HEIGHT / -2, WORLD_DEPTH / -2), float3(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, WORLD_DEPTH / 2));
}

ModuleGOManager::~ModuleGOManager()
{
	if(camera)
		delete camera;

	if(cam_comp)
		delete cam_comp;

	if (root)
		delete root;


	camera = nullptr;
	cam_comp = nullptr;
	selected_go = nullptr;
}

bool ModuleGOManager::Start()
{
	if (root == nullptr)
	{
		LoadEmptyScene();
	}

	return true;
}

update_status ModuleGOManager::PreUpdate(float dt)
{
	std::list<GameObject*>::iterator it = todelete_objects.begin();
	while (it != todelete_objects.end())
	{
		if ((*it)->IsStatic())
		{
			quadtree->Remove((*it));
		}

		else
		{
			dynamic_objects.remove((*it));
		}

		delete (*it);
		it++;
	}

	todelete_objects.clear();

	return UPDATE_CONTINUE;
}



update_status ModuleGOManager::Update(float dt)
{
	//SelectObject(); 2DO reactivate
	DrawLocator();

	//2DO condition to draw
	App->renderer3D->DebugDrawQuadtree(quadtree, quadtree->GetRoot());

	return update_status::UPDATE_CONTINUE;
}

GameObject* ModuleGOManager::CreateEmpty(const char* name)
{
	GameObject* new_go = new GameObject();
	new_go->AddComponent(COMPONENT_TRANSFORM);

	if (name != nullptr && name != "")
	{
		new_go->SetName(name);
	}

	if (root)
		new_go->parent = root;

	else
		new_go = root;

	all_gameobjects.push_back(new_go);

	return new_go;
}

GameObject* ModuleGOManager::CreateGameObject(const char* name, GameObject* parent)
{
	GameObject* new_go = new GameObject();
	new_go->AddComponent(COMPONENT_TRANSFORM);

	if (name != nullptr && name != "")
	{
		new_go->SetName(name);
	}

	if (parent != nullptr)
		new_go->parent = parent;
	else
		new_go->parent = root;

	return new_go;
}

GameObject* ModuleGOManager::CreateGameObject(const char* name, uint uuid, GameObject* parent, bool is_static, bool is_active)
{
	GameObject* ret = new GameObject(name, uuid, parent, is_static, is_active);
	all_gameobjects.push_back(ret);
	return ret;
}


bool ModuleGOManager::DeleteGameObject(GameObject* to_delete)
{
	bool ret = false;

	if (to_delete)
	{
		to_delete_gos.push_back(to_delete);
		ret = true;
	}

	return ret;
}

GameObject* ModuleGOManager::CreateCamera(const char* name)
{
	GameObject* cam = CreateEmpty("Camera");
	cam->AddComponent(COMPONENT_CAMERA);

	return cam;
}


GameObject* ModuleGOManager::GetCameraObject()
{
	std::list<GameObject*>::iterator it = all_gameobjects.begin();

	while (it != all_gameobjects.end())
	{
		if ((*it)->GetComponent(COMPONENT_CAMERA) != nullptr)
		{
			return (*it);
		}
		it++;
	}

	return nullptr;
}

CameraComponent* ModuleGOManager::GetCameraComponent()
{
	return (CameraComponent*)GetCameraObject()->GetComponent(COMPONENT_CAMERA);
}

GameObject* ModuleGOManager::Raycast(const Ray& ray)const
{
	GameObject* ret = nullptr;


	vector<GameObject*> cols;   //list og game objects that collided with the ray

	cols = quadtree->GetColliderObjects(ray);

	//Sort all the candidates by distance
	std::map<float, GameObject*> candidates;
	std::list<GameObject*>::iterator it = App->go_manager->dynamic_objects.begin();
	while (it != dynamic_objects.end())
	{
		float near_dist, far_dist;
		if ((*it)->obb.Intersects(ray, near_dist, far_dist) == true)
		{
			candidates.insert(std::pair<float, GameObject*>(MIN(near_dist, far_dist), (*it)));
		}
	}


	//iterate all the possible collisions by order
	std::map<float, GameObject*>::iterator mapit = candidates.begin();

	while (mapit != candidates.end())
	{
		float coldist = 100000;

		//check if go has mesh
		if (mapit->second->HasComponent(COMPONENT_MESH))
		{
			TransformComponent* t = (TransformComponent*)mapit->second->GetComponent(COMPONENT_TRANSFORM);

			Ray transposed_ray = ray;
			transposed_ray.Transform(t->GetGlobalTranform().InverseTransposed());

			MeshComponent* m = (MeshComponent*)mapit->second->GetComponent(COMPONENT_MESH);
			const uint num_indices = m->mesh->mesh->num_indices;

			uint u1, u2, u3;
			float3 v1, v2, v3;
			Triangle triangle;
			float distance;
			float3 hitpoint;


			//now create triangles and check them
			for (uint i = 0; i < num_indices/3; i++)
			{
				u1 = m->mesh->mesh->indices[i * 3];
				u2 = m->mesh->mesh->indices[i * 3 + 1];
				u3 = m->mesh->mesh->indices[i * 3 + 2];
				v1 = float3(&m->mesh->mesh->vertices[u1]);
				v2 = float3(&m->mesh->mesh->vertices[u2]);
				v3 = float3(&m->mesh->mesh->vertices[u3]);
				triangle = Triangle(v1, v2, v3);

				if (triangle.Intersects(transposed_ray, &distance, &hitpoint) == true)
				{
					if (distance < coldist)
					{
						//save the mesh with less dist
						ret = mapit->second;
					}
				}
			}
		}
	}


	return ret;
}


void ModuleGOManager::SelectObject()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP)
	{
		CameraComponent* cam = (CameraComponent*)camera->GetComponent(COMPONENT_CAMERA);

		float2 pos(App->input->GetMouseX(), App->input->GetMouseY());

		pos.x = 2.0f * pos.x / (float)App->window->GetWidth() - 1.0f;
		pos.y = 1.0f - 2.0f * pos.y / (float)App->window->GetHeight();

		Ray ray = cam->frustum.UnProjectFromNearPlane(pos.x, pos.y);

		selected_go = Raycast(ray);
	}
}

void ModuleGOManager::DrawLocator()
{
	if (selected_go != nullptr)
	{
		float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f);
		TransformComponent* t = (TransformComponent*)selected_go->GetComponent(COMPONENT_TRANSFORM);

		App->renderer3D->DrawLocator(t->GetGlobalTranform(), color);
	}
}


GameObject* ModuleGOManager::GetRoot()
{
	return root;
}

void ModuleGOManager::LoadScene(const char* name)
{
	string scene_path = ASSETS_FOLDER;
	scene_path += name;
	scene_path += ".fbx";

	//2DO
	char* buffer = nullptr;
	uint size = App->file_system->Load(scene_path.data(), &buffer);

	if (size > 0)
	{
		//2DO Delete current scene
		ClearScene();

		JSONWrapper root(buffer);
		JSONWrapper root_value;

		root_value = root.ReadArray("Scene", 0);

		if (root_value.IsNull() == false)
		{
			ClearScene();

			for (int i = 0; i < root.GetArraySize("Scene"); i++)
			{
				LoadGameObject(root.ReadArray("Scene", i));
			}
		}
	}

	else
	{
		if (buffer)
			delete[] buffer;

		LOG("Error while loading Scene: %s", name);
	}
}

void ModuleGOManager::SaveScene(const char* name)
{
	JSONWrapper root_node;
	root_node.WriteArray("Scene");

	//Save all go in the array
	root->Save(root_node);

	char* buf;
	size_t size = root_node.SerializeBuffer(&buf);
	string scene_path = ASSETS_FOLDER;
	scene_path += name;
	
	if(scene_path.find(".scn"))

	App->file_system->Save(scene_path.data(), buf, size);
}


void ModuleGOManager::LoadEmptyScene()
{
	ClearScene();

	//Empty scene
	root = CreateGameObject("root");
}

void ModuleGOManager::ClearScene()
{
	ClearGameObjectFromScene(root);

	root = nullptr;
	selected_go = nullptr;

	all_gameobjects.clear();
	dynamic_objects.clear();

	//2Do reset quadtree?
}


GameObject* ModuleGOManager::LoadGameObject(const JSONWrapper& file)
{
	//get the go data in local variables
	std::string name  = file.ReadString("Name");
	int uuid = file.ReadUInt("UUID");
	int parent_uuid = file.ReadUInt("Parent");
	bool is_static = file.ReadBool("static");
	bool is_active = file.ReadBool("Active");

	//set the parent of the new object
	GameObject*  parent = nullptr;
	std::list<GameObject*>::iterator it = all_gameobjects.begin();
	while (it != all_gameobjects.end())
	{
		if ((*it)->GetUID() == parent_uuid)
			parent = (*it);
		it++;
	}

	GameObject* new_go = CreateGameObject(name.c_str(), uuid, parent, is_static, is_active);

	//set the new object as child from the parent
	if (parent != nullptr)
		parent->childs.push_back(new_go);
	else
		root = new_go;

	//load components
	JSONWrapper root = file;
	JSONWrapper array_value;

	int size = root.GetArraySize("Components");

	for (int i = 0; i < size; i++)
	{
		array_value = root.ReadArray("Components", i);
		COMPONENT_TYPE t = (COMPONENT_TYPE)array_value.ReadUInt("Type");
		
		Component* comp = nullptr;
		switch (t)
		{
		case COMPONENT_TRANSFORM:
			comp = new_go->AddComponent(t);
			comp->Load(array_value);
			break;
		case COMPONENT_MESH:
			comp = new_go->AddComponent(t);
			comp->Load(array_value);
			break;
		case COMPONENT_MATERIAL:
			comp = new_go->AddComponent(t);
			comp->Load(array_value);
			break;
		case COMPONENT_CAMERA:
			break;
		default:
			break;
		}

		if (t == COMPONENT_MESH)
		{
			//Link component mesh and resource mesh
			MeshComponent* mesh = (MeshComponent*)comp;
			mesh->SetResourceMesh(App->resource_manager->LinkResourceMesh(comp->parent->name, comp->path.data()));
			//Load mesh to memory
			string complete_file = comp->path + comp->parent->name + ".mex";
			mesh->mesh->mesh = App->importer->mesh_importer->LoadMesh(complete_file.data());
		}

		if (t == COMPONENT_MATERIAL)
		{
			//Link component material and resource material
			MaterialComponent* mat = (MaterialComponent*)comp;
			mat->SetResourceMaterial(App->resource_manager->LinkResourceMaterial(comp->parent->name));
			mat->material->texture = App->importer->mat_importer->LoadTexture(mat->path.data());
		}
	}

	if (is_static)
	{
		quadtree->Insert(new_go);
	}

	else
	{
		dynamic_objects.push_back(new_go);
	}
	
	return new_go;
}

void ModuleGOManager::ClearGameObjectFromScene(GameObject* go)
{
	if (go)
	{
		if (go->parent != nullptr)
		{
			std::vector<GameObject*>::iterator it = go->parent->childs.begin();
			while (it != go->parent->childs.end())
			{
				if ((*it) == go)
				{
					go->parent->childs.erase(it);
				}

				it++;
			}

			go->parent = nullptr;
		}

		std::vector<GameObject*>::iterator it = go->childs.begin();
		while (it != go->childs.end())
		{
			ClearGameObjectFromScene((*it));
			it++;
		}
			
		go->childs.clear();

		todelete_objects.push_back(go);
	}
}






