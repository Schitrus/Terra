#include "Audio.h"

void audio_callback(void* udata, Uint8* stream, int len) {
	if (audio_len == 0)
		return;

	len = (len > audio_len ? audio_len : len);

	SDL_memset(stream, 0, len);

	SDL_MixAudio(stream, audio_pos, len, volume/100.0*SDL_MIX_MAXVOLUME);

	audio_pos += len;
	audio_len -= len;

}

void Audio::init() {
	SDL_Init(SDL_INIT_AUDIO);
}

int Audio::load(const char* file_path) {

	if (SDL_LoadWAV(file_path, &wav_spec, &wav_buffer, &wav_length) == nullptr) {
		std::cout << "Failed to load file: " << file_path << std::endl;
		return -1;
	}

	wav_spec.callback = audio_callback;
	wav_spec.userdata = nullptr;

	restart();

	if (SDL_OpenAudio(&wav_spec, nullptr) < 0) {
		std::cout << "Failed to open audio" << std::endl;
		return -1;
	}
}

void Audio::setVolume(int volume) {
	::volume = volume;
}

int Audio::getVolume() {
	return volume;
}

void Audio::play() {
	SDL_PauseAudio(0);
}

void Audio::play(int volume) {
	setVolume(volume);
	SDL_PauseAudio(0);
}

void Audio::pause() {
	SDL_PauseAudio(1);
}

bool Audio::isDone() {
	return audio_len <= 0;
}

void Audio::restart() {
	audio_pos = wav_buffer;
	audio_len = wav_length;
}

Audio::~Audio() {
	SDL_CloseAudio();

	SDL_FreeWAV(wav_buffer);

	SDL_Quit();
}
