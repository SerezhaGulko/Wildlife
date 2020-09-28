#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <cstdio>
#include <time.h>
#include <random>
#include <algorithm>


static enum state
{
	IDLE, WALKING
};

class animal 
{
public:
	state c_state = IDLE;
	float speed = 1.0f;
	float x, y;
	float target_x, target_y;

	void UPDATE()
	{
		switch(c_state)
		{
		case IDLE:
			if (rand() % 100 > 70)
				c_state = WALKING;
			break;
		case WALKING:
			if(abs(target_x - x) < 0.1f && abs(target_y - y) < 0.1f)
			{
				if (rand() % 100 > 10)
					c_state = IDLE;
				else 
				{
					target_x = speed * ((float)rand() / RAND_MAX - 0.5f) + x;
					target_y = speed * ((float)rand() / RAND_MAX - 0.5f) + y;
					if (target_x > 1000.0f) target_x = 1000.0f;
					if (target_y > 1000.0f) target_y = 1000.0f;
					if (target_x < 0.0f) target_x = 0.0f;
					if (target_y < 0.0f) target_y = 0.0f;
				}
			}
			else 
			{
				if(abs(target_x - x) > 0.1f) x += (target_x - x);
				if(abs(target_y - y) > 0.1f) y += (target_y - y);
			}
			break;
		}
	}
};

enum tile_type {
	grass, sand, water, rocks, forest
};

class tile {
public:
	tile_type t_type;
	void Update()
	{

	}
	tile() {}
	tile(tile_type t_type) { this->t_type = t_type; }
};

class tile_map {
public:
	tile** tilemap = nullptr;
	int width, height;

	void Update()
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
				tilemap[j][i].Update();
		}
	}
	void generate_map(int width, int height)
	{
		srand(time(0));
		this->width = width; this->height = height;
		if (tilemap != nullptr)
		{
			for (int i = 0; i < height; i++)
			{
				delete[] tilemap[i];
			}
			delete[] tilemap;
		}

		tilemap = new tile * [height];
		for (int h = 0; h < height; h++)
		{
			tilemap[h] = new tile[width];
			for (int w = 0; w < width; w++)
			{
				int rng_num = rand() % 10000;
				if (rng_num < 4500 && w > 0) {
					// left tile
					tilemap[h][w].t_type = tilemap[h][w - 1].t_type;
				}
				else if (rng_num > 4490 && rng_num < 9500 && h > 0) {
					// top tile
					tilemap[h][w].t_type = tilemap[h - 1][w].t_type;
				}
				else {
					// random tile
					tilemap[h][w].t_type = tile_type(rand() % 5);
				}
			}
		}
	}

};

class wildlife : public olc::PixelGameEngine
{
public:
	wildlife()
	{
		sAppName = "Wildlife";
	}

private:
	tile_map map;
	animal* animals;
	int animals_size;
	float xCamera = 0.0f;
	float yCamera = 0.0f;
	float fscale = 1.0f;

public:
	bool OnUserCreate() override
	{
		srand(time(0));
		map.generate_map(1000, 1000);

		//generate random animals
		animals_size = 1000;
		animals = new animal[animals_size];
		for(int i = 0; i < animals_size; i++)
		{
			animals[i].x = rand() % 1000;
			animals[i].y = rand() % 1000;
			animals[i].target_x = animals[i].x + 1.0f;
			animals[i].target_y = animals[i].y + 1.0f;
		}
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Lambda for getting a tile type
		auto get_tile = [&](int x, int y)
		{
			if (x >= 0 && x < map.width && y >= 0 && y < map.height)
				return map.tilemap[y][x].t_type;
			else
				return grass;
		};


		// User input
		if(IsFocused())
		{
			if(GetKey(olc::Key::RIGHT).bHeld)
			{
				xCamera += 100.0f * fElapsedTime;
			}
			if (GetKey(olc::Key::LEFT).bHeld)
			{
				xCamera -= 100.0f * fElapsedTime;
			}
			if (GetKey(olc::Key::UP).bHeld)
			{
				yCamera -= 100.0f * fElapsedTime;
			}
			if (GetKey(olc::Key::DOWN).bHeld)
			{
				yCamera += 100.0f * fElapsedTime;
			}
			if(GetKey(olc::Key::W).bHeld) 
			{
				fscale += 1.0001 * fElapsedTime;
			}
			if (GetKey(olc::Key::S).bHeld)
			{
				fscale -= 0.9999 * fElapsedTime;
			}
		}

		// Draw map
		int nTileWidth = max(int(8 * fscale), 1);
		int nTileHeight = nTileWidth;
		int nVisibleTilesX = ScreenWidth() / nTileWidth;
		int nVisibleTilesY = ScreenHeight() / nTileHeight;

		float foffsetX = xCamera - (float)nVisibleTilesX / 2.0f;
		float foffsetY = yCamera - (float)nVisibleTilesY / 2.0f;
		if (xCamera < 0) xCamera = 0;
		if (xCamera > map.width) xCamera = map.width;
		if (yCamera < 0) yCamera = 0;
		if (yCamera > map.width) yCamera = map.height;
		if (foffsetX < 0) foffsetX = 0;
		if (foffsetY < 0) foffsetY = 0;
		if (foffsetX > map.width - nVisibleTilesX) foffsetX = map.width - nVisibleTilesX;
		if (foffsetY > map.height - nVisibleTilesY) foffsetY = map.height - nVisibleTilesY;

		for(int x = 0; x < nVisibleTilesX + 1; x++)
		{
			for(int y = 0; y < nVisibleTilesY + 1; y++)
			{
				switch(get_tile(x + foffsetX, y + foffsetY))
				{
				case grass:
					FillRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::DARK_GREEN);
					DrawRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::BLACK);
					break;
				case forest:
					FillRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::VERY_DARK_GREEN);
					DrawRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::BLACK);
					break;
				case sand:
					FillRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::YELLOW);
					DrawRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::BLACK);
					break;
				case water:
					FillRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::BLUE);
					DrawRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::BLACK);
					break;
				case rocks:
					FillRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::GREY);
					DrawRect(x * nTileWidth, y * nTileHeight, nTileWidth, nTileHeight, olc::BLACK);
					break;
				default:
					throw 1;
				}
			}
		}

		int circle_size = (nTileWidth + 1) / 1.5f;
		for (int i = 0; i < animals_size; i++)
		{
			animals[i].UPDATE();
			if (animals[i].x >= foffsetX && animals[i].y >= foffsetY &&
				animals[i].x < nVisibleTilesX + foffsetX && animals[i].y < nVisibleTilesY + foffsetY) {
			FillCircle(olc::vi2d((animals[i].x - foffsetX) * nTileWidth, (animals[i].y - foffsetY) * nTileHeight), circle_size, olc::BLACK);
			FillCircle(olc::vi2d((animals[i].x - foffsetX) * nTileWidth, (animals[i].y - foffsetY) * nTileHeight), 1, olc::WHITE);
			}
		}
		return true;
	}
};


int main()
{
	wildlife demo;
	if (demo.Construct(640, 480, 8, 8, true, true))
		demo.Start();

	return 0;
}
