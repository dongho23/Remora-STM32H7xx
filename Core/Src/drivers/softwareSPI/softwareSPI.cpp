#include "SoftwareSPI.h"

SoftwareSPI::SoftwareSPI(const std::string& mosi, const std::string& miso, const std::string& sck)
    : mosi_pin(mosi, OUTPUT), miso_pin(miso, INPUT, PULLUP), sck_pin(sck, OUTPUT), delayTicks(2) {
}

void SoftwareSPI::init()
{
	sck_pin.set(true);
}

void SoftwareSPI::begin() {}

uint8_t SoftwareSPI::transfer(uint8_t data) {
    uint8_t received = 0;

    sck_pin.set(false);

    for (uint8_t i = 7; i >=1; i--) {
    	!!(data & (1<<i))? mosi_pin.set(true) : mosi_pin.set(false);
    	sck_pin.set(true);
    	received |= (miso_pin.get() ? 1 : 0) << i;
    	sck_pin.set(false);
    }

    !!(data & (1<<0))? mosi_pin.set(true) : mosi_pin.set(false);
    sck_pin.set(true);
    received |= (miso_pin.get() ? 1 : 0) << 0;

    return received;
}

uint16_t SoftwareSPI::transfer16(uint16_t data) {
    uint16_t returnVal = 0;
    returnVal |= transfer((data >> 8) & 0xFF) << 8;
    returnVal |= transfer(data & 0xFF);
    return returnVal;
}

void SoftwareSPI::endTransaction() {}
