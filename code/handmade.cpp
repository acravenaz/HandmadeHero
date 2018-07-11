/*  ========================================================================
	$File: $
	$Date: $
	$Revision: $
	$Creator: Casey Muratori $
	$Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
	======================================================================== */

#include "handmade.h"

internal void
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
	int16 ToneVolume = 3000;
	int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;

	int16 *SampleOut = SoundBuffer->Samples;
	for(int SampleIndex = 0;
		SampleIndex < SoundBuffer->SampleCount;
		++SampleIndex)
	{
		// TODO(casey): Draw this out for people
#if 0
		real32 SineValue = sinf(GameState->tSine);
		int16 SampleValue = (int16)(SineValue * ToneVolume);
#else
		int16 SampleValue = 0;
#endif
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;

#if 0
		GameState->tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod;
		if(GameState->tSine > 2.0f*Pi32)
		{
			GameState->tSine -= 2.0f*Pi32;
		}
#endif
	}
}

internal void
DrawRectangle(game_offscreen_buffer *Buffer,
			  real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY,
			  real32 R, real32 G, real32 B)
{
	int32 MinX = RoundReal32ToInt32(RealMinX);
	int32 MinY = RoundReal32ToInt32(RealMinY);
	int32 MaxX = RoundReal32ToInt32(RealMaxX);
	int32 MaxY = RoundReal32ToInt32(RealMaxY);

	if(MinX < 0)
	{
		MinX = 0;
	}
	if(MinY < 0)
	{
		MinY = 0;
	}
	if(MaxX > Buffer->Width)
	{
		MaxX = Buffer->Width;
	}

	if(MaxY > Buffer->Height)
	{
		MaxY = Buffer->Height;
	}
	
	uint32 Color = RoundReal32ToUInt32(R * 255.0f) << 16 |
		RoundReal32ToUInt32(G * 255.0f) << 8 |
		RoundReal32ToUInt32(B * 255.0f) << 0;

	uint8 *Row = ((uint8 *)Buffer->Memory +
				  MinX*Buffer->BytesPerPixel +
				  MinY*Buffer->Pitch);
	for(int Y = MinY;
		Y < MaxY;
		++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for(int X = MinX;
			X < MaxX;
			++X)
		{            
			*Pixel++ = Color;
		}
		
		Row += Buffer->Pitch;
	}
}


internal bitmap 
DEBUGLoadBitmap(thread_context *Thread, 
	debug_platform_read_entire_file *ReadEntireFile, char *FileName)
{
	bitmap Result = {};
	debug_read_file_result ReadResult = ReadEntireFile(Thread, FileName);
	if (ReadResult.ContentsSize != 0)
	{
		bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
		uint32 *Pixels = (uint32 *)((uint8 *)ReadResult.Contents + Header->BitmapOffset);

		uint32 AlphaMask = ~(Header->RedMask | Header->GreenMask | Header->BlueMask);
		uint32 AlphaShift = BitScanForward(AlphaMask).Index;
		uint32 RedShift = BitScanForward(Header->RedMask).Index;
		uint32 GreenShift = BitScanForward(Header->GreenMask).Index;
		uint32 BlueShift = BitScanForward(Header->BlueMask).Index;

		uint32 *SourceDest = Pixels;
		for (int32 Y = 0; Y < Header->Height; ++Y)
		{
			for (int32 X = 0; X < Header->Width; ++X)
			{
				uint8 R = (uint8)(*SourceDest >> RedShift);
				uint8 G = (uint8)(*SourceDest >> GreenShift);
				uint8 B = (uint8)(*SourceDest >> BlueShift);
				uint8 A = (uint8)(*SourceDest >> AlphaShift);
				*SourceDest = ((A << 24) | (R << 16) | (G << 8) | (B << 0));
				++SourceDest;
			}
		}
		Result.Pixels = Pixels;
		Result.Header = Header;
	}
	return (Result);
}

