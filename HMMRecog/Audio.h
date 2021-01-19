#pragma once

// SDL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <SDL\SDL.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
class Audio {
private:
	static void audioRecordingCallback(void* userData, Uint8* stream, int len) {
		if (bufferBytePosition > bufferByteMaxPosition) 
			return;
		
		memcpy(&recordingBuffer[bufferBytePosition], stream, len);
		bufferBytePosition += len;
		
	}

	bool initialized = false;
	static SDL_AudioDeviceID recordingDeviceId;
	static Uint32 bufferDataSize;
	static Uint32 bufferBytePosition;
	static Uint32 bufferByteMaxPosition;
	SDL_AudioSpec receivedAudioSpec;
	std::thread* waitThread = nullptr;

	//singleton class
	static Audio&  audio;

public:
	static Uint8* recordingBuffer;
	enum class RecordingState {
		SELECTING_DEVICE,
		STOPPED,
		RECORDING, 
		RECORDED,
		ERROR
	};


private:
	std::atomic<RecordingState> currState = RecordingState::STOPPED;

	Audio(Audio& other) {}
	Audio& operator=(const Audio& other){}

	Audio() {
		if (SDL_Init(SDL_INIT_AUDIO) < 0) {
			std::cerr << "SDL AUDIO NOT INITIALIZED\n";
			initialized = false;
			currState = RecordingState::ERROR;
			return;
		}
		currState = RecordingState::SELECTING_DEVICE;
		//Get Capture Devices
		int recordingCount = SDL_GetNumAudioDevices(SDL_TRUE);
		if (recordingCount < 1) {
			std::cerr << "RECORDING DEVICE NOT FOUND\n";
			currState = RecordingState::ERROR;
			return;
		}

		//default Audio Spec
		SDL_AudioSpec desiredAudioSpec;
		SDL_zero(desiredAudioSpec);
		desiredAudioSpec.freq = 16000;
		desiredAudioSpec.format = AUDIO_S16;
		desiredAudioSpec.channels = 1;
		desiredAudioSpec.samples = 4096;
		desiredAudioSpec.callback = audioRecordingCallback;
		recordingDeviceId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0, SDL_TRUE), SDL_TRUE, &desiredAudioSpec, &receivedAudioSpec,
			0);
		if (recordingDeviceId == 0) {
			currState = RecordingState::ERROR;
			std::cerr << "RECORDING DEVICE COULD NOT BE OPENED\n";
			return;
		}
			
		//device opened
		initialized = true;
	}
	~Audio() {
		if (recordingBuffer != nullptr) delete[] recordingBuffer;
		if (waitThread != nullptr) {
			waitThread->join();
			delete waitThread;
		}

		SDL_CloseAudioDevice(recordingDeviceId);
	}
public:
	static Audio& getInstance() {
		return audio;
	}

	void setBuffer(int seconds) {
		if (!initialized) return;
		int bytesPerSample = receivedAudioSpec.channels * (SDL_AUDIO_BITSIZE(receivedAudioSpec.format) / 8);
		int bytesPerSecond = receivedAudioSpec.freq * bytesPerSample;
		bufferDataSize = bytesPerSecond * (seconds+1);
		bufferByteMaxPosition = (seconds)* bytesPerSecond;
		if (recordingBuffer != nullptr) 
			delete[] recordingBuffer;
		recordingBuffer = new Uint8[bufferDataSize];
		memset(recordingBuffer, 0, bufferDataSize);
		currState = RecordingState::STOPPED;
	}



	RecordingState getState() {
		return currState;
	}
	void reset() {
		currState = RecordingState::STOPPED;
	}
	Uint32 bufferSize() {
		return bufferByteMaxPosition;
	}

	void waitForRecording() {
		
		bool loop = true;
		while (loop) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			SDL_LockAudioDevice(recordingDeviceId);
			if (bufferBytePosition > bufferByteMaxPosition) {
				SDL_PauseAudioDevice(recordingDeviceId, SDL_TRUE);
				currState = RecordingState::RECORDED;
				loop = false;
			}
			SDL_UnlockAudioDevice(recordingDeviceId);
		}
	}

	void record() {
		if (currState == RecordingState::STOPPED) {
			if (waitThread != nullptr) {
				waitThread->join();
				delete waitThread;
			}

			std::cout << "RECORD START\n";
			bufferBytePosition = 0;
			SDL_PauseAudioDevice(recordingDeviceId, SDL_FALSE);
			currState = RecordingState::RECORDING;

			waitThread = new std::thread([this]() {
				this->waitForRecording();
			});
		}		
	}
};

Audio&  Audio::audio = Audio();

Uint8* Audio::recordingBuffer = nullptr;
Uint32 Audio::bufferDataSize = 0;
Uint32 Audio::bufferBytePosition=0;
Uint32 Audio::bufferByteMaxPosition=0;
SDL_AudioDeviceID Audio::recordingDeviceId=0;