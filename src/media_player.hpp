#pragma once

#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <span>

class MediaPlayer
{
    std::atomic<bool> m_pause = false;
    std::jthread m_play_thread;
    std::vector<std::string> m_list_songs;
    std::atomic<std::size_t> m_current_song = 0;
    void start_song(std::string path_to_file);
public:
    void load_list(std::span<std::string> list_songs);
    void pause();
    void play();
    void toggle_pause_play();
    void stop();
    void next();
    void previous();
};
