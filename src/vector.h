#pragma once

#include "global.h"

#include <vector>

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

private:
	T data[N];
};

using vec2 = Vector<2, i32>;

