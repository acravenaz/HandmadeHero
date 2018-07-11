/*  ========================================================================
    $File: $
    $Date: $
    $Revision: $
    $Creator: Casey Muratori $
    $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
    ======================================================================== */

#include "handmade_entity.h"

//These are in handmade_map.cpp, but we need them now.
internal vec2d 
TileDiff(tile_position *A, tile_position *B);

inline tile_position
MapIntoTileSpace(tile_position Pos, vec2d Offset, uint32 TileWidth, uint32 TileHeight);
//

internal void
ChangeEntitySpace(game_state *GameState, uint32 EntityID, entity_space Space)
{
	if (Space == Space_High)
	{
		if (GameState->EntitySpaces[EntityID] != Space_High)
		{
			entity_high *EntityHigh = &GameState->HighEntities[EntityID];
			entity_info *EntityInfo = &GameState->EntityInfos[EntityID];

			vec2d Diff = TileDiff(&EntityInfo->Pos, &GameState->Camera.P);
			Diff = Diff * (real32)GameState->WorldMap->TileWidth;
			EntityHigh->Pos = Diff;
			EntityHigh->Vel = vec2d{0, 0};
			EntityHigh->TileZ = EntityInfo->Pos.TileZ;
			EntityHigh->Facing = 0;
		}
	}
	GameState->EntitySpaces[EntityID] = Space;
}

internal entity 
GetEntity(game_state *GameState, entity_space Space, uint32 EntityID)
{
	entity Entity = {};
	if (EntityID > 0 && EntityID < ArrayCount(GameState->EntityInfos))
	{
		Entity.Space = &GameState->EntitySpaces[EntityID];
		Entity.Info = &GameState->EntityInfos[EntityID];
		Entity.Low = &GameState->LowEntities[EntityID];
		Entity.High = &GameState->HighEntities[EntityID];
		if (*Entity.Space != Space)
		{
			ChangeEntitySpace(GameState, EntityID, Space);
		}
	}
	return (Entity);
}


internal bool32 
TestWall(real32 WallX, real32 RelX, real32 RelY, real32 EntityDeltaX, real32 EntityDeltaY,
					   real32 *tMin, real32 MinY, real32 MaxY)
{
	bool32 Hit = false;
	real32 tEpsilon = 0.01f;
	if (EntityDeltaX != 0.0f)
	{
		real32 tResult = (WallX - RelX) / EntityDeltaX;
		real32 Y = RelY + (tResult*EntityDeltaY);
		if ((tResult >= 0.0f) && (*tMin > tResult))
		{
			if ((Y >= MinY) && (Y <= MaxY))
			{
				*tMin = MAXIMUM(0.0f, (tResult - tEpsilon));
				Hit = true;
			}
		}
	}
	return (Hit);
}

