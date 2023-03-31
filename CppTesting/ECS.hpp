#pragma once

#include <cstdint>
#include <typeinfo>
#include <typeindex>
#include <functional>

using EntityType = uint32_t;

class Component
{
public:
	Component() = default;
	virtual ~Component() = 0;
	Component(const Component& other) = default;
	Component(const Component&& other) noexcept { }
	Component& operator=(const Component& other)
	{
		if (&other == this)
		{
			return *this;
		}
		return *this;
	}
	Component& operator=(Component&& other) noexcept
	{
		if (&other == this)
		{
			return *this;
		}
		return *this;
	}
};

void  RegisterComponent   (std::type_index type, size_t size, std::function<void(void*)> destructor, std::function<void(void*, void*)> copyConstructor);
void  UnregisterComponent (std::type_index type);
void* AddComponent        (std::type_index type, EntityType entity);
void  RemoveComponent     (std::type_index type, EntityType entity);
void* GetComponent        (std::type_index type, EntityType entity);

template<typename T>                    inline void RegisterComponent   ();
template<typename T>                    inline void UnregisterComponent ();

class Entity
{
public:
	EntityType id = 0;

private:
	Entity(EntityType id);

public:
	static Entity Create();
	static void DestroyEntity(Entity entity);

public:
	template<typename T, typename... TArgs> void AddComponent(TArgs... args);
	template<typename T> void RemoveComponent();
	template<typename T> T& GetComponent();
};

template<typename T>
inline void RegisterComponent()
{
	RegisterComponent(typeid(T), sizeof(T),
		[](void* ptr) -> void
		{
			std::destroy_at<T>(reinterpret_cast<T*>(ptr));
		},
		[](void* destination, void* origin) -> void
		{
			std::construct_at<T>(reinterpret_cast<T*>(destination), *reinterpret_cast<T*>(origin));
		});
}

template<typename T>
inline void UnregisterComponent()
{
	UnregisterComponent(typeid(T));
}

template<typename T, typename... TArgs>
inline void Entity::AddComponent(TArgs... args)
{
	T* componentAddress = reinterpret_cast<T*>(AddComponent(typeid(T), id));
	std::construct_at<T>(componentAddress, args...);
}

template<typename T>
inline void Entity::RemoveComponent()
{
	RemoveComponent(typeid(T), id);
}

template<typename T>
inline T& Entity::GetComponent()
{
	return *static_cast<T*>(GetComponent(typeid(T), id));
}
