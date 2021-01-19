#pragma once

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "util.h"
#include <unordered_map>
class Text {
private:
	static bool INITIALIZED;
	
	TTF_Font* mFont;

	struct TextTexture {
		SDL_Texture* texture = nullptr;
		int width, height;
	};

	std::unordered_map<std::string, TextTexture> strToText;

public:
	Text():
	mFont(nullptr){
		if (INITIALIZED == false) {
			if (TTF_Init() == -1)
				logError("SDL Tiff not initialized");
			INITIALIZED = true;
		}
	}
	~Text() {
		for (auto& p : strToText) {
			if (p.second.texture)
				SDL_DestroyTexture(p.second.texture);
		}
		if(mFont) TTF_CloseFont(mFont);
		TTF_Quit();
	}
	void loadFont(const std::string& font, int size) {
		mFont = TTF_OpenFont(font.c_str(), size);
		if (mFont == nullptr) {
			logError(font + " NOT LOADED\n");
			return;
		}
	}
	
	void render(const std::string& text, SDL_Renderer* renderer, int x, int y, SDL_Color color = { 255, 255, 255, 255 }) {
		
		if (strToText.count(text) == 0) {
			//allocate texture
			SDL_Surface* surf = TTF_RenderText_Blended(mFont, text.c_str(), color);
			if (surf == nullptr) {
				logError(text + " surface NOT created\n");
				return;
			}
			
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
			if (texture == nullptr) {
				SDL_FreeSurface(surf);
				logError(text + " Texutre not created\n");
				return;
			}
			strToText[text] = { texture, surf->w, surf->h };
			SDL_FreeSurface(surf);
		}
		//render
		SDL_Rect target = { x, y, strToText[text].width, strToText[text].height };
		SDL_RenderCopy(renderer, strToText[text].texture, NULL, &target);
	}

	SDL_Rect getSize(const std::string& text) {
		int w, h;
		TTF_SizeText(mFont, text.c_str(), &w, &h);
		return { 0, 0, w, h };
	}
	


};

bool Text::INITIALIZED = false;