internal void 
DrawBitmap(game_offscreen_buffer *Buffer, bitmap *Bitmap, vec2d Origin)
{
	int32 MinX = RoundReal32ToInt32(Origin.X);
	int32 MinY = RoundReal32ToInt32(Origin.Y);
	int32 Width = Bitmap->Header->Width;
	int32 Height = Bitmap->Header->Height;
	int32 MaxX = MinX + Width;
	int32 MaxY = MinY + Height;
	int32 OffsetX = 0;
	int32 OffsetY = 0;

	if (MinX < 0)
	{
		OffsetX = -MinX;
		MinX = 0;
	}
	if (MinY < 0)
	{
		OffsetY = -MinY;
		MinY = 0;
	}
	if (MaxX > Buffer->Width)
	{
		MaxX = Buffer->Width;
	}
	if (MaxY > Buffer->Height)
	{
		MaxY = Buffer->Height;
	}
	
	uint32 *SourceRow = Bitmap->Pixels + Width*(Height - (1 + OffsetY));
	uint8 *DestRow = ((uint8 *)Buffer->Memory +
					   MinX*Buffer->BytesPerPixel +
					   MinY*Buffer->Pitch);
	for (int32 Y = MinY;
		Y < MaxY;
		++Y)
	{
		uint32 *Dest = (uint32 *)DestRow;
		uint32 *Source = SourceRow + OffsetX;
		for (int32 X = MinX;
			X < MaxX;
			++X)
		{
			if (Bitmap->DrawMode == Alpha_Blend) {
				real32 A = (real32)((*Source >> 24) & 0xFF) / 255.0f;
				real32 SourceR = (real32)(*Source >> 16 & 0xFF);
				real32 SourceG = (real32)(*Source >> 8 & 0xFF);
				real32 SourceB = (real32)(*Source >> 0 & 0xFF);

				real32 DestR = (real32)(*Dest >> 16 & 0xFF);
				real32 DestG = (real32)(*Dest >> 8 & 0xFF);
				real32 DestB = (real32)(*Dest >> 0 & 0xFF);

				real32 R = (1.0f - A)*DestR + A*SourceR;
				real32 G = (1.0f - A)*DestG + A*SourceG;
				real32 B = (1.0f - A)*DestB + A*SourceB;

				*Dest = ((uint32)(R + 0.5f) << 16 |
					(uint32)(G + 0.5f) << 8 |
					(uint32)(B + 0.5F) << 0);
			}
			else if (Bitmap->DrawMode == Alpha_Test) {
				if (*Source >> 24 > 128)
				{
					*Dest = *Source;
				}
			}
			else if (Bitmap->DrawMode == Alpha_None)
			{
				*Dest = *Source;
			}
			++Dest;
			++Source;
		}
		DestRow += Buffer->Pitch;
		SourceRow -= Width;
	}
}

internal void 
DrawBitmap(game_offscreen_buffer *Buffer, 
	bitmap *Bitmap, real32 X, real32 Y)
{
	vec2d Origin = {X, Y};
	DrawBitmap(Buffer, Bitmap, Origin);
}
/*
internal void 
Move(tile_position *P, vec2d Delta)
{
	P->Offset = P->Offset + Delta;
	*P = MapIntoTileSpace(*P);
}
*/

internal void
MoveAllEntities(game_state *GameState, real32 XShift, real32 YShift)
{
	for (uint32 i = 0; i < GameState->EntityCount; i++)
	{
		if (GameState->EntitySpaces[i] == Space_High)
		{
			entity Entity = GetEntity(GameState, Space_High, i);
			if (Entity.High)
			{
				Entity.High->Pos.X += XShift;
				Entity.High->Pos.Y += YShift;
			}
		}
	}
}

