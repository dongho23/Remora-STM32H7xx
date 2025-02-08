#ifndef TMCMODULE_H
#define TMCMODULE_H

#include <cstdint>
#include <string>

#include "../../remora.h"
#include "../../modules/module.h"
#include "../../drivers/TMCStepper/TMCStepper.h"

class TMC : public Module
{
  protected:

    float       Rsense;

  public:

    virtual void update(void) = 0;           // Module default interface
    virtual void configure(void) = 0;
};


class TMC2208 : public TMC
{
  protected:

    std::string rxtxPin;     // default to half duplex
    uint16_t    mA;
    uint16_t    microsteps;
    bool        stealth;

    TMC2208Stepper* driver;

  public:

    // SW Serial pin, Rsense, mA, microsteps, stealh
    TMC2208(std::string, float, uint16_t, uint16_t, bool);
    static std::shared_ptr<Module> create(const JsonObject& config, Remora* instance);
    ~TMC2208();

    void update(void);           // Module default interface
    void configure(void);
};


class TMC2209 : public TMC
{
  protected:

    std::string rxtxPin;     // default to half duplex
    uint16_t    mA;
    uint16_t    microsteps;
    bool        stealth;
    uint8_t     addr;
    uint16_t    stall;

    TMC2209Stepper* driver;

  public:

    // SW Serial pin, Rsense, addr, mA, microsteps, stealh, hybrid, stall
    // TMC2209(std::string, float, uint8_t, uint16_t, uint16_t, bool, uint16_t);
    TMC2209(std::string, float, uint8_t, uint16_t, uint16_t, bool, uint16_t);
    static std::shared_ptr<Module> create(const JsonObject& config, Remora* instance);
    ~TMC2209();

    void update(void);           // Module default interface
    void configure(void);
};


#endif
