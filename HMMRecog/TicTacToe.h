#pragma once

#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "Game.h"

class TicTacToe {

public:
	enum class State {
		NONE,
		DRAW,
		X_WON,
		O_WON
	};
private:
	std::vector<char> board;
	State currState = State::NONE;
	std::vector<std::vector<int>> winnings = {
		{0, 1, 2},
		{3, 4, 5},
		{6, 7, 8},
		{0, 3, 6},
		{1, 4, 7},
		{2, 5, 8},
		{0, 4, 8},
		{6, 4, 2}
	};

	char checkWon() {


		bool full = true;
		for (char c : board) {
			if (c == ' ') full = false;
		}

		for (auto& winning : winnings) {
			bool same = true;
			char prev = board[winning[0]];
			for (auto& index : winning) {
				if (board[index] != prev) {
					same = false;
					break;
				}
			}
			if (same && board[winning[0]] != ' ') return board[winning[0]];
		}
		if (full) return 'D';

		return -1;
	}

	void updateState() {
		char win = checkWon();
		switch (win) {
			case 'X':
				currState = State::X_WON;
				break;
			case 'O':
				currState = State::O_WON;
				break;
			case 'D':
				currState = State::DRAW;
				break;
		}
	}

public:

	TicTacToe():board(9, ' ') {
		//init board
		
	}

	bool enter(char ch, int index) {
		if (currState != State::NONE) return false;
		if (index < 0 || index > 8) return false;
		if (ch != 'X' && ch != 'O') return false;
		if (board[index] != ' ') return false;
		board[index] = ch;
		updateState();
		return true;
	}

	bool aiMove(char c) {
		char opponentChar = (c == 'X') ? 'O' : 'X';

		//winning
		for (auto& winning : winnings) {
			int cntC = 0;
			cntC += board[winning[0]] == c;
			cntC += board[winning[1]] == c;
			cntC += board[winning[2]] == c;
			if (cntC == 2) {
				for (auto index : winning) {
					if (board[index] == ' ') {
						enter(c, index);
						return true;
					}
					
				}
			}
			
		}

		//block
		for (auto& winning : winnings) {
			int cntO = 0;
			cntO += board[winning[0]] == opponentChar;
			cntO += board[winning[1]] == opponentChar;
			cntO += board[winning[2]] == opponentChar;
			if (cntO == 2) {
				for (auto index : winning) {
					if (board[index] == ' ') {
						enter(c, index);
						return true;
					}
				}
			}
		}
		//cornerning and rest
		for (int corner : {0, 2, 6, 8, 1,3, 5, 7}) {
			if (board[corner] == ' ') {
				enter(c, corner);
				return true;
			}
		}
		//board full
		return false;
	}

	void drawBoard(Game& game, int boardTex, int crossTex, int oTex) {
		int boardWidth = game.getWidth(), boardHeight = game.getHeight();
		int tileLen = 100;

		int cenX = boardWidth / 2 - tileLen / 2, cenY = boardHeight / 2 - tileLen / 2;

		game.drawTexture(boardTex);

		for (int i = 0; i < 9; i++) {
			char ch = board[i];
			int r = i / 3, c = i % 3;
			int dx = (c-1) * boardWidth / 4, dy = (r-1) * boardWidth / 4;
			SDL_Rect rect = { cenX + dx, cenY + dy, tileLen, tileLen };
			if (ch == 'X')
				game.drawTexture(crossTex, &rect);
			else if (ch == 'O')
				game.drawTexture(oTex, &rect);
			
		}
	
	}
	State getState() {
		return currState;
	}
	void reset() {
		for (char& c : board) c = ' ';
		currState = State::NONE;
	}
	
};