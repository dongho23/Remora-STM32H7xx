#ifndef SOFTWARESPI_H
#define SOFTWARESPI_H

#include <cstdint>
#include <string>
#include "../../drivers/pin/pin.h"

class SoftwareSPI {
public:
    SoftwareSPI(const std::string& mosi, const std::string& miso, const std::string& sck);

    void init();
    void begin();
    uint8_t transfer(uint8_t ulVal);
    uint16_t transfer16(uint16_t data);
    void endTransaction();

private:
    Pin mosi_pin;
    Pin miso_pin;
    Pin sck_pin;
};

#endif
