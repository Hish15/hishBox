#pragma once
extern "C"{
#include "pn532.h"
#include "pn532_rpi.h"
}
#include <spdlog/spdlog.h>

#include <cstdint>
#include <expected>
#include <optional>
#include <vector>
#include <string>

class NfcReader
{

	PN532 pn532;

	uint8_t buff[255];
	uint8_t uid[MIFARE_UID_MAX_LENGTH];
	uint32_t pn532_error = PN532_ERROR_NONE;
	int32_t uid_len = 0;

	NfcReader() = default;
	public:
	static std::expected<NfcReader, std::string> try_init();
	std::vector<uint8_t> read_card();


};
