
#include "moduleList.h"
#include "moduleFactory.h"


// Create module based on thread and type
std::shared_ptr<Module> ModuleFactory::createModule(const char* _tname,
                                   const char* _mtype,
                                   const JsonVariant config,
								   Remora* instance) {
    if (strcmp(_tname, "Base") == 0) {
        if (strcmp(_mtype, "Stepgen") == 0)
            return Stepgen::create(config, instance);
    }
    if (strcmp(_tname, "Servo") == 0) {
        if (strcmp(_mtype, "Blink") == 0)
            return Blink::create(config, instance);
        if (strcmp(_mtype, "Reset Pin") == 0)
                    return ResetPin::create(config, instance);
    }
    if (strcmp(_tname, "On load") == 0) {
        // Future on-load modules can follow the same pattern
    }

    return nullptr;
}

// Static instance accessor
ModuleFactory* ModuleFactory::getInstance() {
    static ModuleFactory* instance = new ModuleFactory();
    return instance;
}
