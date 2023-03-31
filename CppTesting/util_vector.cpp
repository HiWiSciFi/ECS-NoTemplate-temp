#include "util_vector.hpp"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <iostream>

static IdPool<EntityType> entityPool{ };

class ComponentStore
{
private:
	uint8_t* data = nullptr;
	size_t count = 0;
	size_t capacity = 0;
	size_t elementSize = 0;
	std::unordered_map<EntityType, size_t> entityToComponent{ };
	std::unordered_map<size_t, EntityType> ComponentToEntity{ };
	std::unordered_set<size_t> freeComponentIndices{ };

	std::function<void(void*)> dest;

	inline void reallocUpsize()
	{
		capacity *= 2;
		uint8_t* new_data = reinterpret_cast<uint8_t*>(realloc(data, capacity * elementSize));
		data = new_data;
	}

	inline void* GetComponentPtr(size_t id) const
	{
		std::cout << "Accessing " << id
			<< "\n\tsize: " << elementSize
			<< "\n\taddr: 0x" << std::hex << static_cast<void*>(data + (id * elementSize))
			<< std::endl;
		return data + (id * elementSize);
	}

public:
	ComponentStore()
	{ }

	ComponentStore(size_t _elementSize, std::function<void(void*)> dest, size_t count = 0)
		: elementSize(_elementSize), count(count), capacity(count), dest(dest)
	{
		std::cout << "Create Store at 0x" << std::hex << static_cast<void*>(this)
			<< "\n\tInternal Element size: " << elementSize
			<< std::endl;
		if (elementSize < 1) elementSize = 1;
		if (capacity < 2) capacity = 2;
		data = reinterpret_cast<uint8_t*>(malloc(capacity * elementSize));
	}

	/*ComponentStore(const ComponentStore& other)
	{
		std::cout << "copied" << std::endl;
		if (other.count > 0) throw std::runtime_error("you may not copy used component stores");
		count = other.count;
		capacity = other.capacity;
		elementSize = other.elementSize;
		entityToComponent = other.entityToComponent;
		ComponentToEntity = other.ComponentToEntity;
		freeComponentIndices = other.freeComponentIndices;
		dest = other.dest;
		if (elementSize < 1) elementSize = 1;
		if (capacity < 2) capacity = 2;
		data = reinterpret_cast<uint8_t*>(malloc(capacity * elementSize));
	}*/

	~ComponentStore()
	{
		std::cout << "Destroy Store at 0x" << std::hex << static_cast<void*>(this)
			<< "\n\tInternal Element size: " << elementSize
			<< std::endl;
		RemoveAll();
		if (capacity > 0) free(data);
		capacity = 0;
	}

	void* AllocateComponent(EntityType e)
	{
		if (HasComponent(e)) throw std::runtime_error("entity already has component");
		size_t id;
		if (!freeComponentIndices.empty())
		{
			auto iterator = freeComponentIndices.begin();
			id = *iterator;
			freeComponentIndices.erase(iterator);
		}
		else
		{
			if (capacity < count + 1) reallocUpsize();
			id = count;
			count++;
		}
		entityToComponent[e] = id;
		ComponentToEntity[id] = e;
		void* compPtr = GetComponentPtr(id);
		std::cout << "Constructing addr: 0x" << std::hex << compPtr << std::endl;
		std::cout << "\tArray size: " << (count * elementSize) << "\n\tElement size: " << elementSize << std::endl;
		return compPtr;
	}

	void RemoveComponent(EntityType e)
	{
		if (!HasComponent(e)) throw std::runtime_error("entity does not have component");
		size_t id = entityToComponent.at(e);
		GetComponentPtr(id);
		entityToComponent.erase(e);
		ComponentToEntity.erase(id);
		if (id == count - 1) count--;
		else freeComponentIndices.insert(id);
	}

	void* GetComponent(EntityType e) const
	{
		return GetComponentPtr(entityToComponent.at(e));
	}

	bool HasComponent(EntityType e) const
	{
		return entityToComponent.contains(e);
	}

	void RemoveAll()
	{
		for (size_t i = 0; i < count; i++)
		{
			if (!freeComponentIndices.contains(i))
			{
				void* compPtr = GetComponentPtr(i);
				std::cout << "Destroying addr: 0x" << std::hex << compPtr << std::endl;
				dest(compPtr);
			}
		}
		entityToComponent.clear();
		ComponentToEntity.clear();
		count = 0;
	}
};

std::unordered_map<std::type_index, ComponentStore*> componentStores{ };

ComponentStore& GetComponentStore(std::type_index type)
{
	return *componentStores.at(type);
}

Component::~Component()
{ }

EntityType CreateEntity()
{
	return entityPool.Next();
}

void DestroyEntity(EntityType e)
{
	entityPool.Free(e);
}

void RegisterComponent(std::type_index type, size_t size, std::function<void(void*)> dest)
{
	std::cout << "1- RegisterComponent(type, size: " << size << ", dest)" << std::endl;
	//componentStores[type] = new ComponentStore(size, dest);
	componentStores.insert({type, new ComponentStore(size, dest)});
	std::cout << "2- RegisterComponent(type, size: " << size << ", dest)" << std::endl;
}

void UnregisterComponent(std::type_index type)
{
	std::cout << "Component Count: " << componentStores.size() << std::endl;
	delete &GetComponentStore(type);
	componentStores.erase(type);
	std::cout << "Component Count: " << componentStores.size() << std::endl;
}

void* AddComponent(std::type_index type, EntityType e)
{
	return GetComponentStore(type).AllocateComponent(e);
}

void RemoveComponent(std::type_index type, EntityType e)
{
	GetComponentStore(type).RemoveComponent(e);
}

bool HasComponent(std::type_index type, EntityType e)
{
	return GetComponentStore(type).HasComponent(e);
}

void* GetComponent(std::type_index type, EntityType e)
{
	return GetComponentStore(type).GetComponent(e);
}
