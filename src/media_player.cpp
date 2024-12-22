#include "media_player.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

void MediaPlayer::start_song(std::string path_to_file)
{

    // Play the sound
    //m_play_thread.request_stop();
    //m_play_thread.join(); //Wait for the thread to stop
    m_play_thread = std::jthread([path_to_file, this](std::stop_token stoken)
    {
        // Declare a new sound buffer
        sf::SoundBuffer buffer;

        // Load it from a file
        if (!buffer.loadFromFile(path_to_file))
        {
            // error...
        }

        // Create a sound source and bind it to the buffer
        sf::Sound sound;
        sound.setBuffer(buffer);
        sound.play();
        // Loop while the sound is playing
        bool old_pause = this->m_pause;
        while (sound.getStatus() == sf::Sound::Status::Playing || sound.getStatus() == sf::Sound::Status::Paused)
        {

            if(stoken.stop_requested())
            {
                break;
            }
            if(this->m_pause != old_pause)
            {
                old_pause = !old_pause;
                if(old_pause)
                {
                    sound.pause();
                }
                else
                {
                    sound.play();
                }
            }
            // Leave some CPU time for other processes
            sf::sleep(sf::milliseconds(100));
        }

    });
}

void MediaPlayer::load_list(std::span<std::string> list_songs)
{
    if(list_songs.size() == 0)
    {
        return;
    }
    m_list_songs = std::vector<std::string>(list_songs.begin(), list_songs.end());
    start_song(m_list_songs[m_current_song]);
}

void MediaPlayer::pause()
{
    m_pause = true;
}
void MediaPlayer::play()
{
    m_pause = false;
}

void MediaPlayer::stop()
{
    if(m_play_thread.request_stop())
    {
        m_play_thread.join();
    }
}
void MediaPlayer::next()
{
    stop();
    m_current_song++;
    if(m_current_song >= m_list_songs.size())
    {
        m_current_song = 0;
    }
    start_song(m_list_songs[m_current_song]);
}
void MediaPlayer::previous()
{
    stop();
    if(m_current_song == 0)
    {
        m_current_song = m_list_songs.size() - 1;
    }
    else
    {
        m_current_song--;
    }
    start_song(m_list_songs[m_current_song]);
}

