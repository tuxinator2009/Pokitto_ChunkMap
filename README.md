# Pokitto_ChunkMap
A helper class for streaming large maps from SD in TASMODE

This is an example project that uses the ChunkMap.h class to stream large maps from the SD card in TASMODE
Simply copy ChunkMap.h to your project (or put it in a global location) and follow these instructions:

I've put together a simple C++ class that allows for streaming large maps (theoretically up to ~130,000,000 tiles in both width and height). The idea is the maps need to be broken up into "chunks" of either 32x32 tiles (for 8x8 tiles) or 16x16 tiles (for 16x16 tiles).

Using it is somewhat trivial:
```cpp
//main.cpp
#include <Pokitto.h>
#include <miloslav.h>
#include "ChunkMap.h"
#include "tiles.h"

//We need an already open file handle to the map's chunk data.
//The ChunkMap class will use the file's current position so
//you can include other data in the same file
File mapData;
ChunkMap map;
//Some basic variables to determine where everything is at
//The coordinates are in pixels not tiles
int32_t playerX = 0;
int32_t playerY = 0;
int32_t screenX = playerX - 102;
int32_t screenY = playerY - 80;

void init()
{
    PD::loadRGBPalette(miloslav);
    PD::persistence = true;
    PD::invisiblecolor = 0;
    //Here we're just loading from a file that just contains the raw chunk data
    mapData.openRO("/data/chunks.dat");
    //We initialize the map (can be re-initialized at runtime when loading a new map)
    //The first parameter is a pointer to our already open File object
    //The second two parameters is the map's width and height in chunks
    //The fourth parameter is a pointer to the raw tile data
    //The map expects each tile ID to be offset into the tiles array by the size of one tile (64 bytes for 8x8 tiles and 256 bytes for 16x16 tiles).
    map.setup(&mapData, 4, 4, tiles);
}

void update()
{
    //Here's a brief demo of moving the player around based on what buttons are being pressed
	PB::pollButtons();
	if (PB::leftBtn())
		--playerX;
	else if (PB::rightBtn())
		++playerX;
	if (PB::upBtn())
		--playerY;
	else if (PB::downBtn())
		++playerY;
    //Now we clamp the player's location to somewhere within the map
	if (playerX < 0)
		playerX = 0;
	if (playerX > 1024 - 16)
		playerX = 1024 - 16;
	if (playerY < 0)
		playerY = 0;
	if (playerY > 1024 - 16)
		playerY = 1024 - 16;
    //We start the screen off with the player centered on screen (the player is currently 16x16 pixels)
	screenX = playerX - 102;
	screenY = playerY - 80;
    //Here we clamp the screen to also stay within the map (you can have the screen go outside the map, but you'll need to manually call Pokitto::Display::drawTile for each tile outside the map)
	if (screenX < 0)
		screenX = 0;
	if (screenX > 1024 - 220)
		screenX = 1024 - 220;
	if (screenY < 0)
		screenY = 0;
	if (screenY > 1024 - 176)
		screenY = 1024 - 176;
    //Now we tell the map to update itself with the given screen coordinates
    //The map will automatically load new chunks if needed
    //The global tileShift will be updated based on the screen coordinates and the tile's size
    //Finally the map will fill the global tile buffer with the tiles from the chunks
	map.update(screenX, screenY);
    //Now we simply draw the player sprite in screen coordinates
	PD::drawSpriteBitmap(playerX - screenX, playerY - screenY, 16, 16, playerSprite);
}
```
