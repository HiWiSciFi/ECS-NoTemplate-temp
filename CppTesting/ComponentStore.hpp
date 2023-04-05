#pragma once

#include "ECS.hpp"

#include <functional>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

namespace Junia {

class ComponentStore {
private:
	using ComponentStoreMapType = std::unordered_map<std::type_index, std::shared_ptr<ComponentStore>>;

	static ComponentStoreMapType& GetComponentStores();

	std::unordered_map<EntityIdType, ComponentIdType> entityToComponentMap{ };
	std::unordered_set<ComponentIdType> freeComponentIds{ };
	size_t elementSize = 0;
	DestructorFunc destructor;
	CopyConstructorFunc copyConstructor;
	size_t count = 0;
	size_t capacity = 0;
	std::shared_ptr<uint8_t> data = nullptr;

	void ReallocUpsize();

public:
	static void Create(std::type_index type, size_t size, size_t preallocCount,
		DestructorFunc destructor, CopyConstructorFunc copyConstructor);
	static void Destroy(std::type_index type);
	static std::shared_ptr<ComponentStore> Get(std::type_index type);
	static void RemoveAllComponents(EntityIdType entity);

	ComponentStore(size_t size, size_t preallocCount, DestructorFunc destructor, CopyConstructorFunc copyConstructor);
	ComponentStore(const ComponentStore& other);
	ComponentStore(ComponentStore&& other) noexcept;
	~ComponentStore();

	ComponentStore& operator=(const ComponentStore& other);
	ComponentStore& operator=(ComponentStore&& other) noexcept;

	void* AllocateComponent(EntityIdType entity);
	void RemoveComponent(EntityIdType entity);
	void* GetComponent(EntityIdType entity);

	size_t GetComponentOffset(EntityIdType entity);
	void* GetComponentByOffset(size_t offset);
};

}  // namespace Junia
