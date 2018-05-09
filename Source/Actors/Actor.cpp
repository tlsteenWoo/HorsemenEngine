#include "HorsemanStd.h"
#include "Actor.h"
#include "ActorComponent.h"
#include <iostream>

Actor::Actor(ActorId id) {
	m_Id = id;
}

Actor::~Actor(void)
{
}

bool Actor::Init(rapidxml::xml_node<>* pNode)
{
	// TODO: Initialization logic here
	return true;
}

void Actor::PostInit()
{
	// TODO: Post initialization logic here
	
	for (auto comp : m_Components) {
		auto pComp = comp.second;
		pComp->VPostInit();
	}
}

void Actor::Cleanup(void)
{
	m_Components.clear();
}

void Actor::Update(float dt)
{
	for (auto comp : m_Components) {
		auto pComp = comp.second;
		pComp->Update(dt);
	}
}

void Actor::AddComponent(StrongActorComponentPtr pComponent)
{
	m_Components[pComponent->VGetId()] = pComponent;
}
