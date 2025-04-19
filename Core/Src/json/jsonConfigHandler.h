
#ifndef JSON_CONFIG_HANDLER_H
#define JSON_CONFIG_HANDLER_H

#include <memory>
#include <string>
#include "../lib/ArduinoJson7/ArduinoJson.h"
#include "fatfs.h"

class Remora; //forward declaration

class JsonConfigHandler {
private:

	Remora* remoraInstance;
	std::string jsonContent = "";
	const char* filename = "config.txt";
	JsonDocument doc;
	JsonObject thread;
	bool configError;
	bool readFileContents();
	bool parseJson();

public:
	JsonConfigHandler(Remora* _remora);
	bool loadConfiguration();
	void updateThreadFreq();
	JsonArray getModules();
	JsonObject getModuleConfig(const char* threadName, const char* moduleType);
};
#endif
