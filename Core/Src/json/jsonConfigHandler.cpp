
#include "jsonConfigHandler.h"
#include "../remora.h"


JsonConfigHandler::JsonConfigHandler(Remora* _remora) :
	remoraInstance(_remora),
	configError(false)
{
	loadConfiguration();
    updateThreadFreq();
}

bool JsonConfigHandler::loadConfiguration() {
	// Clear any existing configuration
    jsonContent.clear();
    doc.clear();

    // Read and parse the configuration file
    if (!readFileContents()) {
        return false;
    }
    parseJson();
    return true;
}


void JsonConfigHandler::updateThreadFreq() {
    if (configError) return;

    printf("\n5. Updating thread frequencies\n");

    JsonArray Threads = doc["Threads"];

    // create objects from JSON data
    for (JsonArray::iterator it=Threads.begin(); it!=Threads.end(); ++it) {
        thread = *it;
        const char* configor = thread["Thread"];
        uint32_t    freq = thread["Frequency"];
        if (!strcmp(configor,"Base")) {
            printf("	Setting BASE thread frequency to %lu\n", freq);
            remoraInstance->setBaseFreq(freq);
        }
        else if (!strcmp(configor,"Servo")) {
            printf("	Setting SERVO thread frequency to %lu\n", freq);
            remoraInstance->setServoFreq(freq);
        }
    }
}


JsonArray JsonConfigHandler::getModules() {
	if (doc["Modules"].is<JsonVariant>())
        return doc["Modules"].as<JsonArray>();
    else
        return JsonArray();
}

// Method to get specific module configurations
/*
JsonObject JsonConfigHandler::getModuleConfig(const char* threadName, const char* moduleType) {
    if (!doc.containsKey("Modules")) {
        // printf error here and return error code
    }
    JsonArray modules = doc["Modules"];
    for (JsonObject module : modules) {
        if (strcmp(module["Thread"], threadName) == 0 &&
            strcmp(module["Type"], moduleType) == 0) {
            return module;
        }
    }
    return JsonObject(); // Return empty object if not found
}
*/

bool JsonConfigHandler::readFileContents() {

	uint32_t bytesread; // bytes read count


    printf("\n1. Reading JSON configuration file\n");

    // Try to mount the file system
    printf("	Mounting the file system... \n");
    if(f_mount(&SDFatFS, (TCHAR const*)SDPath, 0) != FR_OK)
	{
    	printf("	Failed to mount SD card\n\r");
    	Error_Handler();
	}
    else
    {
		//Open file for reading
		if(f_open(&SDFile, filename, FA_READ) != FR_OK)
		{
			printf("	Failed to open JSON config file\n");
			Error_Handler();
		}
		else
		{
			int32_t length = f_size(&SDFile);
			printf("	JSON config file lenght = %2ld\n", length);

			__attribute__((aligned(32))) char rtext[length];
			if(f_read(&SDFile, rtext, length, (UINT *)&bytesread) != FR_OK)
			{
				printf("	JSON config file read FAILURE\n");
			}
			else
			{
				printf("	JSON config file read SUCCESS!\n");
				// put JSON char array into std::string
				jsonContent.reserve(length + 1);
			    for (int i = 0; i < length; i++) {
			    	jsonContent = jsonContent + rtext[i];
			    }

			    // Remove comments from next line to print out the JSON config file
			    printf("\n%s\n", jsonContent.c_str());
			}

			f_close(&SDFile);
		}
    }

	return true;
}


bool JsonConfigHandler::parseJson() {
	
	printf("\n3. Parsing json configuration file\n");
	
    // Clear any existing parsed data
    doc.clear();

    // Parse JSON
    DeserializationError error = deserializeJson(doc, jsonContent.c_str());

    printf("	Config deserialisation - ");

    switch (error.code())
    {
        case DeserializationError::Ok:
            printf("Deserialization succeeded\n");
            break;
        case DeserializationError::InvalidInput:
            printf("Invalid input!\n");
            configError = true;
            break;
        case DeserializationError::NoMemory:
            printf("Not enough memory\n");
            configError = true;
            break;
        default:
            printf("Deserialization failed: ");
            printf(error.c_str());
            printf("\n");
            configError = true;
            break;
    }

    return true;
}
