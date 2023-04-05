#pragma once

#include "concepts.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <typeindex>
#include <typeinfo>

namespace Junia {

// -----------------------------------------------------------------------------
// ----------------------------- Using declarations ----------------------------
// -----------------------------------------------------------------------------

/**
 * @brief Type for EntityIDs
*/
using EntityIdType = uint32_t;

/**
 * @brief Type for ComponentIDs
*/
using ComponentIdType = size_t;

/**
 * @brief A function calling the destructor for the passed in pointer (actual
 *        pointer type context dependent)
*/
using DestructorFunc = std::function<void(void*)>;

/**
 * @brief A function calling the copy constructor to copy the object from the
 *        second parameter to the first parameter
*/
using CopyConstructorFunc = std::function<void(void*, void*)>;

// -----------------------------------------------------------------------------
// --------------------------------- Functions ---------------------------------
// -----------------------------------------------------------------------------

size_t GetComponentOffset(std::type_index type, EntityIdType entity);
void* GetComponentByOffset(std::type_index type, size_t offset);

/**
 * @brief Register a component type
 * @param type The component type
 * @param size the size of the type in bytes
 * @param destructor A function calling the destructor for an instance of type
 * @param copyConstructor A function calling the copy constructor for type
*/
void RegisterComponent(std::type_index type, size_t size, size_t preallocCount,
	DestructorFunc destructor, CopyConstructorFunc copyConstructor);

/**
 * @brief Unregister a component type
 * @param type The component type
*/
void UnregisterComponent(std::type_index type);

/**
 * @brief Add a component to an entity (only allocates! use std::construct_at()
 *        to initialize memory)
 * @param type The component type to add
 * @param entity The id of the entity to add the component to
 * @return A pointer to the start of the memory where the component can be
 *         constructed
*/
void* AddComponent(std::type_index type, EntityIdType entity);

/**
 * @brief Remove a component from an entity (also calls destructor on the
 *        memory)
 * @param type The component type to remove
 * @param entity The id of the entity to remove the component from
*/
void RemoveComponent(std::type_index type, EntityIdType entity);

/**
 * @brief Get the component for an entity
 * @param type The component type to get
 * @param entity The id of the entity to get the component from
 * @return A pointer to the first byte of memory of the component
*/
void* GetComponent(std::type_index type, EntityIdType entity);

// -----------------------------------------------------------------------------
// ---------------------------------- Classes ----------------------------------
// -----------------------------------------------------------------------------

// Forward declaration for use in Enity class
class Component;

/**
 * @brief Wrapper for ECS Entities
*/
class Entity {
private:
	/**
	 * @brief The ID of the entity
	*/
	EntityIdType id = 0;

	/**
	 * @brief Entity constructor by ID (does not register entity), see
	 *        Junia::Entity::Create() for entity creation
	 * @param entityId The ID of the entity
	*/
	explicit Entity(EntityIdType entityId);

public:
	/**
	 * @brief Entity constructor for temporary placeholders (not safe for use),
	 *        see Junia::Entity::Create() for entity creation
	*/
	Entity();

	/**
	 * @brief Get the ID
	 * @return The ID of the entity
	*/
	[[nodiscard]] EntityIdType GetId() const;

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
	static Entity Get(EntityIdType entityId);

	/**
	 * @brief Destroy an entity
	 * @param entity The entity to destroy
	*/
	static void DestroyEntity(Entity entity);

	/**
	 * @brief Add a component
	 * @tparam T The type of the component to add
	 * @tparam ...TArgs The types of the parameters to pass to the component
	 *                  constructor
	 * @param ...args The parameters to pass to the component constructor
	 * @return A reference to the newly created component
	*/
	template<TypenameDerivedFrom<Component> T, typename... TArgs>
	T& AddComponent(TArgs... args);

	/**
	 * @brief Remove a component
	 * @tparam T The type of the component to remove
	*/
	template<TypenameDerivedFrom<Component> T>
	void RemoveComponent();

