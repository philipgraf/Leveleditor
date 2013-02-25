/*
 * leveleditor.cpp
 *
 *  Created on: 06.01.2013
 *      Author: philip
 */

#include <iostream>
#include <fstream>
#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>

using namespace std;

int main(int argc, char *argv[]) {

	if (argc != 2) {
		cout << "Usage: " << argv[0] << " mapconfigFile" << endl;
		return 1;
	}

	const int MAXTILE =22;
	const int TILESIZE = 32;

	std::string mapname;
	int mapWidth;
	int mapHeight;
	std::string bgName;
	int gravity;
	int time;

	bool startposition = false;

	std::fstream filestream;
	filestream.open(argv[1], std::fstream::in);
	filestream >> mapname >> mapWidth >> mapHeight >> bgName >> gravity >> time;
	filestream.close();

	u_int64_t ***tilelist = NULL;
	tilelist = new u_int64_t**[3];

	for (int i = 0; i < 3; i++) {
		tilelist[i] = new u_int64_t*[mapWidth];
		for (int j = 0; j < mapWidth; j++) {
			tilelist[i][j] = new u_int64_t[mapHeight];
			for (int k = 0; k < mapHeight; k++) {
				tilelist[i][j][k] = 0;
			}
		}
	}

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Surface *screen = SDL_SetVideoMode(800, 600, 32,
			SDL_HWSURFACE | SDL_DOUBLEBUF);

	SDL_Surface *tmp = SDL_LoadBMP("img/tiles.bmp");
	SDL_Surface *tileset = SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);
	tmp = IMG_Load(("img/" + bgName).c_str());
	SDL_Surface *bg = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);
	tmp = SDL_LoadBMP("img/player.bmp");
	SDL_Surface *player = SDL_DisplayFormat(tmp);

	SDL_SetColorKey(player, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(player->format, 255, 0, 255));
	SDL_SetColorKey(bg, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(bg->format, 255, 0, 255));
	SDL_SetColorKey(tileset, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(tileset->format, 255, 0, 255));

	SDL_Event event;

	bool running = true;
	int mouseX;
	int mouseY;
	int currentTile = 0;
	int currentLayer = 1;

	while (running) {
		// Eventhandling
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_SPACE:
					currentTile = currentTile >= MAXTILE ? 0 : currentTile + 1;
					std::cout << "Current Tile: " << currentTile << std::endl;
					break;
				case SDLK_l:
					currentLayer = currentLayer >= 2 ? 0 : currentLayer + 1;
					std::cout << "Current Layer: " << currentLayer << std::endl;
					break;
				case SDLK_ESCAPE:
					running = false;
					break;
				case SDLK_s:
					startposition = true;
					break;
				default:
					break;
				}
				break;
			case SDL_MOUSEMOTION:
				mouseX = event.motion.x;
				mouseY = event.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (startposition) {
					tilelist[1][mouseX / TILESIZE][mouseY / TILESIZE] = 65536;
					startposition = false;
				} else {
					tilelist[currentLayer][mouseX / TILESIZE][mouseY / TILESIZE] =
							currentTile;
				}
				break;
			}
		}

		// logic

		// render
		SDL_BlitSurface(bg, NULL, screen, NULL);

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < mapWidth; j++) {
				for (int k = 0; k < mapHeight; k++) {
					if (tilelist[i][j][k] >> 16) {
						switch (tilelist[i][j][k] >> 16) {
						case 1:
							SDL_Rect srcRect;
							srcRect.x = 0;
							srcRect.y = 0;
							srcRect.w = TILESIZE;
							srcRect.h = TILESIZE * 2;
							SDL_Rect destRect;
							destRect.x = j * TILESIZE;
							destRect.y = k * TILESIZE;
							destRect.w = TILESIZE;
							destRect.h = TILESIZE * 2;
							SDL_BlitSurface(player, &srcRect, screen,
									&destRect);
							break;
						}
					} else {
						SDL_Rect srcRect;
						srcRect.x = 0;
						srcRect.y = TILESIZE * (tilelist[i][j][k] & 0xFFFFFFFF);
						srcRect.w = srcRect.h = TILESIZE;
						SDL_Rect destRect;
						destRect.x = j * TILESIZE;
						destRect.y = k * TILESIZE;
						destRect.w = destRect.h = TILESIZE;
						SDL_BlitSurface(tileset, &srcRect, screen, &destRect);
					}
				}
			}
		}

		if (startposition) {
			rectangleRGBA(screen, (mouseX / TILESIZE) * TILESIZE,
					(mouseY / TILESIZE) * TILESIZE,
					(mouseX / TILESIZE) * TILESIZE + 32,
					(mouseY / TILESIZE) * TILESIZE + 64, 255, 0, 0, 255);
		} else {
			SDL_Rect srcRect;
			srcRect.x = 0;
			srcRect.y = TILESIZE * currentTile;
			srcRect.w = srcRect.h = TILESIZE;
			SDL_Rect destRect;
			destRect.x = (mouseX / TILESIZE) * TILESIZE;
			destRect.y = (mouseY / TILESIZE) * TILESIZE;
			destRect.w = destRect.h = TILESIZE;
			SDL_BlitSurface(tileset, &srcRect, screen, &destRect);
		}

		SDL_Flip(screen);

	}

	//save file
	filestream.open((mapname + ".map").c_str(), std::fstream::out);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < mapHeight; j++) {
			for (int k = 0; k < mapWidth; k++) {
				filestream << tilelist[i][k][j] << " ";
			}
			filestream << "\n";
		}
		filestream << ";" << std::endl;
	}



	filestream.close();

	SDL_FreeSurface(screen);
	SDL_Quit();

	return 0;
}