internal void 
MoveEntity(entity Entity, game_state *GameState, real32 dT, vec2d A, bool32 Sprinting)
{
	world_map *WorldMap = GameState->WorldMap;
	real32 EntitySpeed = Entity.Info->Speed;
	real32 EntityAcceleration = Entity.Info->Acceleration;
	if (Sprinting)
	{
		EntitySpeed *= Entity.Info->SprintModifier;
		EntityAcceleration *= Entity.Info->SprintModifier;
	}

	if (A.X <= 0.0f)
	{
		if (Entity.High->Vel.X > 0)
		{
			if (Entity.High->Vel.X > dT * (EntityAcceleration))
			{
				A.X -= 1.0f;
			}
			else {
				Entity.High->Vel.X = 0.0f;
				A.X = 0.0f;
			}
		}
	}
	if (A.X >= 0.0f)
	{
		if (Entity.High->Vel.X < 0)
		{
			if (Entity.High->Vel.X < dT * (-EntityAcceleration))
			{
				A.X += 1.0f;
			}
			else {
				Entity.High->Vel.X = 0.0f;
				A.X = 0.0f;
			}
		}
	}
	if (A.Y <= 0.0f)
	{
		if (Entity.High->Vel.Y > 0)
		{
			if (Entity.High->Vel.Y > dT * (EntityAcceleration))
			{
				A.Y -= 1.0f;
			}
			else {
				Entity.High->Vel.Y = 0.0f;
				A.Y = 0.0f;
			}
		}
	}
	if (A.Y >= 0.0f)
	{
		if (Entity.High->Vel.Y < 0)
		{
			if (Entity.High->Vel.Y < dT * (-EntityAcceleration))
			{
				A.Y += 1.0f;
			}
			else {
				Entity.High->Vel.Y = 0.0f;
				A.Y = 0.0f;
			}
		}
	}

	real32 VLengthSq = LengthSq(Entity.High->Vel);
	if (VLengthSq > Square(EntitySpeed))
	{
		Entity.High->Vel *= (EntitySpeed / SquareRoot(VLengthSq));
	}

	A *= EntityAcceleration;

	vec2d OldP = Entity.High->Pos;
	vec2d EntityDelta = {(0.5f*A*Square(dT) + 
		(dT * Entity.High->Vel))};
	Entity.High->Vel = (dT * A) + Entity.High->Vel;
	
	vec2d NewP = OldP + EntityDelta;

/*
	uint32 StartTileX = MINIMUM(Entity.High->Pos.TileX, NewP.TileX);
	uint32 StartTileY = MINIMUM(Entity.High->Pos.TileY, NewP.TileY);
	uint32 EndTileX = MAXIMUM(Entity.High->Pos.TileX, NewP.TileX);
	uint32 EndTileY = MAXIMUM(Entity.High->Pos.TileY, NewP.TileY);

	uint32 EntityTileWidth = CeilReal32ToInt32(Entity.Info->Width);
	uint32 EntityTileHeight = CeilReal32ToInt32(Entity.Info->Height);

	StartTileX -= EntityTileWidth;
	StartTileY -= EntityTileHeight;
	EndTileX += EntityTileWidth;
	EndTileY += EntityTileHeight;

	uint32 TileZ = Entity.High->Pos.TileZ;
*/
	real32 tRemaining = 1.0f;
	for (uint32 i = 0;
		 (i < 4) && (tRemaining > 0.0f);
		 i++)
	{
		real32 tMin = 1.0f;
		vec2d WallNormal = {0, 0};
		uint32 HitEntityID = 0;

        for(uint32 EntityID = 1;
            EntityID < GameState->EntityCount;
            ++EntityID)
        {
            entity TestEntity = GetEntity(GameState, Space_High, EntityID);
            if(TestEntity.High != Entity.High)
            {
                if(TestEntity.Info->Collides)
                {
					real32 DiamW = (TestEntity.Info->Width + Entity.Info->Width) * GameState->WorldMap->TileWidth;
					real32 DiamH = (TestEntity.Info->Height + Entity.Info->Height) * GameState->WorldMap->TileHeight;

					vec2d MinCorner = -0.5f*vec2d{DiamW, DiamH};
					vec2d MaxCorner = 0.5f*vec2d{DiamW, DiamH};

					vec2d RelEntityP = Entity.High->Pos - TestEntity.High->Pos;
					
					if (TestWall(MinCorner.X, RelEntityP.X, RelEntityP.Y, EntityDelta.X, EntityDelta.Y,
							&tMin, MinCorner.Y, MaxCorner.Y))
					{
						WallNormal = vec2d{-1, 0};
						HitEntityID = EntityID;
					}
					if (TestWall(MaxCorner.X, RelEntityP.X, RelEntityP.Y, EntityDelta.X, EntityDelta.Y,
							&tMin, MinCorner.Y, MaxCorner.Y))
					{
						WallNormal = vec2d{1, 0};
						HitEntityID = EntityID;
					}
					if (TestWall(MinCorner.Y, RelEntityP.Y, RelEntityP.X, EntityDelta.Y, EntityDelta.X,
							&tMin, MinCorner.X, MaxCorner.X))
					{
						WallNormal = vec2d{0, -1};
						HitEntityID = EntityID;
					}
					if (TestWall(MaxCorner.Y, RelEntityP.Y, RelEntityP.X, EntityDelta.Y, EntityDelta.X,
							&tMin, MinCorner.X, MaxCorner.X))
					{
						WallNormal = vec2d{0, 1};
						HitEntityID = EntityID;
					}
				}
			}
		}

		Entity.High->Pos += tMin*EntityDelta;
        if(HitEntityID)
        {
            Entity.High->Vel = Entity.High->Vel - 1*Inner(Entity.High->Vel, WallNormal)*WallNormal;
            EntityDelta = EntityDelta - 1*Inner(EntityDelta, WallNormal)*WallNormal;
            tRemaining -= tMin*tRemaining;            

            entity HitEntity = GetEntity(GameState, Space_Dormant, HitEntityID);
            Entity.High->TileZ += HitEntity.Info->dAbsTileZ;
        }
        else
        {
            break;
        }
	}

	if (AbsValue(Entity.High->Vel.X) > AbsValue(Entity.High->Vel.Y)) {
		if (Entity.High->Vel.X > 0)
		{
			Entity.High->Facing = FACING_EAST;
		}
		else if (Entity.High->Vel.X < 0)
		{
			Entity.High->Facing = FACING_WEST;
		}
	}
	else if (AbsValue(Entity.High->Vel.Y) > AbsValue(Entity.High->Vel.X))
	{
		if (Entity.High->Vel.Y > 0)
		{
			Entity.High->Facing = FACING_NORTH;
		}
		else if (Entity.High->Vel.Y < 0)
		{
			Entity.High->Facing = FACING_SOUTH;
		}
	}
	//TODO(Andrew): Always write back a valid tile position to Entity.Low
	Entity.Info->Pos = MapIntoTileSpace(GameState->Camera.P, Entity.High->Pos, GameState->WorldMap->TileWidth, GameState->WorldMap->TileHeight);
}

internal void 
InitializeCamera(world_map *Map, camera *Camera)
{
	Camera->P.TileX = (CHUNK_WIDTH * Map->MapWidth / 2);
	Camera->P.TileY = (CHUNK_HEIGHT * Map->MapHeight / 2);
	Camera->P.TileZ = 0;
	Camera->P.Offset = {-0.5f, -0.5f};
	Camera->FollowIndex = 1;
}

