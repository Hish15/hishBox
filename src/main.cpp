extern "C"{
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "pn532.h"
#include "pn532_rpi.h"
#include <wiringPi.h>
}

#include <nfc_reader.hpp>
#include <nfc_frame.hpp>
#include <media_player.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstring>
#include <chrono>
#include <iostream>
#include <optional>
#include <regex>
#include <filesystem>
#include <span>
#include <string>
#include <vector>
/*

   +-------------------------+----------------------------------------------------------------
   | D1                      | Record header (MB = ME = 1, CF = 0, SR = 1, IL = 0, TNF = 0x1)
   +-------------------------+----------------------------------------------------------------
   | 01                      | Type Length (1 byte)
   +-------------------------+----------------------------------------------------------------
   | 0B                      | Payload Length (11 bytes)
   +-------------------------+----------------------------------------------------------------
   | 55                      | Type Name ("U")
   +-------------------------+----------------------------------------------------------------
   | 02 67 6F 6F 67 6C 65 2E | Payload: Identifier code = 2 (prefix "https://www."),
   | 63 6F 6D                |          truncated URI = "google.com"
   +-------------------------+----------------------------------------------------------------
   */


std::string get_youtube_id(std::string url)
{
	std::smatch m;
	if(std::regex_search(url, m, std::regex("youtu\\.?be")) == false)
	{
		return "";
	}
	if(std::regex_search(url, m, std::regex("v=([a-zA-Z0-9]+)")))
	{
		return m[1];
	}
	else if(std::regex_search(url, m, std::regex("//.[^/]+/([a-zA-Z]+)"))) 
	{
		return m[1];
	}
	return "";
}

std::string get_matching_path(std::string_view directory, std::string_view pattern)
{
	// directory_iterator can be iterated using a range-for loop
	for (auto const& dir_entry : std::filesystem::directory_iterator{directory}) 
	{
		const bool is_file = std::filesystem::is_regular_file(dir_entry);
		if(is_file == false)
		{
			continue;
		}
		const std::string tested_file = dir_entry.path().string();
		if(dir_entry.path().string().contains(pattern))
			return tested_file;
	}
	return "";

}

#define PIN_BUTTON 18

MediaPlayer media_player;

#include <functional>
struct InitGpio
{
	int pin;
	int mode;
	enum class pull {off = 0, up, down};
	pull pull;
	std::function<void(void)> callback;
};

class GpioHandler
{
public:
	GpioHandler(std::initializer_list<InitGpio> list_gpio)
	{
		for(const auto& gpio : list_gpio)
		{
			wiringPiSetupGpio();
			pinMode(gpio.pin, gpio.mode);
			if (gpio.mode == INPUT)
			{
				switch(gpio.pull)
				{
					case InitGpio::pull::off:
						pullUpDnControl(gpio.pin, PUD_OFF);
						break;
					case InitGpio::pull::up:
						pullUpDnControl(gpio.pin, PUD_UP);
						break;
					case InitGpio::pull::down:
						pullUpDnControl(gpio.pin, PUD_DOWN);
						break;
				}
			}
			wiringPiISR(gpio.pin, INT_EDGE_FALLING, gpio.callback.target<void(void)>());
		}
	}
};

int main(int argc, char** argv) {

	GpioHandler gpio_handler({
		{PIN_BUTTON, INPUT, InitGpio::pull::up, [](){media_player.next();}}
	});

	std::vector<std::string> list_songs;
	list_songs.push_back("./youtube/Nicki Minaj - Anaconda [LDZX4ooRsWs].wav");
	list_songs.push_back("./youtube/We Don't Talk About Bruno (From ＂Encanto＂⧸Lyric Video) [-IFD4ozm1DA].wav");
    //TEST
    media_player.load_list(list_songs);
    using namespace std::chrono_literals;

	while(1)
	{
		if(digitalRead(PIN_BUTTON) == LOW)
		{
			media_player.next();
			std::this_thread::sleep_for(1s);
		}
	}
	media_player.play();
    
	std::this_thread::sleep_for(10s);
    media_player.pause();
	std::this_thread::sleep_for(5s);
	media_player.next();
	std::this_thread::sleep_for(10s);
    //FIN TEST
	std::expected<NfcReader, std::string> try_reader = NfcReader::try_init();
	if(try_reader.has_value() == false)
	{
		spdlog::error(try_reader.error());
		return -1;
	}
	NfcReader& reader = try_reader.value();
	printf("Waiting for RFID/NFC card...\r\n");
	printf("Reading blocks...\r\n");
	std::vector<uint8_t> raw_frame = reader.read_card();

	auto payload = NfcFrame::get_payload(raw_frame);

	std::string url(payload.begin(), payload.end());
	const auto id = get_youtube_id(url);
    MediaPlayer media_player;
	if(id.empty() == false)
	{
		const auto matching_path = get_matching_path("youtube",id);
		if(matching_path.empty())
		{
			std::string download_cmd = fmt::format("docker run --rm -it -v \"/home/hish/hishBox/youtube/:/downloads:rw\" ghcr.io/jauderho/yt-dlp:latest {} --extract-audio --audio-format wav",url );

			std::system(download_cmd.c_str());
		}
		const auto matching_path2 = get_matching_path("youtube", id);
		spdlog::info(matching_path2);
		std::array<std::string, 1> song = {matching_path2};
        media_player.load_list(song);
	}
	else
	{
		spdlog::error("The URL is not handled yet");
	}
	return 0;
}
