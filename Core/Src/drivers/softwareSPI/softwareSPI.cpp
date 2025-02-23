#include "softwareSPI.h"

softwareSPI::softwareSPI(const std::string& mosi, const std::string& miso, const std::string& sck)
    : mosi_pin(mosi, OUTPUT), miso_pin(miso, INPUT, PULLUP), sck_pin(sck, OUTPUT) {}

void softwareSPI::init() {
    sck_pin.set(true); // Ensure clock starts high
}

void softwareSPI::begin() {}

uint8_t softwareSPI::transfer(uint8_t ulVal) {
    uint8_t value = 0;
    sck_pin.set(false);

    for (int i = 7; i >= 0; i--) {
        mosi_pin.set(ulVal & (1 << i));
        sck_pin.set(true);
        if (miso_pin.get()) {
            value |= (1 << i);
        }
        sck_pin.set(false);
    }
    return value;
}

uint16_t softwareSPI::transfer16(uint16_t data) {
    uint16_t returnVal = 0;
    returnVal |= transfer((data >> 8) & 0xFF) << 8;
    returnVal |= transfer(data & 0xFF);
    return returnVal;
}

void softwareSPI::endTransaction() {}
