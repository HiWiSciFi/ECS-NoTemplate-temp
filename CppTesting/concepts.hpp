#pragma once

#include <type_traits>

namespace Junia
{
	template<typename T, typename Base>
	concept TypenameDerivedFrom = std::is_base_of<Base, T>::value;
}
