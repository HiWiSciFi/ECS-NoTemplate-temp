#include "ECS.hpp"

#include <array>
#include <iostream>

class MyComponent : public Junia::Component {
public:
	uint32_t x = 0;
	uint32_t y = 0;

	MyComponent() = default;

	MyComponent(uint32_t x, uint32_t y)
		: x(x), y(y) { }

	~MyComponent() override {
		std::cout << "Destroying { " << x << ", " << y << " }" << std::endl;
	}

	void Set(uint32_t x, uint32_t y) {
		this->x = x;
		this->y = y;
	}
};

constexpr size_t componentCount = 6;

int main() {
	Junia::Component::Register<MyComponent>(componentCount);

	std::array<Junia::ComponentRef<MyComponent>, componentCount> components;
	for (auto& component : components) {
		component = Junia::ComponentRef<MyComponent>(
			Junia::Entity::Create().AddComponent<MyComponent>());
	}

	const uint32_t scaleFactor = 10;
	for (auto& component : components) {
		component->Set(
			(component->GetEntity().GetId() + 1) * scaleFactor,
			(component->GetEntity().GetId() + 1) * scaleFactor);
	}

	Junia::Component::Unregister<MyComponent>();

	return 0;
}
