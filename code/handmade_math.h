#ifndef HANDMADE_MATH_H
#define HANDMADE_MATH_H
/*  ========================================================================
	$File: $
	$Date: $
	$Revision: $
	$Creator: Casey Muratori $
	$Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
	======================================================================== */

union vec2d
{
	real32 E[2];
	struct 
	{
		real32 X, Y;
	};
};

vec2d operator*(real32 A, vec2d B)
{
	vec2d Result = {};
	Result.X = A*B.X;
	Result.Y = A*B.Y;
	return(Result);
}

vec2d operator*(vec2d A, real32 B)
{
	vec2d Result = B * A;
	return(Result);
}

inline vec2d &operator*=(vec2d &A, real32 B)
{
	A = B * A;
	return(A);
}

inline vec2d operator-(vec2d A)
{
	vec2d Result = {};
	Result.X = -A.X;
	Result.Y = -A.Y;
	return(Result);
}

inline vec2d operator+(vec2d A, vec2d B)
{
	vec2d Result = {};
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return(Result);
}

inline vec2d &operator+=(vec2d &A, vec2d B)
{
	A = A + B;
	return(A);
}

inline vec2d operator-(vec2d A, vec2d B)
{
	vec2d Result = {};
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return(Result);
}

inline real32 Inner(vec2d A, vec2d B)
{
	real32 Result = (A.X * B.X) + (A.Y * B.Y);
	return (Result);
}

inline real32 LengthSq(vec2d A)
{
	real32 Result = Inner(A, A);
	return(Result);
}

#endif