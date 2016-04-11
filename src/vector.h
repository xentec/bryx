#pragma once

#include "global.h"

#include <cppformat/format.h>
#include <istream>
#include <initializer_list>

template<usz N, class T>
struct Vector
{
	using vec = Vector<N,T>;

	Vector(T init = 0)
	{
		for(T& e : data)
			e = init;
	}

	Vector(const vec& other)
	{
		for(u32 i = 0; i < N; i++)
			data[i] = other[i];
	}

	Vector(std::initializer_list<T> list)
	{
		if(list.size() != N)
			throw std::logic_error("element number != vector size");

		usz i = 0;
		for(T e : list)
			data[i++] = e;
	}

	T operator[](usz index) const
	{
		return data[index];
	}

	T& operator[](usz index)
	{
		return data[index];
	}

	bool operator ==(const vec& other) const
	{
		for(u32 i = 0; i < N; i++)
		{
			if(data[i] != other[i])
				return false;
		}

		return true;
	}

	vec operator -() const
	{
		vec res;
		for(u32 i = 0; i < N; i++)
			res[i] = -data[i];

		return res;
	}

	vec operator +(const vec& other) const
	{
		vec res;
		for(u32 i = 0; i < N; i++)
			res[i] = data[i] + other[i];

		return res;
	}

	vec operator -(const vec& other) const
	{
		return *this + -other;
	}

	vec operator *(T other) const
	{
		vec res;
		for(u32 i = 0; i < N; i++)
			res[i] = data[i] * other;

		return res;
	}

	T operator *(const vec& other) const
	{
		T res;
		for(u32 i = 0; i < N; i++)
			res += data[i] * other[i];

		return res;
	}

	string asString() const
	{
		fmt::MemoryWriter str;
		str << "[" << data[0];

		for(usz i = 1; i < N; i++)
			str << ", " << data[i];

		str << "]";
		return str.str();
	}

	friend std::ostream &operator<<(std::ostream &os, const vec &vec)
	{
		return os << vec.asString();
	}

	static const vec O;

	union
	{
		T data[N];
		struct {
			T x, y, z, w;
		};
	};
};

template<usz N, class T>
const Vector<N,T> Vector<N,T>::O;

namespace std {
	template<usz N, class T>
	struct hash<Vector<N,T>>
	{
		typedef Vector<N,T> argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& o) const
		{
			result_type h = o[0];
			for(usz i = 1; i < N; i++)
				h ^= (o[i] << i);

			return h;
		}
	};
}

using Vec2 = Vector<2, i32>;

template<class CharT, class Traits, usz N, class T>
std::basic_istream<CharT,Traits>& operator>>(std::basic_istream<CharT,Traits>& stream, Vector<N,T>& o)
{
	for(u32 i = 0; i < N; i++)
		stream >> o[i];
	return stream;
}

template<usz N, class T>
bool inBox(const Vector<N,T>& x, const Vector<N,T>& begin, const Vector<N,T>& end)
{
	for(usz i = 0; i < N; i++)
		if(x[i] < begin[i] || end[i] < x[i])
			return false;

	return true;
}
