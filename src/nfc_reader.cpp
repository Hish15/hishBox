
#include "nfc_reader.hpp"
	std::expected<NfcReader, std::string> NfcReader::try_init()
	{
		NfcReader reader;	
		PN532_I2C_Init(&reader.pn532);
		//PN532_UART_Init(&pn532);
		if (PN532_GetFirmwareVersion(&reader.pn532, reader.buff) == PN532_STATUS_OK) {
			printf("Found PN532 with firmware version: %d.%d\r\n", reader.buff[1], reader.buff[2]);
		} else {
			return std::unexpected("Could not find PN532 device");
		}
		PN532_SamConfiguration(&reader.pn532);
		return reader;
	}
std::vector<uint8_t> NfcReader::read_card()
{

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
	return raw_frame;
}
