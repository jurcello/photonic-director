//
//  ConfigManager.hpp
//  OpenGlTest
//
//  Created by Jur de Vries on 14/10/2017.
//

#ifndef ConfigManager_h
#define ConfigManager_h

#include <stdio.h>
#include "cinder/Xml.h"
#include "cinder/Utilities.h"
#include "Light.h"

class ConfigManager {
public:
    void readLights(fs::path path, std::vector<Light*> &lights);
    void writeLights(fs::path path, std::vector<Light*> &lights);
};

#endif /* ConfigManager_h */
