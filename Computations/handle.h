#pragma once

#include <core.h>
#include <climits>

using Handle = u64;

constexpr Handle null_handle = std::numeric_limits<Handle>::max();
