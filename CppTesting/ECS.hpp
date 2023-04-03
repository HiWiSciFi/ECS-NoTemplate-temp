#pragma once

#include <cstdint>
#include <typeinfo>
#include <typeindex>
#include <functional>
#include <concepts>
#include <memory>

// TODO(HiWiSciFi): Move to external header [03-Apr-23]
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

	// TODO(HiWiSciFi): Rework not to expose these [03-Apr-23]
	std::shared_ptr<uint8_t> GetComponentTypeData(std::type_index type);
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

	class Component;

	/**
	 * @brief Wrapper for ECS Entities
	*/
	class Entity
	{
	private:
		EntityType id = 0;
		explicit Entity(EntityType entityId);

	public:
		Entity();

		[[nodiscard]] EntityType GetId() const;

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
		static Entity Get(EntityType entityId);

		/**
		 * @brief Destroy an entity
		 * @param entity The entity to destroy
		*/
		static void DestroyEntity(Entity entity);

		/**
		 * @brief Add a component
		 * @tparam T The type of the component to add
		 * @tparam ...TArgs The types of the parameters to pass to the component constructor
		 * @param ...args The parameters to pass to the component constructor
		 * @return A reference to the newly created component
		*/
		template<TypenameDerivedFrom<Component> T, typename... TArgs> T& AddComponent(TArgs... args);

		/**
		 * @brief Remove a component
		 * @tparam T The type of the component to remove
		*/
		template<TypenameDerivedFrom<Component> T> void RemoveComponent();

		/**
		 * @brief Get a component (that has been previously added)
		 * @tparam T The type of the component to get
		 * @return A reference to the component
		*/
		template<TypenameDerivedFrom<Component> T> T& GetComponent();
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

		Component(const Component&) = default;

		Component(Component&& other) noexcept
			: entity(other.entity)
		{ }

		Component& operator=(const Component& other)
		{
			if (&other == this) return *this;
			entity = other.entity;
			return *this;
		}

		Component& operator=(Component&& other) noexcept
		{
			if (&other == this) return *this;
			entity = other.entity;
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
		inline void SetEntity(EntityType entityId)
		{
			entity = Entity::Get(entityId);
		}

		/**
		 * @brief Register a component
		 * @tparam T The type of the component to register
		*/
		template<TypenameDerivedFrom<Component> T> static inline void Register();

		/**
		 * @brief Unregister a component (also removes it from all entities)
		 * @tparam T The type of the component to unregister
		*/
		template<TypenameDerivedFrom<Component> T> static inline void Unregister();
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

		explicit ComponentRef(Entity entity)
			: offset(GetComponentId(typeid(T), entity.GetId()) * sizeof(T))
		{ }

		explicit ComponentRef(T& component)
			: offset(GetComponentId(typeid(T), component.GetEntity().GetId()) * sizeof(T))
		{ }

		T* operator->()
		{
			return static_cast<T*>(static_cast<void*>(GetComponentTypeData(typeid(T)).get() + offset));
		}

		T& operator*()
		{
			return *static_cast<T*>(static_cast<void*>(GetComponentTypeData(typeid(T)).get() + offset));
		}
	};

	template<TypenameDerivedFrom<Component> T>
	inline void Component::Register()
	{
		Junia::RegisterComponent(typeid(T), sizeof(T),
			[](void* ptr) -> void
			{
				std::destroy_at<T>(static_cast<T*>(ptr));
			},
			[](void* destination, void* origin) -> void
			{
				std::construct_at<T>(static_cast<T*>(destination), *static_cast<T*>(origin));
			});
	}

	template<TypenameDerivedFrom<Component> T>
	inline void Component::Unregister()
	{
		UnregisterComponent(typeid(T));
	}

	template<TypenameDerivedFrom<Component> T, typename ...TArgs>
	inline T& Entity::AddComponent(TArgs ...args)
	{
		T* componentAddress = static_cast<T*>(Junia::AddComponent(typeid(T), id));
		std::construct_at<T>(componentAddress, args...);
		componentAddress->SetEntity(id);
		return *componentAddress;
	}

	template<TypenameDerivedFrom<Component> T>
	inline void Entity::RemoveComponent()
	{
		Junia::RemoveComponent(typeid(T), id);
	}

	template<TypenameDerivedFrom<Component> T>
	inline T& Entity::GetComponent()
	{
		return *static_cast<T*>(Junia::GetComponent(typeid(T), id));
	}
} // namespace Junia
