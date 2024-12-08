extern "C"{
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "pn532.h"
#include "pn532_rpi.h"


}
#include <nfc_frame.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstring>
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

int main(int argc, char** argv) {
    uint8_t buff[255];
    uint8_t uid[MIFARE_UID_MAX_LENGTH];
    uint32_t pn532_error = PN532_ERROR_NONE;
    int32_t uid_len = 0;
    printf("Hello!\r\n");
    PN532 pn532;
    //PN532_SPI_Init(&pn532);
    PN532_I2C_Init(&pn532);
    //PN532_UART_Init(&pn532);
    if (PN532_GetFirmwareVersion(&pn532, buff) == PN532_STATUS_OK) {
        printf("Found PN532 with firmware version: %d.%d\r\n", buff[1], buff[2]);
    } else {
        return -1;
    }
    PN532_SamConfiguration(&pn532);
    printf("Waiting for RFID/NFC card...\r\n");
    while (1)
    {
        // Check if a card is available to read
        uid_len = PN532_ReadPassiveTarget(&pn532, uid, PN532_MIFARE_ISO14443A, 1000);
        if (uid_len == PN532_STATUS_ERROR) {
            printf(".");
        } else {
            printf("Found card with UID: ");
            for (uint8_t i = 0; i < uid_len; i++) {
                printf("%02x", uid[i]);
            }
            printf("\r\n");
            break;
        }
    }
    printf("Reading blocks...\r\n");
    std::vector<uint8_t> raw_frame;
    raw_frame.reserve(512);
        for (uint8_t block_number = 0; block_number < 135; block_number++) {
        pn532_error = PN532_Ntag2xxReadBlock(&pn532, buff, block_number);
        if (pn532_error != PN532_ERROR_NONE) {
            break;
        }
        printf("%d: ", block_number);
        raw_frame.insert(raw_frame.end(), buff, buff + 4);
        for (uint8_t i = 0; i < 4; i++) {
            printf("%02x ", buff[i]);
        }
        printf("................");
        for (uint8_t i = 0; i < 4; i++) {
            printf("%i ", buff[i]);
        }
        printf("\r\n");
    }
    if (pn532_error) {
        printf("Error: 0x%02x\r\n", pn532_error);
    }
    auto payload = NfcFrame::get_payload(raw_frame);
    std::string url(payload.begin(), payload.end());
    const auto id = get_youtube_id(url);
    if(id.empty() == false)
    {
	    const auto matching_path = get_matching_path("youtube",id);
	    if(matching_path.empty())
	    {
		    std::string download_cmd = fmt::format("docker run --rm -it -v \"/home/hish/hishBox/youtube/:/downloads:rw\" ghcr.io/jauderho/yt-dlp:latest {} --extract-audio --audio-format wav",url );

		    std::system(download_cmd.c_str());
	    }
	    const auto matching_path2 = get_matching_path("youtube", id);
	    const auto run_cmd = fmt::format("mplayer \"{}\"", matching_path2);
	    spdlog::info(run_cmd);
	    std::system(run_cmd.c_str());
    }
    else
    {
	spdlog::error("The URL is not handled yet");
    }
    return 0;
}
