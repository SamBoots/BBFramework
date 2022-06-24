#pragma once
#include <cmath>

namespace BB
{
	namespace Math
	{
		inline static size_t RoundUp(size_t a_NumToRound, size_t a_Multiple)
		{
			return ((a_NumToRound + a_Multiple - 1) / a_Multiple) * a_Multiple;
		}

		inline static size_t Max(size_t a_A, size_t a_B)
		{
			if (a_A > a_B)
				return a_A;
			return a_B;
		}

		inline float Lerp(float a, float b, float t)
		{
			return a + t * (b - a);
		}
	}

	namespace Utils
	{
		static unsigned int MATH_RANDOMSEED = 1;

		/// <summary>
		/// Set the random seed that the Math.h header uses.
		/// </summary>
		inline static void SetMathRandomSeed(unsigned int a_Seed)
		{
			MATH_RANDOMSEED = a_Seed;
		}

		/// <summary>
		/// Check if the chosenValue is close to the between value by using an 0.00001f epsilon.
		/// </summary>
		inline bool Aprox(float chosenValue, float betweenValue)
		{
			if (chosenValue - 3.0f < betweenValue && chosenValue + 3.0f > betweenValue)
			{
				return true;
			}

			return false;
		}

#pragma region Random unsigned ints

		/// <summary>
		/// Get a Random unsigned int between 0 and INT_MAX.
		/// </summary>
		inline unsigned int RandomUInt()
		{
			MATH_RANDOMSEED ^= MATH_RANDOMSEED << 13, MATH_RANDOMSEED ^= MATH_RANDOMSEED >> 17;
			MATH_RANDOMSEED ^= MATH_RANDOMSEED << 5;
			return MATH_RANDOMSEED;
		}

		/// <summary>
		/// Get a Random unsigned int between 0 and maxValue.
		/// </summary>
		inline unsigned int RandomUIntMax(unsigned int maxValue)
		{
			return RandomUInt() % maxValue;
		}

		/// <summary>
		/// Get a Random unsigned int between min and max value.
		/// </summary>
		inline static unsigned int RandomUintMinMax(unsigned int min, unsigned int max)
		{
			return RandomUInt() % (max + 1 - min) + min;
		}

#pragma endregion

		/// <summary>
		/// Get a Random float between 0 and 1.
		/// </summary>
		inline float RandFloat()
		{
			return fmod(static_cast<float>(RandomUInt()) * 2.3283064365387e-10f, 1.0f);
		}

		/// <summary>
		/// Get a Random float between 0 and max value.
		/// </summary>
		inline float RandFloatMinMax(float min, float max)
		{
			return (RandFloat() * (max - min)) + min;
		}
	}
}