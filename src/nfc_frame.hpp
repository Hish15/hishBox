
#pragma once
#include <spdlog/spdlog.h>

#include <array>
#include <cstring>
#include <cstdint>
#include <span>
#include <vector>

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

