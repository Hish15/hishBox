extern "C"{
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "pn532.h"
#include "pn532_rpi.h"


}

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <span>
#include <optional>
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

#pragma pack(push, 1)
class NfcFrame
{
public:
    struct locked_t
    {
        std::array<uint8_t, 8> uid;
        uint32_t internal;
        std::array<uint8_t, 4> capability_container;

    }locked;

    static_assert(sizeof(locked) == 16, "This needs to be packed");
    static std::vector<uint8_t> get_payload(std::span<uint8_t> raw_frame)
    {
        if(raw_frame.size() < 4) //TODO find a better value
        {
            return {};
        }
        NfcFrame frame;
        
        auto it_raw_frame = raw_frame.begin();

        std::memcpy(&frame.locked, &*it_raw_frame, sizeof(frame.locked));
        it_raw_frame += sizeof(frame.locked);
        const uint8_t constrol_char = *it_raw_frame;
        it_raw_frame++;
        if(raw_frame[4*4] != 0x03)
        {
            spdlog::error("Got unexpected for first byte of block 4. got {}, expected 0x03", raw_frame[4*4] );
            return {}; 

        }
        const uint8_t full_payload_size = *it_raw_frame;
        spdlog::info("full payload size is {}", full_payload_size);
        if(full_payload_size == 0xFF) // Long URL not handled for now
        {
            spdlog::error("Long frame are not handled for now");
            return {};
            
        }
        it_raw_frame++;
        const uint8_t record_header = *it_raw_frame;
        it_raw_frame++;
        const uint8_t type_length = *it_raw_frame;
        it_raw_frame++;
        const uint8_t payload_length = *it_raw_frame;
        it_raw_frame++;
        const uint8_t type_name = *it_raw_frame;
        it_raw_frame++;
        if(type_name != 'U') //Only handle URI for now
        {
            
            spdlog::error("Got unexpected Type name {} ", type_name );
            return {};
        }
        const uint8_t identifier_code = *it_raw_frame;
        it_raw_frame++;
        std::vector<uint8_t> payload;
        spdlog::info("identifier_code is {}", identifier_code);
        if(identifier_code == 0x04)
        {
            payload.insert(payload.end(), {'h', 't', 't', 'p', 's', ':', '/', '/'});
        }

        payload.insert(payload.end(), it_raw_frame, it_raw_frame + payload_length -1); //Minus 1 since the identifier il already read
        return payload;
    }

};
#pragma pack(pop)


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
    std::string a(payload.begin(), payload.end());
    std::cout << a << std::endl;
    return 0;
}
