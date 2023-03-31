#include <iostream>
//#include "util_vector.hpp"
#include "ECS.hpp"

class MyComponent : public Component
{
public:
	uint32_t x = 0;
	uint32_t y = 0;

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
};

int main()
{
	RegisterComponent<MyComponent>();

	MyComponent components[] = {
		Entity::Create().AddComponent<MyComponent>(1, 1),
		Entity::Create().AddComponent<MyComponent>(2, 2),
		Entity::Create().AddComponent<MyComponent>(3, 3),
		Entity::Create().AddComponent<MyComponent>(4, 4),
		Entity::Create().AddComponent<MyComponent>(5, 5),
		Entity::Create().AddComponent<MyComponent>(6, 6)
	};

	UnregisterComponent<MyComponent>();
	return 0;
}
