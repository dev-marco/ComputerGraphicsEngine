#include "audio.h"

namespace Engine {

    bool Audio::initialized = false;
    int Audio::max_channels = 0;
    std::list<int> Audio::next_channel;

    void Audio::AddChannels (const int channels) {
        if (Audio::initialized) {

            int old = Audio::max_channels;

            Audio::max_channels += channels;

            for (; old < Audio::max_channels; ++old) {
                Audio::next_channel.push_back(old);
            }

            Mix_AllocateChannels(Audio::max_channels);
        }
    }

}
