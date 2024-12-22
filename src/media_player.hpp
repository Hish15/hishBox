#pragma once

#include <atomic>
#include <thread>
#include <iostream>


class MediaPlayer
{
    std::atomic<bool> m_pause = false;
    std::jthread m_play_thread;
public:
    void start_song(std::string path_to_file);
    void pause();
    void play();
    void stop();
};
