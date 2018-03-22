#pragma once
#include <functional>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

/*!
 * calculates a mod b, by the mathematical definition:
 * the C operator % does not give the right answer if a < 0
 */
inline int modulus(int a, int b)
{
	if (a < 0) return b + (a %  b);
	else return (a % b);
}

/*!
 * calculates the a mod b, where a and b are double floats.
 */
inline double modulus(double a, double b)
{
	if (a < 0) return a - static_cast<int>(a / b - 1) * b;
	else return a - static_cast<int>(a / b) * b;
}

inline size_t ipow(size_t N, unsigned R)
{
	size_t S = 1;
	for (unsigned i = 0; i < R; ++i)
		S *= N;
	return S;
}

