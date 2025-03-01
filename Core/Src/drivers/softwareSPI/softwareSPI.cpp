#include "SoftwareSPI.h"

SoftwareSPI::SoftwareSPI(const std::string& mosi, const std::string& miso, const std::string& sck)
    : mosi_pin(mosi, OUTPUT), miso_pin(miso, INPUT, PULLUP), sck_pin(sck, OUTPUT) {}

void SoftwareSPI::init() {
    sck_pin.set(true); // Ensure clock starts high
}

void SoftwareSPI::begin() {}

uint8_t SoftwareSPI::transfer(uint8_t ulVal) {
    uint8_t value = 0;
    sck_pin.set(false);

    for (int i = 7; i >= 1; i--) {
    	// Write bit
        !!(ulVal & (1 << i)) ? mosi_pin.set(true) : mosi_pin.set(false);
        // Start clock pulse
        sck_pin.set(true);
        // Read bit
        value |= ( miso_pin.get() ? 1 : 0) << i;
        // Stop clock pulse
        sck_pin.set(false);
    }

    !!(ulVal & (1 << 0)) ? mosi_pin.set(true) : mosi_pin.set(false);
    sck_pin.set(true);
    value |= ( miso_pin.get() ? 1 : 0) << 0;

    return value;
}

uint16_t SoftwareSPI::transfer16(uint16_t data) {
    uint16_t returnVal = 0;
    returnVal |= transfer((data >> 8) & 0xFF) << 8;
    returnVal |= transfer(data & 0xFF);
    return returnVal;
}

void SoftwareSPI::endTransaction() {}
