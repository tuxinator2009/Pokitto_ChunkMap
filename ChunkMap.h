#pragma once

#include <Pokitto.h>
#include <File>
#include <cstdint>

#if PROJ_TILE_W == 8
#define CHUNK_SIZE 1024
#elif PROJ_TILE_W == 16
#define CHUNK_SIZE 256
#endif

class ChunkMap
{
	public:
		void setup(File *f, uint8_t chunksWide, uint8_t chunksHigh, const uint8_t *tiledata)
		{
			file = f;
			fileStart = file->tell();
			tiles = tiledata;
			width = chunksWide;
			height = chunksHigh;
			for (int i = 0; i < 4; ++i)
			{
				chunkIDs[i] = -1;
				chunkX[i] = 0;
				chunkY[i] = 0;
			}
		}
		void update(int32_t cameraX, int32_t cameraY)
		{
			//First we need to determine how many pixels to shift the tile at 0,0
			int8_t chunkXOffset, chunkYOffset;
			int8_t shiftX = cameraX % PROJ_TILE_W;
			int8_t shiftY = cameraY % PROJ_TILE_H;
			if (shiftX < 0)
				shiftX += PROJ_TILE_W;
			if (shiftY < 0)
				shiftY += PROJ_TILE_H;
			//Next we calculate the top left chunk's coordinates (in chunks)
			chunkX[0] = cameraX / 256;
			chunkY[0] = cameraY / 256;
			chunkXOffset = cameraX / PROJ_TILE_W;
			chunkYOffset = cameraY / PROJ_TILE_H;
			if (cameraX < 0)
			{
				chunkX[0] = -1;
				if (shiftX != 0)
					--chunkXOffset;
			}
			if (cameraY < 0)
			{
				chunkY[0] = -1;
				if (shiftY != 0)
					--chunkYOffset;
			}
			//Determine the offset of the chunk based on the camera location
			chunkXOffset -= chunkX[0] * 32;
			chunkYOffset -= chunkY[0] * 32;
			for (int i = 1; i < 4; ++i)
			{
				chunkX[i] = chunkX[0] + (i % 2);
				chunkY[i] = chunkY[0] + (i / 2);
			}
			Pokitto::Display::shiftTilemap(shiftX, shiftY);
			for (int i = 0; i < 4; ++i)
			{
				if (chunkX[i] < 0 || chunkX[i] >= width || chunkY[i] < 0 || chunkY[i] >= height)
					chunkIDs[i] = -1;
				else
				{
					if (chunkIDs[i] != chunkY[i] * width + chunkX[i])
					{
						chunkIDs[i] = chunkY[i] * width + chunkX[i];
						file->seek(fileStart + chunkIDs[i] * CHUNK_SIZE);
						file->read(buffer + i * CHUNK_SIZE, CHUNK_SIZE);
					}
					drawChunk(buffer + i * CHUNK_SIZE, (i % 2) * 32 - chunkXOffset, (i / 2) * 32 - chunkYOffset);
				}
			}
		}
	private:
		void drawTiles(const uint8_t *array, int numTiles, int x, int y, const uint8_t *tileArray)
		{
			if (y < 0) return;
			if (y > 22) return;
			if (x < 0)
			{
				array -= x;
				numTiles += x;
				x = 0;
			}
			if (x + numTiles >= 29)
				numTiles = 29 - x;
			while (numTiles-- > 0)
				Pokitto::Display::drawTile(x++, y, tiles + *array++ * 64);
		}
		void drawChunk(const uint8_t *chunk, int x, int y)
		{
			for (int cy = 0; cy < 32; ++cy)
				drawTiles(chunk + cy * 32, 32, x, y + cy, tiles);
		}
		File *file;
		uint32_t fileStart;
		uint8_t buffer[CHUNK_SIZE * 4];
		uint8_t chunkIDs[4];
		uint8_t chunkX[4];
		uint8_t chunkY[4];
		const uint8_t *tiles;
		uint8_t width;
		uint8_t height;
};
