#pragma once

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "util.h"

class Texture {
private:
	static bool INITIALIZED;
	SDL_Texture* texture = nullptr;
public:
	Texture(const std::string& path, SDL_Renderer* renderer) {
		if (INITIALIZED == false) {
			if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0)
				logError("SDL Image not initialized");
			INITIALIZED = true;
		}
		SDL_Surface* surf = IMG_Load(path.c_str());
		if (surf == nullptr) {
			logError(path + " NOT FOUND\n");
			return;
		}

		texture = SDL_CreateTextureFromSurface(renderer, surf);
		if (texture == nullptr) {
			SDL_FreeSurface(surf);
			logError(path + " Texutre not created\n");
			return;
		}
		SDL_FreeSurface(surf);
	}

	~Texture() {
		if (texture) {
			SDL_DestroyTexture(texture);
		}
	}

	SDL_Texture* getTexture() {
		return texture;
	}
};

bool Texture::INITIALIZED = false;