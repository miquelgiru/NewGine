#include "GameObject.h"
#include "Globals.h"
#include "Glew\include\glew.h"
#include "GlobalFunctions.h"

#include "TransformComponent.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "CameraComponent.h"


GameObject::GameObject() 
{
	uid = GenerateUUID();
	aabb.SetNegativeInfinity();
	originalAABB.SetNegativeInfinity();
	strcpy(name, "Unnamed");
}

GameObject::GameObject(const uint uid)
{
	this->uid = uid;
	aabb.SetNegativeInfinity();
	originalAABB.SetNegativeInfinity();
	strcpy(name, "Unnamed");
}

GameObject::GameObject(const char* name, uint uuid, GameObject* parent, bool is_static, bool is_active)
{
	strcpy(this->name, name);
	uid = uuid;
	parent = parent;
	this->is_static = is_static;
	this->active = is_active;

	aabb.SetNegativeInfinity();
	originalAABB.SetNegativeInfinity();
}


GameObject::~GameObject() 
{
	std::list<Component*>::iterator it = components.begin();
	while(it != components.end())
	{
		delete (*it);
		it++;
	}
	components.clear();

	parent = nullptr;
}

const uint64_t GameObject::GetUID() { return uid; }

void GameObject::DrawLocator()
{
	//2DO implement locator
}

void GameObject::DrawAABB()
{
	if (aabb.IsFinite())
	{
		math::float3 corners[8];
		aabb.GetCornerPoints(corners);
	}
}

void GameObject::DrawOBB()
{
	if (obb.IsFinite())
	{
		math::float3 corners[8];
		obb.GetCornerPoints(corners);
	}
}

void GameObject::Select()
{
	selected = true;
	
	for (int i = 0; i < childs.size(); i++)
	{
		childs[i]->Select();
	}
}

void GameObject::Unselect()
{
	selected = false;

	for (int i = 0; i < childs.size(); i++)
	{
		childs[i]->Unselect();
	}
}


void GameObject::UpdateAABB()
{
	aabb.SetNegativeInfinity();
	obb.SetNegativeInfinity();
	if (originalAABB.IsFinite())
	{
		obb = originalAABB;
		TransformComponent* t = (TransformComponent*)GetComponent(COMPONENT_TRANSFORM);
		obb.Transform(t->GetGlobalTranform().Transposed());
		aabb.Enclose(obb);
	}
}

void GameObject::UpdateTransformMatrix()
{
	if (HasComponent(COMPONENT_TYPE::COMPONENT_TRANSFORM))
	{
		TransformComponent* t = (TransformComponent*)GetComponent(COMPONENT_TRANSFORM);
		t->UpdateGlobalTransform();
	}

	UpdateAABB();

	//Call again for every child
	if (childs.size() > 0)
	{
		std::vector<GameObject*>::iterator child = childs.begin();
		while (child != childs.end())
		{
			(*child)->UpdateTransformMatrix();
			child++;
		}
	}
}

void GameObject::SetActive(bool state)
{
	active = state;
}

bool GameObject::IsActive()
{
	return active;
}

void GameObject::SetStatic(bool Stat)
{
	is_static = Stat;
}

bool GameObject::IsStatic()
{
	return is_static;
}

void GameObject::SetName(const char* newName)
{
	strcpy(name, newName);
}

const char* GameObject::GetName()
{
	return name;
}

void GameObject::Save(JSONWrapper& file)
{
	JSONWrapper array_value;

	array_value.WriteString("name", name);
	array_value.WriteUInt("UUID", uid);

	if (parent == nullptr)
		array_value.WriteUInt("parent", 0);
	else
		array_value.WriteUInt("parent", parent->uid);

	array_value.WriteBool("static", is_static);
	array_value.WriteBool("active", active);


	//Save Components
	array_value.WriteArray("components");
	std::list<Component*>::const_iterator it = components.begin();
	while (it != components.end())
	{
		(*it)->Save(array_value);
		it++;
	}

	file.WriteArrayValue(array_value);


	//save for its childs
	std::vector<GameObject*>::const_iterator child = childs.begin();
	while (child != childs.end())
	{
		(*child)->Save(file);
		child++;
	}
		
}



//Components===================================================================

Component* GameObject::AddComponent(COMPONENT_TYPE type)
{
	if (type == COMPONENT_TRANSFORM)
	{
		TransformComponent* transform = new TransformComponent(type, this);
		components.push_back(transform);
		return transform;
	}

	if (type == COMPONENT_MATERIAL)
	{
		MaterialComponent* material = new MaterialComponent(type, this);
		components.push_back(material);
		return material;
	}

	if (type == COMPONENT_MESH)
	{
		MeshComponent* mesh = new MeshComponent(type, this);
		components.push_back(mesh);
		return mesh;
	}

	if (type == COMPONENT_CAMERA)
	{
		CameraComponent* camera = new CameraComponent(type, this);
		components.push_back(camera);
		return camera;
	}
}

bool GameObject::DeleteComponent(Component* ComponentToDelete)
{
	bool ret = false;

	//2DO

	return ret;
}

bool GameObject::HasComponent(COMPONENT_TYPE type)
{
	std::list<Component*>::iterator it = components.begin();
	while (it != components.end())
	{
		if ((*it)->type == type)
		{
			return true;
		}
		it++;
	}
	return false;
}

Component* GameObject::GetComponent(COMPONENT_TYPE type)
{
	std::list<Component*>::iterator it = components.begin();
	while (it != components.end())
	{
		if ((*it)->type == type)
		{
			return (*it);
		}

		it++;
	}
	return nullptr;
}


