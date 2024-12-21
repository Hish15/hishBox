#pragma once

#include <thread>
#include <iostream>


class MediaPlayer
{
    std::jthread m_play_thread;
public:
    void start_song(std::string path_to_file);
    void stop();
};