	/**
	 * @brief Get a component (that has been previously added)
	 * @tparam T The type of the component to get
	 * @return A reference to the component
	*/
	template<TypenameDerivedFrom<Component> T>
	T& GetComponent();
};

/**
 * @brief Abstract class for deriving components from
*/
class Component {
private:
	/**
	 * @brief The entity this component is attached to
	*/
	Entity entity{ };

public:
	Component();
	virtual ~Component() = 0;
	Component(const Component& other);
	Component(Component&& other) noexcept;

	Component& operator=(const Component& other);
	Component& operator=(Component&& other) noexcept;

	/**
	 * @brief Get the entity this component is attached to
	 * @return An Entity instace wrapping the entity
	*/
	Entity GetEntity();

	/**
	 * @brief INTERNAL USE ONLY - THIS ONLY SETS THE MEMBER
	 * @param id The id of the entity
	*/
	void SetEntity(EntityIdType entityId);

	/**
	 * @brief Register a component
	 * @tparam T The type of the component to register
	*/
	template<TypenameDerivedFrom<Component> T>
	static inline void Register(size_t preallocCount = 1);

	/**
	 * @brief Unregister a component (also removes it from all entities)
	 * @tparam T The type of the component to unregister
	*/
	template<TypenameDerivedFrom<Component> T>
	static inline void Unregister();
};

/**
 * @brief A reference to a component (stays valid after reallocation)
 * @tparam T The type of the component to reference
*/
template<TypenameDerivedFrom<Component> T>
struct ComponentRef {
private:
	/**
	 * @brief The offset of the component in the ComponentStore memory
	*/
	size_t offset;

public:
	/**
	 * @brief Constructor for placeholder instances (see other constructors for
	 *        ComponentRef creation)
	*/
	ComponentRef();
	explicit ComponentRef(Entity entity);
	explicit ComponentRef(T& component);

	T* operator->();
	T& operator*();
};

// -----------------------------------------------------------------------------
// ------------------------------ Implementations ------------------------------
// -----------------------------------------------------------------------------

// ----------------------------------- Entity ----------------------------------

template<TypenameDerivedFrom<Component> T, typename ...TArgs>
inline T& Entity::AddComponent(TArgs ...args) {
	T* componentAddress = static_cast<T*>(Junia::AddComponent(typeid(T), id));
	std::construct_at<T>(componentAddress, args...);
	componentAddress->SetEntity(id);
	return *componentAddress;
}

template<TypenameDerivedFrom<Component> T>
inline void Entity::RemoveComponent() {
	Junia::RemoveComponent(typeid(T), id);
}

template<TypenameDerivedFrom<Component> T>
inline T& Entity::GetComponent() {
	return *static_cast<T*>(Junia::GetComponent(typeid(T), id));
}

// --------------------------------- Component ---------------------------------

template<TypenameDerivedFrom<Component> T>
inline void Component::Register(size_t preallocCount) {
	Junia::RegisterComponent(typeid(T), sizeof(T), preallocCount,
		[](void* ptr) -> void {
			std::destroy_at<T>(static_cast<T*>(ptr));
		},
		[](void* destination, void* origin) -> void {
			std::construct_at<T>(
				static_cast<T*>(destination),
				*static_cast<T*>(origin));
		});
}

template<TypenameDerivedFrom<Component> T>
inline void Component::Unregister() {
	UnregisterComponent(typeid(T));
}

// ------------------------------ ComponentRef<T> ------------------------------

template<TypenameDerivedFrom<Component> T>
inline ComponentRef<T>::ComponentRef()
	: offset(0) { }

template<TypenameDerivedFrom<Component> T>
inline ComponentRef<T>::ComponentRef(Entity entity)
	: offset(GetComponentOffset(typeid(T), entity.GetId())) { }

template<TypenameDerivedFrom<Component> T>
inline ComponentRef<T>::ComponentRef(T& component)
	: offset(GetComponentOffset(typeid(T),
		component.GetEntity().GetId())) { }

template<TypenameDerivedFrom<Component> T>
inline T* ComponentRef<T>::operator->() {
	return static_cast<T*>(GetComponentByOffset(typeid(T), offset));
}

template<TypenameDerivedFrom<Component> T>
inline T& ComponentRef<T>::operator*() {
	return *static_cast<T*>(GetComponentByOffset(typeid(T), offset));
}

} // namespace Junia
