#include "MaterialComponent.h"
#include "imGUI\imgui.h"

#include "Devil\include\il.h"
#include "Devil\include\ilu.h"
#include "Devil\include\ilut.h"


MaterialComponent::MaterialComponent(COMPONENT_TYPE type, GameObject* game_object) : Component(type, game_object, 2)
{
	id = GenerateUUID();
}

MaterialComponent::~MaterialComponent()
{
	delete material;
	material = nullptr;
}

void MaterialComponent::ComponentEditor()
{
	ImGui::Text("Id Material: ");
	ImGui::SameLine();
	ImGui::Text("%d", material->texture->id);
	ImGui::Text("Width: ");
	ImGui::SameLine();
	ImGui::Text("%d", material->texture->width);
	ImGui::Text("Height: ");
	ImGui::SameLine();
	ImGui::Text("%d", material->texture->height);
	ImGui::Image((ImTextureID)material->texture->id, ImVec2(250, 250));
}


void MaterialComponent::SetColor(float r, float g, float b, float a)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = a;
}

void MaterialComponent::SetResourceMaterial(ResourceMaterial* mat)
{
	material = mat;
}
