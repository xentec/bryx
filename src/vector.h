#pragma once

#include "global.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <initializer_list>

struct vec
{
	inline vec(i32 init = 0)
	{
		for(i32& e : data)
			e = init;
	}

	inline vec(const vec& other):
		blob(other.blob)
	{}

	inline vec(std::initializer_list<i32> list)
	{
		auto i = list.begin();
		x = *i++;
		y = *i;
	}

	inline i32 operator[](usz index) const
	{
		return data[index];
	}

	inline i32& operator[](usz index)
	{
		return data[index];
	}

	inline bool operator ==(const vec& other) const
	{
		return blob == other.blob;
	}

	inline bool operator !=(const vec& other) const
	{
		return !(blob == other.blob);
	}

	inline vec operator -() const
	{
		return { -x, -y };
	}

	inline vec operator +(const vec& other) const
	{
		return { x + other.x, y+other.y };
	}

	inline vec operator -(const vec& other) const
	{
		return *this + -other;
	}

	inline vec operator *(i32 other) const
	{
		return { x*other, y*other };
	}

	inline i32 operator *(const vec& other) const
	{
		return x*other.x + y*other.y;
	}

	inline string asString() const
	{
		return fmt::format("[{}, {}]", x, y);
	}

	inline friend std::ostream& operator<<(std::ostream &os, const vec &vec)
	{
		return os << vec.asString();
	}

	union
	{
		u64 blob;
		i32 data[2];
		struct {
			i32 x, y;
		};
	};
};

namespace std {
	template<>
	struct hash<vec>
	{
		typedef vec argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& o) const
		{
			return o.x ^ (o.y << 1);
		}
	};
}


inline bool inBox(const vec& x, const vec& begin, const vec& end)
{
	return begin.x <= x.x && x.x <= end.x && begin.y <= x.y && x.y <= end.y;
}
