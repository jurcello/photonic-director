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
    ConfigManager();
    void readFromFile(fs::path path);
    void readLights(std::vector<Light*> &lights);
    int readInt(std::string name);
    
    void startNewDoc();
    void writeLights(std::vector<Light*> &lights);
    void writeInt(std::string name, int value);
    void writeToFile(fs::path path);
    
protected:
    XmlTree mDoc;
};

#endif /* ConfigManager_h */
