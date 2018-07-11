#ifndef HANDMADE_ENTITY_H
#define HANDMADE_ENTITY_H
/*  ========================================================================
    $File: $
    $Date: $
    $Revision: $
    $Creator: Casey Muratori $
    $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
    ======================================================================== */



enum entity_space
{
	Space_None,
	Space_Dormant,
	Space_Low,
	Space_High
};

#define FACING_NORTH 0
#define FACING_EAST 1
#define FACING_SOUTH 2
#define FACING_WEST 3

struct entity_high
{
	vec2d Pos;
	vec2d Vel;
	uint32 Facing;
	uint32 TileZ;
};

struct entity_low
{
	
};

struct entity_info 
{
	real32 Width;
	real32 Height;
	real32 Acceleration;
	real32 Speed;
	real32 SprintModifier;
	bool32 Collides;
	bool32 Solid;

	uint32 BitmapIndex;
	//bitmap Image;

	tile_position Pos;
	int32 dAbsTileZ;
};

struct entity
{
	entity_space *Space;
	entity_info *Info;
	entity_low *Low;
	entity_high *High;
};

struct ai_controller
{
	real32 DirectionX;
	real32 DirectionY;
	uint32 DirectionTimer;
	uint32 MoveTimer;
	uint32 ControllingEntityID;
};

struct camera
{
	uint32 FollowIndex;
	tile_position P;
};

#endif