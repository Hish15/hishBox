#include "media_player.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

void MediaPlayer::start_song(std::string path_to_file)
{


    // Play the sound
    //m_play_thread.request_stop();
    //m_play_thread.join(); //Wait for the thread to stop
    m_play_thread = std::jthread([path_to_file](std::stop_token stoken)
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
        while (sound.getStatus() == sf::Sound::Status::Playing)
        {
            if(stoken.stop_requested())
            {
                break;
            }
            // Leave some CPU time for other processes
            sf::sleep(sf::milliseconds(100));
        }
    });
}

void MediaPlayer::stop()
{
    if(m_play_thread.request_stop())
    {
        m_play_thread.join();
    }
}

