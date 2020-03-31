#pragma once

#include "SDL/SDL.h"
#include <iostream>

static Uint32 audio_len;
static Uint8* audio_pos;

static int volume = 100;

void audio_callback(void* udata, Uint8* stream, int len);

class Audio
{
public:
	void init();

	int load(const char* file_path);

	void setVolume(int volume);

	int getVolume();

	void play();
	void play(int volume);

	void pause();

	bool isDone();

	void restart();

	~Audio();
private:
	SDL_AudioSpec wav_spec;
	Uint32 wav_length;
	Uint8* wav_buffer;
};