internal void 
InitializePlayer(game_state *GameState, uint32 EntityIndex)
{
	entity_space Space = Space_Dormant;
	entity Entity = GetEntity(GameState, Space, EntityIndex);

	Entity.Info->Width = 0.75f;
	Entity.Info->Height = 0.5f;
	Entity.Info->Pos.TileX = (CHUNK_WIDTH * GameState->WorldMap->MapWidth / 2);
	Entity.Info->Pos.TileY = (CHUNK_HEIGHT * GameState->WorldMap->MapHeight / 2);
	Entity.Info->Pos.TileZ = 0;
	Entity.Info->Pos.Offset = {-0.5f, -0.5f};
	Entity.Info->BitmapIndex = rand() % 2;
	Entity.Info->Acceleration = Square(40.0f);
	Entity.Info->Speed = 140.0f;
	Entity.Info->SprintModifier = 2.0f;
	Entity.Info->Collides = false;
	Entity.Info->Solid = false;

	ChangeEntitySpace(GameState, EntityIndex, Space_High);

}

internal void
InitializeAIController(ai_controller *Controller, uint32 ControlID)
{
	Controller->DirectionX = 0.0f;
	Controller->DirectionY = 0.0f;
	Controller->DirectionTimer = 0;
	Controller->MoveTimer = 0;
	Controller->ControllingEntityID = ControlID;
}

internal ai_controller 
*GetAIController(game_state *GameState, uint32 ControllerID)
{
	ai_controller *Result = 0;
	if (ControllerID < ArrayCount(GameState->AIControllers))
	{
		Result = &GameState->AIControllers[ControllerID];
	}
	return (Result);
}

internal uint32 
InitializeEntity(game_state *GameState)
{
	ASSERT(GameState->EntityCount < ArrayCount(GameState->EntityInfos));
	uint32 NewEntityID = GameState->EntityCount++;
	entity_info *Info = &GameState->EntityInfos[NewEntityID];
	entity_low *Low = &GameState->LowEntities[NewEntityID];
	entity_high *High = &GameState->HighEntities[NewEntityID];
	entity_space *Space = &GameState->EntitySpaces[NewEntityID];
	*Info = {};
	*Low = {};
	*High = {};
	*Space = Space_Dormant;
	return (NewEntityID);
}

internal void
InitializeWall(game_state *GameState, uint32 EntityIndex, uint32 TileX, uint32 TileY, uint32 TileZ)
{
	entity_space Space = Space_Dormant;
	entity Entity = GetEntity(GameState, Space, EntityIndex);

	Entity.Info->Width = 1.0f;
	Entity.Info->Height = 1.0f;
	Entity.Info->Pos.TileX = TileX;
	Entity.Info->Pos.TileY = TileY;
	Entity.Info->Pos.TileZ = TileZ;
	Entity.Info->Pos.Offset = {0.0f, 0.0f};
	Entity.Info->BitmapIndex = 2;
	Entity.Info->Acceleration = 0;
	Entity.Info->Speed = 0.0f;
	Entity.Info->SprintModifier = 0.0f;
	Entity.Info->Collides = true;
	Entity.Info->Solid = true;

	ChangeEntitySpace(GameState, EntityIndex, Space_High);
}

internal void
InitializeStair(game_state *GameState, uint32 EntityIndex, uint32 TileX, uint32 TileY, uint32 TileZ, bool32 StairsUp)
{
	entity_space Space = Space_Dormant;
	entity Entity = GetEntity(GameState, Space, EntityIndex);

	Entity.Info->Width = 1.0f;
	Entity.Info->Height = 1.0f;
	Entity.Info->Pos.TileX = TileX;
	Entity.Info->Pos.TileY = TileY;
	Entity.Info->Pos.TileZ = TileZ;
	Entity.Info->Pos.Offset = {0.0f, 0.0f};
    if (StairsUp)
    {
	    Entity.Info->BitmapIndex = 3;
        Entity.Info->dAbsTileZ = 1;
    }
    else 
    {
        Entity.Info->BitmapIndex = 4;
        Entity.Info->dAbsTileZ = -1;
    }
	Entity.Info->Acceleration = 0;
	Entity.Info->Speed = 0.0f;
	Entity.Info->SprintModifier = 0.0f;
	Entity.Info->Collides = false;
	Entity.Info->Solid = false;

	ChangeEntitySpace(GameState, EntityIndex, Space_High);
}

internal void 
DEBUGInitializeDummies(game_state *GameState)
{
	for (uint32 i = 1;
		i < DEBUG_SPAM_ENTITIES_COUNT;
		i++)
	{
		ai_controller *Controller = GetAIController(GameState, i - 1);		
		uint32 EntityID = InitializeEntity(GameState);
		//entity Entity = GetEntity(GameState, Space_High, EntityID);
		InitializeAIController(Controller, EntityID);
		InitializePlayer(GameState, EntityID);
	}
}