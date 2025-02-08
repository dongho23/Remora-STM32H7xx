#ifndef REMORA_H
#define REMORA_H

#include <memory>

#include "configuration.h"
#include "data.h"
#include "comms/commsInterface.h"
#include "lib/ArduinoJson7/ArduinoJson.h"
#include "modules/moduleFactory.h"
#include "modules/moduleList.h"
#include "thread/pruThread.h"

#include "modules/comms/commsHandler.h"  //TODO figure out why this is not being included from moduleList.h


#define MAJOR_VERSION 	2
#define MINOR_VERSION	0
#define PATCH			0


class JsonConfigHander; //forward declaration

class Remora {
private:

    enum State {
        ST_SETUP = 0,
        ST_START,
        ST_IDLE,
        ST_RUNNING,
        ST_STOP,
        ST_RESET,
        ST_SYSRESET
    };

    enum State currentState;
    enum State prevState;

    volatile txData_t*  ptrTxData;
    volatile rxData_t*  ptrRxData;
    volatile bool reset;

	std::unique_ptr<JsonConfigHandler> configHandler;
	std::shared_ptr<CommsHandler> comms;

    std::unique_ptr<pruThread> baseThread;
    std::unique_ptr<pruThread> servoThread;

    uint32_t baseFreq;
    uint32_t servoFreq;
    uint32_t commsFreq;

    bool threadsRunning;

    void transitionToState(State);
    void handleSetupState();
    void handleStartState();
    void handleIdleState();
    void handleRunningState();
    void handleResetState();
    void handleSysResetState();
    void startThread(const std::unique_ptr<pruThread>&, const char*);
    void resetBuffer(volatile uint8_t*, size_t);
    void loadModules();

public:

	Remora();
	void run();
    void setBaseFreq(uint32_t freq) { baseFreq = freq; }
    void setServoFreq(uint32_t freq) { servoFreq = freq; }

    volatile txData_t* getTxData() { return &txData; }
    volatile rxData_t* getRxData() { return &rxData; }
    volatile bool* getReset() { return &reset; };
};


#endif
