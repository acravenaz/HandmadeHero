#ifndef HANDMADE_INTRINSICS_H
#define HANDMADE_INTRINSICS_H
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

//
// TODO(casey): Convert all of these to platform-efficient versions
// and remove math.h
//

#include <cmath>

//IMPORTANT(Andrew): NOT a proper signum function!
inline int32 SignOf(int32 Value)
{
    // This will return positive 1 if the value is 0.
    int32 Result = (Value >= 0) ? 1 : -1;
    return (Result);
}

inline real32 
AbsValue(real32 Value)
{
    real32 Result = fabsf(Value);
    return (Result);
}

inline real32 
SquareRoot(real32 Real32)
{
    real32 Result = sqrtf(Real32);
    return (Result);
}

inline real32 
Square(real32 Real32)
{
	real32 Result = Real32 * Real32;
	return (Result);
}

inline real32 
RoundReal32(real32 Value)
{
    real32 Result = roundf(Value);
    return (Result);
}

inline int32
RoundReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)roundf(Real32);
    return(Result);
}

inline uint32
RoundReal32ToUInt32(real32 Real32)
{
    uint32 Result = (uint32)roundf(Real32);
    return(Result);
}

inline int32 
FloorReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)floorf(Real32);
    return(Result);
}

inline int32 
CeilReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)ceilf(Real32);
    return(Result);
}

inline int32
TruncateReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)Real32;
    return(Result);
}

inline real32
Sin(real32 Angle)
{
    real32 Result = sinf(Angle);
    return(Result);
}

inline real32
Cos(real32 Angle)
{
    real32 Result = cosf(Angle);
    return(Result);
}

inline real32
ATan2(real32 Y, real32 X)
{
    real32 Result = atan2f(Y, X);
    return(Result);
}

struct bit_scan_result
{
	bool32 Found;
	uint32 Index;
};

internal bit_scan_result BitScanForward(uint32 Value)
{
	bit_scan_result Result = {};

#if COMPILER_MSVC
	Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
	Result.Found = false;
	for (uint32 Test = 0;
		Test < 32;
		Test++)
	{
		if (Value & 1 << Test)
		{
			Result.Index = Test;
			Result.Found = true;
			break;
		}
	}
#endif
	return (Result);
}
#endif