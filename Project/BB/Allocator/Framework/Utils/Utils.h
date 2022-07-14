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

		inline static size_t Min(size_t a_A, size_t a_B)
		{
			if (a_A < a_B)
				return a_A;
			return a_B;
		}

		inline static  float Lerp(float a_A, float a_B, float a_T)
		{
			return a_A + a_T * (a_B - a_A);
		}
	}

	namespace Random
	{
		static unsigned int MathRandomSeed = 1;

		/// <summary>
		/// Set the random seed that the Math.h header uses.
		/// </summary>
		inline static void Seed(unsigned int a_Seed)
		{
			MathRandomSeed = a_Seed;
		}

		/// <summary>
		/// Check if the chosenValue is close to the between value by using an 0.00001f epsilon.
		/// </summary>
		inline static bool Aprox(float a_A, float a_B)
		{
			if (a_A - 3.0f < a_B && a_A + 3.0f > a_B)
			{
				return true;
			}

			return false;
		}

		/// <summary>
		/// Get a Random unsigned int between 0 and INT_MAX.
		/// </summary>
		inline static  unsigned int Random()
		{
			MathRandomSeed ^= MathRandomSeed << 13, MathRandomSeed ^= MathRandomSeed >> 17;
			MathRandomSeed ^= MathRandomSeed << 5;
			return MathRandomSeed;
		}

		/// <summary>
		/// Get a Random unsigned int between 0 and maxValue.
		/// </summary>
		inline static  unsigned int Random(unsigned int a_Max)
		{
			return Random() % a_Max;
		}

		/// <summary>
		/// Get a Random unsigned int between min and max value.
		/// </summary>
		inline static unsigned int Random(unsigned int a_Min, unsigned int a_Max)
		{
			return Random() % (a_Max + 1 - a_Min) + a_Min;
		}

		/// <summary>
		/// Get a Random float between 0 and 1.
		/// </summary>
		inline  static float RandomF()
		{
			return fmod(static_cast<float>(Random()) * 2.3283064365387e-10f, 1.0f);
		}

		/// <summary>
		/// Get a Random float between 0 and max value.
		/// </summary>
		inline static  float RandomF(float a_Min, float a_Max)
		{
			return (RandomF() * (a_Max - a_Min)) + a_Min;
		}
	}

	namespace Pointer
	{
		/// <summary>
		/// Move the given pointer by a given size.
		/// </summary>
		/// <param name="a_Ptr:"> The pointer you want to shift </param>
		/// <param name="a_Add:"> The amount of bytes you want move the pointer forward. </param>
		/// <returns>The shifted pointer. </returns>
		inline static void* Add(void* a_Ptr, size_t a_Add)
		{
			return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(a_Ptr) + a_Add);
		}

		/// <summary>
		/// Move the given pointer by a given size.
		/// </summary>
		/// <param name="a_Ptr:"> The pointer you want to shift </param>
		/// <param name="a_Subtract:"> The amount of bytes you want move the pointer backwards. </param>
		/// <returns>The shifted pointer. </returns>
		inline static void* Subtract(void* a_Ptr, size_t a_Subtract)
		{
			return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(a_Ptr) - a_Subtract);
		}

		/// <summary>
		/// Align a given pointer forward.
		/// </summary>
		/// <param name="a_Ptr:"> The pointer you want to align </param>
		/// <param name="a_Alignment:"> The alignment of the data. </param>
		/// <returns>The given address but aligned forward. </returns>
		inline static size_t AlignForwardAdjustment(const void* a_Ptr, size_t a_Alignment)
		{
			size_t adjustment = a_Alignment - (reinterpret_cast<uintptr_t>(a_Ptr) & static_cast<uintptr_t>(a_Alignment - 1));

			if (adjustment == a_Alignment) return 0;

			//already aligned 
			return adjustment;
		}

		/// <summary>
		/// Align a given pointer forward.
		/// </summary>
		/// <param name="a_Ptr:"> The pointer you want to align </param>
		/// <param name="a_Alignment:"> The alignment of the data. </param>
		/// <param name="a_HeaderSize:"> The size in bytes of the Header you want to align forward's too </param>
		/// <returns>The given address but aligned forward with the allocation header's size in mind. </returns>
		inline static size_t AlignForwardAdjustmentHeader(const void* a_Ptr, size_t a_Alignment, size_t a_HeaderSize)
		{
			size_t adjustment = AlignForwardAdjustment(a_Ptr, a_Alignment);
			size_t neededSpace = a_HeaderSize;

			if (adjustment < neededSpace)
			{
				neededSpace -= adjustment;

				//Increase adjustment to fit header 
				adjustment += a_Alignment * (neededSpace / a_Alignment);

				if (neededSpace % a_Alignment > 0) adjustment += a_Alignment;
			}

			return adjustment;
		}
	}
}