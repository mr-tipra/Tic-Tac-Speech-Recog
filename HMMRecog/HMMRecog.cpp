// HMMRecog.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
typedef long double ldouble;

#include "HMM.h"
#include "Frame.h"
#include "Codebook.h"

#include <SDL/SDL.h>
#include "Audio.h"
#include "Timer.h"
#include "Game.h"
#include "Texture.h"
#include "TicTacToe.h"

#include <SDL/SDL_image.h>


int _tmain(int argc, _TCHAR* argv[])
{	
	
	Codebook cb = Codebook::readCodebook("../codebook.txt");	
	int N=5, M=cb.size();

	std::vector<std::string> words;
	for (int i = 0; i <= 8; i++) words.push_back(std::to_string(i));
	
	int nw = words.size();

	//for(int i=0; i<nw; i++){
	//	auto& hmm = HMM::train(30, 5, 2, N, cb, "..\\my recordings\\" + words[i], words[i]);
	//	//auto& hmm = HMM::train(20, 5, 2, N, cb, "..\\194101013_digit", words[i]);
	//	std::stringstream ss;
	//	ss.str("");
	//	ss << "..\\hmms\\" << words[i] << "_hmm.txt";
	//	hmm.saveState(ss.str());
	//}
	

	std::vector<HMM> hmm(nw);

	for(int i=0; i<nw; i++){
		std::stringstream ss;
		ss << "..\\hmms\\" << words[i] <<"_hmm.txt";
		hmm[i].readState(ss.str(), N, M);
	}

	//int corr = 0, cnt = 5;
	//for (int i = 0; i < nw; i++) 
	//	corr += HMM::test(words, hmm, cb, "..\\my recordings\\" + words[i], words[i], cnt, 20);
	//	//corr += HMM::test(words, hmm,cb,  "..\\194101013_digit" ,words[i], 10, 21);
	//std::cout << (float)corr / (cnt*nw) << "\n";


	const int WIDTH = 600, HEIGHT = 511;
	Game game("TIC TAC", WIDTH, HEIGHT, 200, 100);

	int boardTex = game.loadTexture("..\\res\\images\\board.png");
	int crossTex = game.loadTexture("..\\res\\images\\cross.png");
	int oTex = game.loadTexture("..\\res\\images\\o.png");

	Audio& audio = Audio::getInstance();
	audio.setBuffer(1);
	
	//Text
	game.loadFont("..\\res\\fonts\\LibreBaskerville-Bold.ttf", 25);
	std::string currText = "Press S to say the position!";
	SDL_Color textColor = { 25, 25, 25,255 };
	//Tic Tac Toe
	TicTacToe tictac;
	auto prevState = tictac.getState();


	game.setColor(0, 0, 0);

	
	while (game.getState() == Game::State::RUNNING) {
	
		game.clearWindow();
		
		
		tictac.drawBoard(game, boardTex,crossTex, oTex);

		auto size = game.getTextSize(currText);
		game.renderText(currText, WIDTH/2 - size.w/2, HEIGHT - size.h, textColor);
		if (tictac.getState() != prevState) {
			switch (tictac.getState()) {
			case TicTacToe::State::X_WON:
				currText = "X Won! R to reset";
				textColor = { 200, 0, 0, 255 };
				break;
			case TicTacToe::State::O_WON:
				currText = "O Won! R to reset";
				textColor = { 0, 0, 200, 255 };
				break;
			case TicTacToe::State::DRAW:
				currText = "Draw! R to Reset";
				textColor = { 25,25,25, 255 };
				break;
			default:
				currText = "S to say position!";
				textColor = { 25,25,25,255 };
				break;
			}
			prevState = tictac.getState();
		}
		
		if (audio.getState() == Audio::RecordingState::RECORDED) {
		
			int w = HMM::predict(words, hmm, cb, Audio::recordingBuffer, audio.bufferSize());
			std::cout << words[w] << "\n";
			bool valid = tictac.enter('X', w);
			if (!valid) {
				currText = "Didn't get that! S to say again!";

			}
			else {
				tictac.aiMove('O');
				currText = "You Chose " + words[w];
				textColor = { 25, 25, 25, 255 };
			}
			audio.reset();
		
		}
		
		SDL_Event e;
		while (game.getEvent(&e)) {
			if (e.type == SDL_QUIT)
				game.close();

			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
					case SDLK_ESCAPE:
						game.close();
						break;
					case SDLK_s:
						audio.record();
						break;
					case SDLK_r:
						tictac.reset();
						currText = "S to Choose!";
						break;
				}
			}
		}

		game.updateWindow();

	}


	return 0;
}

