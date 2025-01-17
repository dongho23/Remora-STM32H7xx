#ifndef BLINK_H
#define BLINK_H

#include <cstdint>
#include <string>

#include "../../remora.h"
#include "../../modules/module.h"
#include "../../drivers/pin/pin.h"

/**
 * @brief Factory function to create a Blink module from JSON configuration.
 * 
 * @param config JSON object containing configuration parameters for the Blink module.
 * @return A unique pointer to the created Blink module.
 */
shared_ptr<Module> createBlink(const JsonObject& config);

/**
 * @class Blink
 * @brief A module for toggling a pin at a specific frequency.
 * 
 * The Blink class controls a GPIO pin, toggling its state at a specified frequency.
 */
class Blink : public Module
{

	private:

		bool 					bState;
		uint32_t 				periodCount;
		uint32_t 				blinkCount;

		std::unique_ptr<Pin> 	blinkPin;

	public:

		Blink(std::string _portAndPin, uint32_t _threadFreq, uint32_t _freq);

		virtual void update(void);
		virtual void slowUpdate(void);
};

#endif

