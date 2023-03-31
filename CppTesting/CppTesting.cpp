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

	~MyComponent()
	{
		std::cout << "Destroying: { " << x << ", " << y << " }" << std::endl;
	}
};

int main()
{
	RegisterComponent<MyComponent>();

	Entity::Create().AddComponent<MyComponent>(1, 1);
	Entity::Create().AddComponent<MyComponent>(2, 2);
	Entity::Create().AddComponent<MyComponent>(3, 3);
	Entity::Create().AddComponent<MyComponent>(4, 4);
	Entity::Create().AddComponent<MyComponent>(5, 5);
	Entity::Create().AddComponent<MyComponent>(6, 6);

	UnregisterComponent<MyComponent>();
	return 0;
}

//static void destComponent(void* ptr)
//{
//	MyComponent* m = reinterpret_cast<MyComponent*>(ptr);
//	std::cout << "D: { " << m->x << ", " << m->y << " }" << std::endl;
//	m->~MyComponent();
//}

//int main()
//{
//	//RegisterComponent(typeid(MyComponent), sizeof(MyComponent), destComponent);
//	size_t compSize = sizeof(MyComponent);
//	std::cout << "Component Size: " << compSize << std::endl;
//	RegisterComponent<MyComponent>();
//	std::cout << "Component Size: " << compSize << std::endl;
//	EntityType e1 = CreateEntity();
//	EntityType e2 = CreateEntity();
//	//EntityType e3 = CreateEntity();
//	AddComponent<MyComponent>(e1, 1, 1);
//	AddComponent<MyComponent>(e2, 2, 2);
//	//AddComponent<MyComponent>(e3, 3, 3);
//	//MyComponent& component = GetComponent<MyComponent>(e1);
//
//	UnregisterComponent<MyComponent>();
//}
