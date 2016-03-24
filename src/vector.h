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

private:
	T data[N];
};

using vec2 = Vector<2, i32>;

