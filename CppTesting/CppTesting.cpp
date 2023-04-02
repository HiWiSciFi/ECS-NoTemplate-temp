#include <iostream>
#include "ECS.hpp"

class MyComponent : public Junia::Component
{
public:
	uint32_t x = 0;
	uint32_t y = 0;

	MyComponent() : x(0), y(0)
	{ }

	MyComponent(uint32_t x, uint32_t y)
		: x(x), y(y)
	{
		std::cout << "Constructing: { " << x << ", " << y << " }" << std::endl;
	}

	MyComponent(const MyComponent& other)
		: x(other.x), y(other.y)
	{
		std::cout << "Copying: { " << x << ", " << y << " }" << std::endl;
	}

	~MyComponent()
	{
		std::cout << "Destroying: { " << x << ", " << y << " }" << std::endl;
	}

	void Set(uint32_t x, uint32_t y)
	{
		uint32_t old_x = this->x;
		uint32_t old_y = this->y;
		this->x = x;
		this->y = y;
		std::cout << "Changing: { " << old_x << ", " << old_y << " } to { " << x << ", " << y << " }" << std::endl;
	}
};

int main()
{
	Junia::Component::Register<MyComponent>();

	Junia::ComponentRef<MyComponent> components[6];
	for (int i = 0; i < 6; i++)
	{
		Junia::Entity e = Junia::Entity::Create();
		components[i] = Junia::ComponentRef<MyComponent>(e.AddComponent<MyComponent>(i + 1, i + 1));
	}

	std::cout << "-----CREATED-----" << std::endl;

	components[0]->Set(10, 10);
	components[1]->Set(20, 20);
	components[2]->Set(30, 30);

	std::cout << "-----EDITED------" << std::endl;

	Junia::Component::Unregister<MyComponent>();

	std::cout << "-----DELETED-----" << std::endl;
	return 0;
}
