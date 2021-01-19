#pragma once

#include <SDL/SDL.h>

class Timer {
public:
	Timer():mStarted(false), mPaused(false), mStartTicks(0), mPausedTicks(0)
	{}

	void start(){
		mStarted = true;
		mPaused = false;
		mStartTicks = SDL_GetTicks();
		mPausedTicks = 0;
	}

	void stop(){
		mStarted = false;
		mPaused = false;
		mStartTicks = 0;
		mPausedTicks = 0;
	}

	void pause(){
		if (mStarted && !mPaused) {
			mPaused = true;
			mPausedTicks = SDL_GetTicks() - mStartTicks;
			mStartTicks = 0;
		}
	}

	void unpause(){
		if (mStarted && mPaused) {
			mPaused = false;
			mStartTicks = SDL_GetTicks() - mPausedTicks;
			mPausedTicks = 0;

		}
	}

	Uint32 getTicks(){
		Uint32 time = 0;
		if (mStarted) {
			if (mPaused) {
				time = mPausedTicks;
			}
			else {
				time = SDL_GetTicks() - mStartTicks;
			}
		}
		return time;
	}

	bool isStarted(){
		return mStarted;
	}
	bool isPaused(){
		return mStarted && mPaused ;
	}
private:
	Uint32 mStartTicks;
	Uint32 mPausedTicks;

	bool mPaused;
	bool mStarted;

};