internal void
SetCamera(game_state *GameState, tile_position NewCameraP)
{
	vec2d dCameraP = TileDiff(&NewCameraP, &GameState->Camera.P);
	GameState->Camera.P = NewCameraP;
	vec2d EntityOffset = -(dCameraP * (real32)GameState->WorldMap->TileHeight);
	MoveAllEntities(GameState, EntityOffset.X, EntityOffset.Y);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	ASSERT((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
		(ArrayCount(Input->Controllers[0].Buttons)));
	ASSERT(sizeof(game_state) <= Memory->PermanentStorageSize);

	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if (!Memory->IsInitialized)
	{
		srand((uint32)time(NULL));
		//NOTE(Andrew): InitializeEntity() needs to start at 1, because 0 is reserved
		GameState->EntityCount = 1;

		GameState->EmptyTile = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/empty.bmp");
		GameState->EmptyTile.DrawMode = Alpha_Test;
		GameState->TestBitmap = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_background.bmp");
		GameState->Grass[0] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/grass.bmp");
		GameState->Grass[1] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/grass2.bmp");
		GameState->Grass[2] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/grass3.bmp");
		GameState->Dirt[0] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/dirt1.bmp");
		GameState->Dirt[1] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/dirt2.bmp");
		GameState->Dirt[2] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/dirt3.bmp");
		GameState->Gravel[0] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/gravel1.bmp");
		GameState->Gravel[1] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/gravel2.bmp");
		GameState->Gravel[2] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/gravel3.bmp");
		GameState->EntityBitmaps[0][0] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/hero_north.bmp");
		GameState->EntityBitmaps[0][0].DrawMode = Alpha_Test;
		GameState->EntityBitmaps[0][1] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/hero_east.bmp");
		GameState->EntityBitmaps[0][1].DrawMode = Alpha_Test;
		GameState->EntityBitmaps[0][2] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/hero_south.bmp");
		GameState->EntityBitmaps[0][2].DrawMode = Alpha_Test;
		GameState->EntityBitmaps[0][3] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/hero_west.bmp");
		GameState->EntityBitmaps[0][3].DrawMode = Alpha_Test;
		GameState->EntityBitmaps[1][0] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/hero2_north.bmp");
		GameState->EntityBitmaps[1][0].DrawMode = Alpha_Test;
		GameState->EntityBitmaps[1][1] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/hero2_east.bmp");
		GameState->EntityBitmaps[1][1].DrawMode = Alpha_Test;
		GameState->EntityBitmaps[1][2] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/hero2_south.bmp");
		GameState->EntityBitmaps[1][2].DrawMode = Alpha_Test;
		GameState->EntityBitmaps[1][3] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/hero2_west.bmp");
		GameState->EntityBitmaps[1][3].DrawMode = Alpha_Test;
		GameState->EntityBitmaps[2][0] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/wall.bmp");
		GameState->EntityBitmaps[3][0] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/stairup.bmp");
		GameState->EntityBitmaps[4][0] = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "test/stairdown.bmp");

		memory_region WorldMapMemory;
		InitializeMemoryRegion(&WorldMapMemory, Memory->PermanentStorageSize - sizeof(game_state),
			(uint8 *)Memory->PermanentStorage + sizeof(game_state));
		GameState->WorldMap = ReserveMemory(&WorldMapMemory, world_map);

		GameState->WorldMap->MapWidth = 3;
		GameState->WorldMap->MapHeight = 3;
		GameState->WorldMap->MapDepth = 3;
		GameState->WorldMap->TileWidth = 32;
		GameState->WorldMap->TileHeight = 32;

		GameState->WorldMap->Chunks = 
			ReserveArray(&WorldMapMemory, 
			GameState->WorldMap->MapWidth * GameState->WorldMap->MapHeight * GameState->WorldMap->MapDepth, 
			chunk);

		uint32 Row = 0;
		uint32 Column = 0;
		
		InitializeCamera(GameState->WorldMap, &GameState->Camera);
		DEBUGInitializeDummies(GameState);

		GameState->WorldMap->MapMemory = WorldMapMemory;

		Memory->IsInitialized = true;
	}
	world_map *WorldMap = GameState->WorldMap;

	for (int ControllerIndex = 0;
		ControllerIndex < ArrayCount(Input->Controllers);
		++ControllerIndex)
	{
		game_controller_input *Controller = GetController(Input, ControllerIndex);
		uint32 PlayerEntityIndex = GameState->ControllerPlayerIndex[ControllerIndex];
		entity Player = GetEntity(GameState, Space_High, PlayerEntityIndex);
		if (Controller->Start.EndedDown && Controller->Start.HalfTransitionCount > 0)
		{
			if (!Player.High)
			{
				GameState->ControllerPlayerIndex[ControllerIndex] = InitializeEntity(GameState);
				//entity NewPlayer = GetEntity(GameState, Space_High, GameState->ControllerPlayerIndex[ControllerIndex]);
				InitializePlayer(GameState, GameState->ControllerPlayerIndex[ControllerIndex]);
			}
			GameState->Camera.FollowIndex = GameState->ControllerPlayerIndex[ControllerIndex];
		}
		if (Player.High)
		{
			if (Controller->Back.EndedDown && Controller->Back.HalfTransitionCount > 0)
			{
				if (Player.Info->BitmapIndex == 0)
				{
					Player.Info->BitmapIndex = 1;
				}
				else
				{
					Player.Info->BitmapIndex = 0;
				}
			}
			vec2d A = {};

			if (Controller->IsAnalog)
			{
				A = {Controller->StickAverageX, Controller->StickAverageY};
			}
			else
			{
				if (Controller->MoveLeft.EndedDown) {
					A.X -= 1.0f;
				}
				if (Controller->MoveRight.EndedDown) {
					A.X += 1.0f;
				}
				if (Controller->MoveUp.EndedDown) {
					A.Y += 1.0f;
				}
				if (Controller->MoveDown.EndedDown) {
					A.Y -= 1.0f;
				}
			}
			bool32 Sprinting = false;
			if (Controller->ActionRight.EndedDown)
			{
				Sprinting = true;
			}
#if 0
			if (Controller->ActionUp.EndedDown) {
				tile *Tile = GetTile(WorldMap, Player.High->Pos.TileX, 
											   Player.High->Pos.TileY, 
											   Player.High->Pos.TileZ);
			}
#endif
			MoveEntity(Player, GameState, Input->dtForFrame, A, Sprinting);
		}
	}
#if 1
	for (uint32 i = 0; i < ArrayCount(GameState->AIControllers); i++)
	{
		ai_controller *Controller = GetAIController(GameState, i);
		entity Entity = GetEntity(GameState, Space_High, Controller->ControllingEntityID);

		vec2d A = {};
		A.X = Controller->DirectionX;
		A.Y = Controller->DirectionY;

		if (Controller->DirectionTimer > Controller->MoveTimer)
		{
			Controller->DirectionX = (real32)((rand() % 3) - 1);
			Controller->DirectionY = (real32)((rand() % 3) - 1);
			Controller->DirectionTimer = 0;
			Controller->MoveTimer = (rand() % 400); 
		}

		MoveEntity(Entity, GameState, Input->dtForFrame, A, false);
		++Controller->DirectionTimer;
	}
#endif

#if 1
	int32 TilesPerScreenX = 8;
	int32 TilesPerScreenY = 8;
	tile_position NewCameraP = GameState->Camera.P;
	if(GameState->EntitySpaces[GameState->Camera.FollowIndex] == Space_High)
	{
		entity FollowEntity = GetEntity(GameState, Space_High, GameState->Camera.FollowIndex);
		#if 0
		if (FollowEntity.High->Pos.X > TilesPerScreenX * GameState->WorldMap->TileWidth)
		{
			NewCameraP.TileX += TilesPerScreenX * 2;
			//real32 XShift = -(((real32)TilesPerScreenX * GameState->WorldMap->TileWidth) * 2);
			//MoveAllEntities(GameState, XShift, 0);
		}
		if (FollowEntity.High->Pos.X < -(TilesPerScreenX * (int32)GameState->WorldMap->TileWidth))
		{
			NewCameraP.TileX -= TilesPerScreenX * 2;
			//real32 XShift = ((real32)TilesPerScreenX * GameState->WorldMap->TileWidth) * 2;
			//MoveAllEntities(GameState, XShift, 0);
		}
		if (FollowEntity.High->Pos.Y > TilesPerScreenY * GameState->WorldMap->TileHeight)
		{
			NewCameraP.TileY += TilesPerScreenY * 2;
			//real32 YShift = -(((real32)TilesPerScreenY * GameState->WorldMap->TileHeight) * 2);
			//MoveAllEntities(GameState, 0, YShift);
		}
		if (FollowEntity.High->Pos.Y < -(TilesPerScreenY * (int32)GameState->WorldMap->TileHeight))
		{
			NewCameraP.TileY -= TilesPerScreenY * 2;
			//real32 YShift = ((real32)TilesPerScreenY * GameState->WorldMap->TileHeight) * 2;
			//MoveAllEntities(GameState, 0, YShift);
		}
		#endif

		SetCamera(GameState, FollowEntity.Info->Pos);

		if (GameState->Camera.P.TileZ != FollowEntity.High->TileZ)
		{
			GameState->Camera.P.TileZ = FollowEntity.High->TileZ;
		}
	}
#endif
	//BEGIN RENDERING

	real32 ScreenTileCountX = (real32)RENDER_WIDTH / (real32)WorldMap->TileWidth;
	real32 ScreenTileCountY = (real32)RENDER_HEIGHT / (real32)WorldMap->TileHeight;
	vec2d ScreenCenter = {(RENDER_WIDTH * 0.5f), (RENDER_HEIGHT * 0.5f)};

	DrawBitmap(Buffer, &GameState->TestBitmap, 0.0f, 0.0f);

	vec2d MinStart = {RoundReal32(ScreenCenter.X - ((GameState->Camera.P.Offset.X + 0.5f) * WorldMap->TileWidth)),
					RoundReal32(ScreenCenter.Y + ((GameState->Camera.P.Offset.Y + 0.5f) * WorldMap->TileHeight))};

	for (int32 Row = FloorReal32ToInt32(-(ScreenTileCountY * 0.5f)); 
		Row <= CeilReal32ToInt32((ScreenTileCountY * 0.5f)); 
		Row++)
	{
		for (int32 Column = FloorReal32ToInt32(-(ScreenTileCountX * 0.5f)); 
			Column <= CeilReal32ToInt32((ScreenTileCountX * 0.5f)); 
			Column++)
		{
			tile_position P = {(uint32)(GameState->Camera.P.TileX + Column), 
					(uint32)(GameState->Camera.P.TileY + Row), GameState->Camera.P.TileZ, 0, 0};
			tile *Tile = GetTile(GameState, P.TileX, P.TileY, P.TileZ);
			if (Tile != NULL) {
				
				vec2d Min = MinStart + vec2d{((real32)Column * WorldMap->TileWidth), 
										   -(((real32)Row * WorldMap->TileHeight) + WorldMap->TileHeight)};
				bitmap *Texture = &GameState->EmptyTile;
				if (Tile->TileID == 1) {
					//Wall
					Texture = &GameState->EntityBitmaps[2][0];
				}
				if (Tile->TileID == 2) {
					//Grass
					Texture = &GameState->Grass[Tile->Variant]; 
				}
				if (Tile->TileID == 3) {
					//Dirt
					Texture = &GameState->Dirt[Tile->Variant];
				}
				if (Tile->TileID == 4) {
					//Gravel
					Texture = &GameState->Gravel[Tile->Variant];
				}
#if 0
				if (Tile->X == Player.P.TileX && Tile->Y == Player.P.TileY && Tile->Z == Player.P.TileZ)
				{
					Texture = &GameState->Wall;
				}
#endif				
				DrawBitmap(Buffer, Texture, Min);
			}
		}
	}

	for (uint32 EntityIndex = 0;
		EntityIndex < GameState->EntityCount;
		++EntityIndex)
	{

		if (GameState->EntitySpaces[EntityIndex] == Space_High)
		{
			entity Entity = GetEntity(GameState, Space_High, EntityIndex);
			if (Entity.High->TileZ == GameState->Camera.P.TileZ)
			{
				bitmap *Bitmap = &GameState->EntityBitmaps[Entity.Info->BitmapIndex][Entity.High->Facing];

				//vec2d Diff = TileDiff(&Entity.Low->Pos, &GameState->Camera.P);
				vec2d Min = {(SCREEN_X_OFFSET + ((Entity.High->Pos.X) 
								+ (ScreenCenter.X - (Bitmap->Header->Width / 2)))),
							(SCREEN_Y_OFFSET - ((Entity.High->Pos.Y)
								+ (ScreenCenter.Y + Bitmap->Header->Height 
									- ((Entity.Info->Height * 0.5f) * WorldMap->TileHeight))))};
				real32 RectMinX = Min.X + (Bitmap->Header->Width / 2) - ((Entity.Info->Width * WorldMap->TileWidth) / 2);
				real32 RectMinY = Min.Y + (Bitmap->Header->Height) - (Entity.Info->Height * WorldMap->TileHeight);
				DrawRectangle(Buffer, RectMinX, RectMinY, RectMinX + (Entity.Info->Width * WorldMap->TileWidth),
								RectMinY + (Entity.Info->Height * WorldMap->TileHeight), 1.0f, 0.0f, 0.0f);
				DrawBitmap(Buffer, Bitmap, Min);
			}
		}
	}
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, 400);
}