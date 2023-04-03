#include "ECS.hpp"
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <stdexcept>
#include "gsl.hpp"
#include "IdPool.hpp"

namespace Junia
{
	Component::~Component() = default;

	static void DeleteByteArrayCallback(gsl::owner<const uint8_t*> ptr)
	{
		delete[] ptr;
	}

	class ComponentStore
	{
	public:
		static void Create(std::type_index type, size_t size, std::function<void(void*)> destructor, std::function<void(void*, void*)> copyConstructor)
		{
			GetComponentStores()[type] = std::make_shared<ComponentStore>(size, std::move(destructor), std::move(copyConstructor));
		}

		static void Destroy(std::type_index type)
		{
			GetComponentStores().erase(type);
		}

		static std::shared_ptr<ComponentStore> Get(std::type_index type)
		{
			return GetComponentStores().at(type);
		}

		static void RemoveAllComponents(EntityType entity)
		{
			for (auto& componentStorePair : GetComponentStores())
				componentStorePair.second->RemoveComponent(entity);
		}

	private:
		static inline std::unordered_map<std::type_index, std::shared_ptr<ComponentStore>>& GetComponentStores()
		{
			static std::unordered_map<std::type_index, std::shared_ptr<ComponentStore>> componentStores{ };
			return componentStores;
		}

	public:
		ComponentStore(size_t size, std::function<void(void*)> destructor, std::function<void(void*, void*)> copyConstructor)
			: elementSize(size), destructor(std::move(destructor)), copyConstructor(std::move(copyConstructor)), capacity(2),
			data(new uint8_t[capacity * elementSize], DeleteByteArrayCallback)
		{ }

		ComponentStore(const ComponentStore& other)
			: entityToComponentMap(other.entityToComponentMap), freeComponentIds(other.freeComponentIds), elementSize(other.elementSize), destructor(other.destructor),
			copyConstructor(other.copyConstructor), count(other.count), capacity(other.capacity),
			data(new uint8_t[capacity * elementSize], DeleteByteArrayCallback)
		{
			for (ComponentId i = 0; i < count; i++)
			{
				if (freeComponentIds.contains(i)) continue;
				copyConstructor(data.get() + (i * elementSize), other.data.get() + (i * elementSize));
			}
		}

		ComponentStore(ComponentStore&& other) noexcept
			: entityToComponentMap(std::move(other.entityToComponentMap)), freeComponentIds(std::move(other.freeComponentIds)), elementSize(other.elementSize), destructor(std::move(other.destructor)),
			copyConstructor(std::move(other.copyConstructor)), count(other.count), capacity(other.capacity), data(std::move(other.data))
		{
			other.capacity = 0;
			other.count = 0;
			other.data = nullptr;
		}

		ComponentStore& operator=(const ComponentStore& other)
		{
			if (&other == this) return *this;
			if (capacity > 0 && data != nullptr)
			{
				for (ComponentId i = 0; i < count; i++)
				{
					if (freeComponentIds.contains(i)) continue;
					destructor(data.get() + (i * elementSize));
				}
				entityToComponentMap.clear();
				capacity = 0;
				count = 0;
			}
			entityToComponentMap = other.entityToComponentMap;
			freeComponentIds = other.freeComponentIds;
			elementSize = other.elementSize;
			destructor = other.destructor;
			copyConstructor = other.copyConstructor;
			count = other.count;
			capacity = other.capacity;
			data = std::shared_ptr<uint8_t>(new uint8_t[capacity * elementSize], DeleteByteArrayCallback);
			for (ComponentId i = 0; i < count; i++)
			{
				if (freeComponentIds.contains(i)) continue;
				copyConstructor(data.get() + (i * elementSize), other.data.get() + (i * elementSize));
			}
			return *this;
		}

		ComponentStore& operator=(ComponentStore&& other) noexcept
		{
			if (capacity > 0 && data != nullptr)
			{
				for (ComponentId i = 0; i < count; i++)
				{
					if (freeComponentIds.contains(i)) continue;
					destructor(data.get() + (i * elementSize));
				}
				entityToComponentMap.clear();
				capacity = 0;
				count = 0;
			}
			entityToComponentMap = std::move(other.entityToComponentMap);
			freeComponentIds = std::move(other.freeComponentIds);
			elementSize = other.elementSize;
			destructor = std::move(other.destructor);
			copyConstructor = std::move(other.copyConstructor);
			count = other.count;
			capacity = other.capacity;
			data = std::move(other.data);
			other.capacity = 0;
			other.count = 0;
			other.data = nullptr;
			return *this;
		}

