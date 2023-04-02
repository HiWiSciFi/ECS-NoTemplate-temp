#pragma once

#include <cstdint>
#include <typeinfo>
#include <typeindex>
#include <functional>
#include <concepts>

template<typename T, typename Base>
concept TypenameDerivedFrom = std::is_base_of<Base, T>::value;

namespace Junia
{
	using EntityType = uint32_t;
	using ComponentId = size_t;

	/**
	 * @brief Register a component type
	 * @param type The component type
	 * @param size the size of the type in bytes
	 * @param destructor A function calling the destructor for an instance of type
	 * @param copyConstructor A function calling the copy constructor for type
	*/
	void RegisterComponent(std::type_index type, size_t size, std::function<void(void*)> destructor, std::function<void(void*, void*)> copyConstructor);

	/**
	 * @brief Unregister a component type
	 * @param type The component type
	*/
	void UnregisterComponent(std::type_index type);

	// TODO: rework
	uint8_t*    GetComponentTypeData(std::type_index type);
	ComponentId GetComponentId(std::type_index type, EntityType entity);

	/**
	 * @brief Add a component to an entity (only allocates! use std::construct_at() to initialize memory)
	 * @param type The component type to add
	 * @param entity The id of the entity to add the component to
	 * @return A pointer to the start of the memory where the component can be constructed
	*/
	void* AddComponent(std::type_index type, EntityType entity);

	/**
	 * @brief Remove a component from an entity (also calls destructor on the memory)
	 * @param type The component type to remove
	 * @param entity The id of the entity to remove the component from
	*/
	void RemoveComponent(std::type_index type, EntityType entity);

	/**
	 * @brief Get the component for an entity
	 * @param type The component type to get
	 * @param entity The id of the entity to get the component from
	 * @return A pointer to the first byte of memory of the component
	*/
	void* GetComponent(std::type_index type, EntityType entity);

	/**
	 * @brief Wrapper for ECS Entities
	*/
	class Entity
	{
	public:
		EntityType id = 0;
		Entity();

	private:
		Entity(EntityType id);

	public:
		/**
		 * @brief Create an entity
		 * @return An Entity instance wrapping the created entity
		*/
		static Entity Create();

		/**
		 * @brief Get an entity by id
		 * @param id The id of the entity to get
		 * @return An entity instance wrapping the entity
		*/
		static Entity Get(EntityType id);

		/**
		 * @brief Destroy an entity
		 * @param entity The entity to destroy
		*/
		static void DestroyEntity(Entity entity);

	public:

		/**
		 * @brief Add a component
		 * @tparam T The type of the component to add
		 * @tparam ...TArgs The types of the parameters to pass to the component constructor
		 * @param ...args The parameters to pass to the component constructor
		 * @return A reference to the newly created component
		*/
		template<typename T, typename... TArgs> T& AddComponent(TArgs... args);

		/**
		 * @brief Remove a component
		 * @tparam T The type of the component to remove
		*/
		template<typename T> void RemoveComponent();

		/**
		 * @brief Get a component (that has been previously added)
		 * @tparam T The type of the component to get
		 * @return A reference to the component
		*/
		template<typename T> T& GetComponent();
	};

	/**
	 * @brief Abstract class for deriving components from
	*/
	class Component
	{
	private:
		Entity entity{ };

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

		/**
		 * @brief Get the entity this component is attached to
		 * @return An Entity instace wrapping the entity
		*/
		inline Entity GetEntity()
		{
			return entity;
		}

		/**
		 * @brief INTERNAL USE ONLY - THIS ONLY SETS THE MEMBER
		 * @param id The id of the entity
		*/
		inline void SetEntity(EntityType id)
		{
			entity = Entity::Get(id);
		}

	public:
		/**
		 * @brief Register a component
		 * @tparam T The type of the component to register
		*/
		template<typename T> static inline void Register();

		/**
		 * @brief Unregister a component (also removes it from all entities)
		 * @tparam T The type of the component to unregister
		*/
		template<typename T> static inline void Unregister();
	};

	/**
	 * @brief A reference to a component (stays valid after reallocation)
	 * @tparam T The type of the component to reference
	*/
	template<TypenameDerivedFrom<Component> T>
	struct ComponentRef
	{
	private:
		size_t offset;

	public:
		ComponentRef() : offset(0)
		{ }

		ComponentRef(Entity entity)
			: offset(GetComponentId(typeid(T), entity.id) * sizeof(T))
		{ }

		ComponentRef(T& component)
			: offset(GetComponentId(typeid(T), component.GetEntity().id) * sizeof(T))
		{ }

		T* operator->()
		{
			return reinterpret_cast<T*>(GetComponentTypeData(typeid(T)) + offset);
		}

		T& operator*()
		{
			return *reinterpret_cast<T*>(GetComponentTypeData(typeid(T)) + offset);
		}
	};

	template<typename T>
	inline void Component::Register()
	{
		Junia::RegisterComponent(typeid(T), sizeof(T),
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
	inline void Component::Unregister()
	{
		UnregisterComponent(typeid(T));
	}

	template<typename T, typename ...TArgs>
	inline T& Entity::AddComponent(TArgs ...args)
	{
		T* componentAddress = static_cast<T*>(Junia::AddComponent(typeid(T), id));
		std::construct_at<T>(componentAddress, args...);
		return *componentAddress;
	}

	template<typename T>
	inline void Entity::RemoveComponent()
	{
		Junia::RemoveComponent(typeid(T), id);
	}

	template<typename T>
	inline T& Entity::GetComponent()
	{
		return *static_cast<T*>(Junia::GetComponent(typeid(T), id));
	}
}
