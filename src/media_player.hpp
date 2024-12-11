#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <thread>
#include <iostream>


class MediaPlayer
{
    std::jthread m_play_thread;
    public:
    void start_song(std::string path_to_file)
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
 
// Play the sound
sound.play();

        // Loop while the sound is playing
        while (sound.getStatus() == sf::Sound::Status::Playing)
        {
            // Leave some CPU time for other processes
            sf::sleep(sf::milliseconds(100));

            // Display the playing position
            std::cout << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        " << std::flush;
        }

        std::cout << '\n' << std::endl;
    }
};