		~ComponentStore()
		{
			for (ComponentId i = 0; i < count; i++)
			{
				if (freeComponentIds.contains(i)) continue;
				destructor(data.get() + (i * elementSize));
			}
		}

		std::shared_ptr<uint8_t> GetData()
		{
			return data;
		}

		void* AllocateComponent(EntityType entity)
		{
			if (entityToComponentMap.contains(entity))
				throw std::runtime_error("entity already has component");

			ComponentId newComponentId = count;
			if (!freeComponentIds.empty())
			{
				auto iterator = freeComponentIds.begin();
				newComponentId = *iterator;
				freeComponentIds.erase(iterator);
			}
			else
			{
				if (capacity < count + 1) ReallocUpsize();
				count++;
			}
			entityToComponentMap[entity] = newComponentId;
			return data.get() + (newComponentId * elementSize);
		}

		void RemoveComponent(EntityType entity)
		{
			if (!entityToComponentMap.contains(entity)) return;
			const ComponentId componentId = entityToComponentMap.at(entity);
			destructor(data.get() + (componentId * elementSize));
			entityToComponentMap.erase(entity);
			freeComponentIds.insert(componentId);
			if (componentId == count - 1) count--;
		}

		void* GetComponent(EntityType entity)
		{
			const ComponentId componentId = entityToComponentMap.at(entity);
			return data.get() + (componentId * elementSize);
		}

		ComponentId GetComponentId(EntityType entity)
		{
			return entityToComponentMap.at(entity);
		}

	private:
		std::unordered_map<EntityType, ComponentId> entityToComponentMap{ };
		std::unordered_set<ComponentId> freeComponentIds{ };
		size_t elementSize = 0;
		std::function<void(void*)> destructor;
		std::function<void(void*, void*)> copyConstructor;
		size_t count = 0;
		size_t capacity = 0;
		std::shared_ptr<uint8_t> data = nullptr;

		void ReallocUpsize()
		{
			capacity *= 2;
			if (capacity == 0) capacity = 2;
			const std::shared_ptr<uint8_t> new_data(new uint8_t[capacity * elementSize], DeleteByteArrayCallback);

			for (ComponentId i = 0; i < count; i++)
			{
				if (freeComponentIds.contains(i)) continue;
				void* old_comp = data.get() + (i * elementSize);
				copyConstructor(new_data.get() + (i * elementSize), old_comp);
				destructor(old_comp);
			}
			data = new_data;
		}
	};

	void RegisterComponent(std::type_index type, size_t size, std::function<void(void*)> destructor, std::function<void(void*, void*)> copyConstructor)
	{
		ComponentStore::Create(type, size, std::move(destructor), std::move(copyConstructor));
	}

	void UnregisterComponent(std::type_index type)
	{
		ComponentStore::Destroy(type);
	}

	std::shared_ptr<uint8_t> GetComponentTypeData(std::type_index type)
	{
		return ComponentStore::Get(type)->GetData();
	}

	ComponentId GetComponentId(std::type_index type, EntityType entity)
	{
		return ComponentStore::Get(type)->GetComponentId(entity);
	}

	void* AddComponent(std::type_index type, EntityType entity)
	{
		return ComponentStore::Get(type)->AllocateComponent(entity);
	}

	void RemoveComponent(std::type_index type, EntityType entity)
	{
		ComponentStore::Get(type)->RemoveComponent(entity);
	}

	void* GetComponent(std::type_index type, EntityType entity)
	{
		return ComponentStore::Get(type)->GetComponent(entity);
	}

	static inline IdPool<EntityType>& GetEntityPool()
	{
		static IdPool<EntityType> pool = IdPool<EntityType>();
		return pool;
	}

	Entity Entity::Create()
	{
		return Entity(GetEntityPool().Next());
	}

	Entity Entity::Get(EntityType entityId)
	{
		return Entity(entityId);
	}

	void Entity::DestroyEntity(Entity entity)
	{
		ComponentStore::RemoveAllComponents(entity.id);
		GetEntityPool().Free(entity.id);
	}

	Entity::Entity(EntityType entityId) : id(entityId)
	{ }

	Entity::Entity() = default;

	EntityType Entity::GetId() const
	{
		return id;
	}
} // namespace Junia
