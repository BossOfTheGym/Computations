#pragma once

#include <core.h>
#include <limits>

using id_t = uint;

constexpr const id_t null_id = std::numeric_limits<id_t>::max();

template<class ... Types>
class type_id
{
	inline static id_t base = 0;
	
public:
	template<class T = void>
	inline static const id_t id = ++base;
};
