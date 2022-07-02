#pragma once

#include <cassert>
#include <iostream>

namespace BB
{
	/*  Check for unintented behaviour at compile time, if a_Check is false the program will stop and post a message.
@param a_Check, If false the program will print the message and assert.
@param a_Msg, The message that will be printed. */
#define BB_STATIC_ASSERT(a_Check, a_Msg)\
			if (!(a_Check)) \
			{\
				printf("FRAMEWORK ERROR: ");\
				printf((a_Msg));\
				printf("\n");\
				static_assert(a_Check, a_Msg);\
			}

	/*  Check for unintented behaviour at runetime, if a_Check is false the program will stop and post a message.
	@param a_Check, If false the program will print the message and assert.
	@param a_Msg, The message that will be printed. */
#define BB_ASSERT(a_Check, a_Msg)\
			if (!(a_Check)) \
			{\
				printf("FRAMEWORK ERROR: ");\
				printf((a_Msg));\
				printf("\n");\
				assert(false);\
			}

	/*  Check for unintented behaviour at runtime, if a_Check is false the program will post a warning message.
@param a_Check, If false the program will print the message and assert.
@param a_Msg, The message that will be printed. */
#define BB_EXCEPTION(a_Check, a_Msg)\
			if (!(a_Check)) \
			{\
				printf("FRAMEWORK EXCEPTION: ");\
				printf((a_Msg));\
				printf("\n");\
				throw a_Msg;\
			}

	/*  Check for unintented behaviour at runtime, if a_Check is false the program will post a warning message.
	@param a_Check, If false the program will print the message and assert.
	@param a_Msg, The message that will be printed. */
#define BB_WARNING(a_Check, a_Msg)\
			if (!(a_Check)) \
			{\
				printf("FRAMEWORK WARNING: ");\
				printf((a_Msg));\
				printf("\n");\
			}
}

