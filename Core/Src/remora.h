#ifndef REMORA_H
#define REMORA_H

#include <memory>

#include "configuration.h"
#include "data.h"
#include "comms/commsInterface.h"
#include "comms/SPIComms.h"
#include "lib/ArduinoJson7/ArduinoJson.h"
//#include "states.h"
#include "modules/moduleFactory.h"
#include "modules/moduleList.h"
#include "thread/pruThread.h"

#include "modules/comms/commsHandler.h"

class JsonConfigHander; //forward declaration

class Remora {

private:

	JsonConfigHandler* configHandler;
	CommsHandler* comms;

    std::unique_ptr<pruThread> baseThread;
    std::unique_ptr<pruThread> servoThread;

    uint32_t baseFreq;
    uint32_t servoFreq;
    uint32_t commsFreq;

    void loadModules();

public:

	Remora();
    void setBaseFreq(uint32_t freq) { baseFreq = freq; }
    void setServoFreq(uint32_t freq) { servoFreq = freq; }
};


#endif
