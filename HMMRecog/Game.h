#pragma once

#include <SDL/SDL.h>
#include <exception>
#include <iostream>
#include <unordered_map>
#include "util.h"
#include "Texture.h"
#include "Text.h"


class Game {
public:
	enum class State {
		RUNNING,
		QUIT
	};
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	int width, height;
	int x, y;
	State currState;

	//Texture stuff
	int textureId = 0;
	std::vector<Texture*> textures;
	//Text Rendering
	Text textRenderer;

public:
	

	Game(const std::string& windowName, int w, int h, int x, int y):
		width(w), height(h), x(x), y(y)
	{
		//Init SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			logError("SDL NOT INIT");
		}
		window = SDL_CreateWindow(windowName.c_str(), x, y, w, h, SDL_WINDOW_SHOWN);
		if (window == nullptr) {
			logError("Window Not Created");
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (renderer == nullptr) logError("Renderer not created\n");
		currState = State::RUNNING;
	}

	~Game() {
		if (window) {
			SDL_DestroyWindow(window);
			SDL_Quit();
		}
		for (Texture* tex : textures) {
			if (tex) delete tex;
		}
	}

	bool getEvent(SDL_Event* e) {		
		return SDL_PollEvent(e) != 0;
	}

	State getState() {
		return currState;
	}

	void close() {
		currState = State::QUIT;
	}
	void setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) {
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
	}
	void clearWindow() {
		SDL_RenderClear(renderer);
	}
	void updateWindow() {
		SDL_RenderPresent(renderer);
	}
	
	inline int getWidth() {
		return width;
	}
	inline int getHeight() {
		return height;
	}
	//Texuture Related
	
	//loads texture and returns an id
	int loadTexture(const std::string& path) {
		//create Texture
		Texture* tex = new Texture(path, renderer);
		textures.push_back(tex);
		return textureId++;
	}

	void drawTexture(int id, SDL_Rect* dstRect = nullptr, SDL_Rect* srcRect = nullptr) {
		if (id >= (int)textures.size()) return;
		Texture* tex = textures[id];
		if (tex != nullptr) {
			SDL_RenderCopy(renderer, tex->getTexture(), srcRect, dstRect);
		}
	}

	//Text Related
	void loadFont(const std::string& path, int size) {
		textRenderer.loadFont(path, size);
	}
	void renderText(const std::string& text, int x, int y, SDL_Color color = { 255, 255,255,255 }) {
		textRenderer.render(text, renderer, x, y, color);
	}

	SDL_Rect getTextSize(const std::string& text) {
		return textRenderer.getSize(text);
	}
};