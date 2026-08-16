#ifndef GPU_H
#define GPU_H

#include <vector>
#include <array>
#include <span>
#include <cmath>
#include "../port.h"

template<typename T>
struct vector{
	T x, y;
};

const vector<int> screenSize = vector<int>{1920, 1080};

struct sprite{
	uint32_t frame;
	bool isLeft;
	vector<float> position;
};

enum GPUCmd : uint32_t {
	DRAW_FRAME = 0x00,
	
	// get the SET_SPRITE cmd, then the index of the sprite, then the next cmd, then the data values
	SET_SPRITE = 0x01,
	SPRITE_POS = 0x02,
	SPRITE_FACE = 0x03,
	SPRITE_FRAME = 0x04,
	
	// get the SET_TILE cmd, then the tile x and then y, and then the new tile index
	SET_TILE = 0x05,
	
	HALT = 0x06
};

struct GPU{
	std::vector<uint32_t> screen;
	vector<float> camPos;
	int tileSize;
	std::vector<sprite> sprites;
	std::vector<uint16_t> tilemap;
	vector<int> mapSize;
	std::vector<uint8_t> tilePalette;
	std::vector<uint8_t> spritePalette;
	std::array<uint32_t, 256> colorPalette;
	
	IOPort commandQueue;
	
	GPU(int tilesize, int numSprites, int numFrames, int numTiles, vector<int> mapS) : tileSize(tilesize), mapSize(mapS) {
		sprites.resize(numSprites);
		tilemap.resize(mapSize.x * mapSize.y);
		tilePalette.resize(numTiles * tileSize * tileSize);
		spritePalette.resize(numFrames * tileSize * tileSize);
		screen.resize(screenSize.y * screenSize.x);
	}
	
	std::span<const uint8_t> fetchSpriteFrame(const sprite& Sprite){
		return std::span<const uint8_t>(spritePalette.data() + (Sprite.frame * tileSize * tileSize), tileSize * tileSize);
	}
	
	std::span<const uint8_t> fetchTile(int tileIndex){
		return std::span<const uint8_t>(tilePalette.data() + (tileIndex * tileSize * tileSize), tileSize * tileSize);
	}
	
	void drawTile(int tileIndex, vector<int> pixelPos){
		std::span<const uint8_t> tileData = fetchTile(tileIndex);
		
		auto pixel = tileData.begin();
		
		for(int y = 0; y < tileSize; y++){
			int screenY = pixelPos.y + y;
			if(screenY < 0 || screenY > screenSize.y) continue;
			for(int x = 0; x < tileSize; x++){
				int screenX = pixelPos.x + x;
				if(screenX < 0 || screenX > screenSize.x) continue;
				uint8_t colorIndex = *pixel;
				pixel++;
				
				uint32_t color = colorPalette[colorIndex];
				
				screen[(screenY * screenSize.x) + screenX] = color;
			}
		}
	}
	
	void drawTiles(){
		vector<int> scroll = vector<int>{-std::abs((int)camPos.x % tileSize), -std::abs((int)camPos.y % tileSize)};
		vector<int> floorPos = vector<int>{std::floor(camPos.x / (float)tileSize), std::floor(camPos.y / (float)tileSize)};
		
		for(int y = 0; y <= screenSize.y/tileSize; y++){
			for(int x = 0; x <= screenSize.x/tileSize; x++){
				int tileIndex = tilemap[(floorPos.y + y) * mapSize.x + (floorPos.x + x)];
				
				drawTile(tileIndex, vector<int>{x * tileSize + scroll.x, y * tileSize + scroll.y});
			}
		}
	}
	
	void drawSprite(sprite& Sprite, vector<int> pixelPos){
		std::span<const uint8_t> spriteData = fetchSpriteFrame(Sprite);
		
		auto pixel = spriteData.begin();
		
		for(int y = 0; y < tileSize; y++){
			int screenY = pixelPos.y + y;
			if(screenY < 0 || screenY > screenSize.y) continue;
			for(int x = 0; x < tileSize; x++){
				int screenX = pixelPos.x + x;
				if(screenX < 0 || screenX > screenSize.x) continue;
				uint8_t colorIndex = *pixel;
				pixel++;
				
				uint32_t color = colorPalette[colorIndex];
				
				screen[(screenY * screenSize.x) + screenX] = color;
			}
		}
	}
	
	void drawSprites(){
		vector<float> lowViewBound = vector<float>{camPos.x - ((float)screenSize.x / (float)tileSize), camPos.y - ((float)screenSize.y / (float)tileSize)};
		vector<float> highViewBound = vector<float>{camPos.x + ((float)screenSize.x / (float)tileSize), camPos.y + ((float)screenSize.y / (float)tileSize)};
		
		for(int i = 0; i < sprites.size(); ++i){
			if(sprites[i].position.x > lowViewBound.x - 1 && sprites[i].position.y > lowViewBound.y - 1 && sprites[i].position.x < highViewBound.x && sprites[i].position.y < highViewBound.y){
				vector<float> relativePos = vector<float>{-(camPos.x - sprites[i].position.x), -(camPos.x - sprites[i].position.x)};
				vector<int> screenPos = vector<int>{(int)(relativePos.x * tileSize) + (screenSize.x / 2), (int)(relativePos.y * tileSize) + (screenSize.y / 2)};
				drawSprite(sprites[i], screenPos);
			}
		}
	}
	
	void processCommand(GPUCmd instruction){
		switch(instruction) {
			case DRAW_FRAME:
				drawTiles();
				drawSprites();
				break;
				
			case SET_SPRITE: {
				uint32_t index;
				uint32_t cmd;
				if(commandQueue.pollToHost(index)){
					if(commandQueue.pollToHost(cmd)){
						if(cmd == SPRITE_POS){
							uint32_t xPos;
							if(commandQueue.pollToHost(xPos)){
								sprites[index].position.x = (xPos & 0xFFFFFFFF);
							}
							uint32_t yPos;
							if(commandQueue.pollToHost(yPos)){
								sprites[index].position.y = (yPos & 0xFFFFFFFF);
							}
						} else if(cmd == SPRITE_FACE){
							sprites[index].isLeft = !sprites[index].isLeft;
						} else if(cmd == SPRITE_FRAME){
							commandQueue.pollToHost(sprites[index].frame);
						}
					}
				}
				break;
			}
			case SET_TILE: {
				uint32_t x, y;
				uint32_t val;
				
				commandQueue.pollToHost(x);
				commandQueue.pollToHost(y);
				commandQueue.pollToHost(val);
				tilemap[y * mapSize.x + x] = ((val << 16) & 0xFFFF);
			}
			default:
				break;
		}
	}
	
	void Run(){
		uint32_t cmd;
		bool isCmd;
		do {
			commandQueue.tick();
			isCmd = commandQueue.pollToHost(cmd);
			if(isCmd) processCommand(static_cast<GPUCmd>(cmd));
		} while (cmd != HALT);
	}
};

#endif
