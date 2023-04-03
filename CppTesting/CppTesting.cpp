#include <iostream>
#include <array>
#include "ECS.hpp"

class MyComponent : public Junia::Component
{
public:
	uint32_t x = 0;
	uint32_t y = 0;

	MyComponent() = default;

	MyComponent(uint32_t x, uint32_t y)
		: x(x), y(y)
	{ }

	void Set(uint32_t x, uint32_t y)
	{
		const uint32_t old_x = this->x;
		const uint32_t old_y = this->y;
		this->x = x;
		this->y = y;
		std::cout << "Changing: { " << old_x << ", " << old_y << " } to { " << x << ", " << y << " }" << std::endl;
	}
};

int main()
{
	Junia::Component::Register<MyComponent>();

	const size_t componentCount = 6;
	std::array<Junia::ComponentRef<MyComponent>, componentCount> components;
	for (auto& component : components)
		component = Junia::ComponentRef<MyComponent>(Junia::Entity::Create().AddComponent<MyComponent>());

	std::cout << "-----CREATED-----" << std::endl;

	const uint32_t scaleFactor = 10;
	for (auto& component : components)
	{
		component->Set(component->x * scaleFactor, component->y * scaleFactor);
	}

	std::cout << "-----EDITED------" << std::endl;

	Junia::Component::Unregister<MyComponent>();

	std::cout << "-----DELETED-----" << std::endl;

	return 0;
}
