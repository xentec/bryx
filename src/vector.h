#pragma once

#include "global.h"

#include <sstream>
#include <initializer_list>

template<usz N, class T>
struct Vector
{
	Vector(T init = 0)
	{
		for(T& e : data)
			e = init;
	}

	Vector(const Vector<N,T>& other)
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

	bool operator ==(const Vector<N,T>& other) const
	{
		for(u32 i = 0; i < N; i++)
			if(data[i] != other[i])
				return false;

		return true;
	}

	Vector<N,T> operator -() const
	{
		Vector<N,T> res;
		for(u32 i = 0; i < N; i++)
			res[i] = -data[i];
	}

	Vector<N,T> operator +(const Vector<N,T>& other) const
	{
		Vector<N,T> res;
		for(u32 i = 0; i < N; i++)
			res[i] = data[i] + other[i];

		return res;
	}

	Vector<N,T> operator -(const Vector<N,T>& other) const
	{
		return *this + -other;
	}

	Vector<N,T> operator *(T other) const
	{
		Vector<N,T> res;
		for(u32 i = 0; i < N; i++)
			res[i] = data[i] * other;

		return res;
	}

	T operator *(const Vector<N,T>& other) const
	{
		T res;
		for(u32 i = 0; i < N; i++)
			res += data[i] * other[i];

		return res;
	}

	string asString() const
	{
		std::stringstream str;
		str << "[" << data[0];

		for(usz i = 1; i < N; i++)
			str << ", " << data[i];

		str << "]";
		return str.str();
	}

	static const Vector<N,T> O;
private:
	T data[N];
};

template<usz N, class T>
const Vector<N,T> Vector<N,T>::O;

namespace std {
	template<usz N, class T>
	struct hash<Vector<N,T>>
	{
		typedef Vector<N,T> argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& vec) const
		{
			result_type h = vec[0];
			for(usz i = 1; i < N; i++)
				h ^= (vec[i] << i);

			return h;
		}
	};
}

using Vec2 = Vector<2, i32>;

template<usz N, class T>
bool inQuadRange(const Vector<N,T>& x, const Vector<N,T>& begin, const Vector<N,T>& end)
{
	for(usz i = 0; i < N; i++)
		if(x[i] < begin[i] || end[i] < x[i])
			return false;

	return true;
}
