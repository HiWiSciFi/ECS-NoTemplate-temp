#include "ECS.hpp"
#include "gsl.hpp"
#include "IdPool.hpp"
#include "ComponentStore.hpp"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace Junia {

static IdPool<EntityIdType>& GetEntityPool() {
	static IdPool<EntityIdType> pool = IdPool<EntityIdType>();
	return pool;
}

// -----------------------------------------------------------------------------
// ------------------------------ Global functions -----------------------------
// -----------------------------------------------------------------------------

void RegisterComponent(std::type_index type, size_t size, size_t preallocCount,
	DestructorFunc destructor, CopyConstructorFunc copyConstructor) {
	ComponentStore::Create(type, size, preallocCount,
		std::move(destructor), std::move(copyConstructor));
}

void UnregisterComponent(std::type_index type) {
	ComponentStore::Destroy(type);
}

size_t GetComponentOffset(std::type_index type, EntityIdType entity) {
	return ComponentStore::Get(type)->GetComponentOffset(entity);
}

void* GetComponentByOffset(std::type_index type, size_t offset) {
	return ComponentStore::Get(type)->GetComponentByOffset(offset);
}

void* AddComponent(std::type_index type, EntityIdType entity) {
	return ComponentStore::Get(type)->AllocateComponent(entity);
}

void RemoveComponent(std::type_index type, EntityIdType entity) {
	ComponentStore::Get(type)->RemoveComponent(entity);
}

void* GetComponent(std::type_index type, EntityIdType entity) {
	return ComponentStore::Get(type)->GetComponent(entity);
}

// -----------------------------------------------------------------------------
// ---------------------------------- Classes ----------------------------------
// -----------------------------------------------------------------------------

// --------------------------------- Component ---------------------------------

Component::Component() = default;

Component::~Component() = default;

Component::Component(const Component&) = default;

Component::Component(Component&& other) noexcept
	: entity(other.entity) { }

Component& Component::operator=(const Component& other) {
	if (&other == this) return *this;
	entity = other.entity;
	return *this;
}

Component& Component::operator=(Component&& other) noexcept {
	if (&other == this) return *this;
	entity = other.entity;
	return *this;
}

Entity Component::GetEntity() {
	return entity;
}

void Component::SetEntity(EntityIdType entityId) {
	entity = Entity::Get(entityId);
}

// ----------------------------------- Entity ----------------------------------

Entity Entity::Create() {
	return Entity(GetEntityPool().Next());
}

Entity Entity::Get(EntityIdType entityId) {
	return Entity(entityId);
}

void Entity::DestroyEntity(Entity entity) {
	ComponentStore::RemoveAllComponents(entity.id);
	GetEntityPool().Free(entity.id);
}

Entity::Entity() = default;

Entity::Entity(EntityIdType entityId)
	: id(entityId) { }

EntityIdType Entity::GetId() const {
	return id;
}

} // namespace Junia
