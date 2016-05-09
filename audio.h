#ifndef SRC_ENGINE_AUDIO_H_
#define SRC_ENGINE_AUDIO_H_

#define SDL_MAIN_HANDLED

#include <iostream>
#include <list>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

namespace Engine {

    class Audio {

    static bool initialized;
    static int max_channels;
    static std::list<int> next_channel;

    inline static int GetChannel (void) {
        if (!Audio::initialized) {
            Audio::Init();
        }
        if (!Audio::next_channel.empty()) {
            int channel = Audio::next_channel.front();
            Audio::next_channel.pop_front();
            return channel;
        }
        throw std::string("No more channels available");
    }

    inline static void FreeChannel (const int channel) { if (Audio::initialized && channel >= 0 && channel < Audio::max_channels) Audio::next_channel.push_front(channel); }

    public:

        static void AddChannels(const int channels);

        inline static void Init (void) {
            if (!Audio::initialized) {

                Audio::initialized = true;

                if (SDL_Init(SDL_INIT_AUDIO) < 0) {
                    throw std::string(SDL_GetError());
                }

                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        			throw std::string(Mix_GetError());
        		}

                Audio::AddChannels(128);
            }
        }

        inline static void End (void) {
            Audio::max_channels = 0;
            Mix_HaltChannel(-1);
            Mix_AllocateChannels(0);
            next_channel.clear();
            Audio::initialized = false;
        }

        class Sound {

            Mix_Chunk *sound = nullptr;
            int channel = -1, volume = MIX_MAX_VOLUME;
            bool paused = false, started = false;

        public:

            inline Sound (void) {}
            inline Sound (const std::string &file) { this->getChannel(), this->load(file); }

            inline bool validSound (void) const { return this->sound != nullptr; }
            inline bool validChannel (void) const { return this->channel >= 0; }
            inline bool valid (void) const { return this->validSound() && this->validChannel(); }

            inline void getChannel (void) { this->freeChannel(), this->channel = Audio::GetChannel(), this->setVolume(volume); }

            inline void freeSound (void) { if (this->validSound()) Mix_FreeChunk(this->sound); }
            inline void freeChannel (void) { if (this->validChannel()) Audio::FreeChannel(this->channel), this->channel = -1; }
            inline void free (void) { this->stop(), this->freeChannel(), this->freeSound(); }

            inline void setVolume (int _volume) { if (this->validChannel()) this->volume = std::min(std::max(0, _volume), MIX_MAX_VOLUME), Mix_Volume(this->channel, this->volume); }
            inline void mute (void) { this->setVolume(0); }
            inline void maxVolume (void) { this->setVolume(MIX_MAX_VOLUME); }
            inline int getVolume (void) const { return this->volume; }

            inline void load (const std::string &file) {
                if (!this->validChannel()) {
                    this->channel = Audio::GetChannel();
                }
                this->freeSound();
                this->sound = Mix_LoadWAV(file.c_str());
            }

            inline void start (const int loops = 0) {
                if (this->valid()) {
                    Mix_PlayChannel(this->channel, this->sound, loops);
                }
                this->started = true;
                this->paused = false;
            }

            inline void play () {
                if (!this->started || !this->paused) {
                    this->start();
                } else if (this->valid()) {
                    Mix_Resume(this->channel);
                }
                this->paused = false;
            }

            inline void pause (void) {
                if (this->validChannel()) {
                    Mix_Pause(this->channel);
                }
                this->paused = true;
            }

            inline void stop (void) {
                if (this->validChannel()) {
                    Mix_HaltChannel(this->channel);
                }
                this->started = false;
                this->paused = false;
            }

            inline void fadeIn (const int ms, const int loops = 0) const { if (this->valid()) Mix_FadeInChannel(this->channel, this->sound, loops, ms); }
            inline void fadeOut (const int ms) { if (this->validChannel()) Mix_FadeOutChannel(this->channel, ms); }

            inline operator bool (void) const { return this->valid(); }
        };

    };

};

#endif
