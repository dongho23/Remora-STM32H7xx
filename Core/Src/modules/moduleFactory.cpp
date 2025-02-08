
#include "moduleList.h"
#include "moduleFactory.h"


// Create module based on thread and type
std::shared_ptr<Module> ModuleFactory::createModule(const char* _tname,
                                   const char* _mtype,
                                   const JsonVariant config,
                                   Remora* instance) {
    if (strcmp(_tname, "Base") == 0) {
        if (strcmp(_mtype, "Stepgen") == 0) {
            return Stepgen::create(config, instance);
        }
    } else if (strcmp(_tname, "Servo") == 0) {
        if (strcmp(_mtype, "Blink") == 0) {
            return Blink::create(config, instance);
        } else if (strcmp(_mtype, "Reset Pin") == 0) {
            return ResetPin::create(config, instance);
        } else if (strcmp(_mtype, "Digital Pin") == 0) {
            return DigitalPin::create(config, instance);
        } else if (strcmp(_mtype, "Sigma Delta") == 0) {
            return SigmaDelta::create(config, instance);
        }
    } else if (strcmp(_tname, "On load") == 0) {
        // Future on-load modules can follow the same pattern
    } else {
        printf("Error: Unknown thread type '%s' or module type '%s'\n", _tname, _mtype);
    }

    return nullptr;
}


// Static instance accessor
ModuleFactory* ModuleFactory::getInstance() {
    static ModuleFactory* instance = new ModuleFactory();
    return instance;
}
