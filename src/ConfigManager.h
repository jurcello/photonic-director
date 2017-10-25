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
#include "Effects.h"

class ConfigManager {
public:
    ConfigManager();
    void readFromFile(fs::path path);
    void readLights(std::vector<Light*> &lights);
    void readChannels(std::vector<InputChannelRef> &channels);
    int readInt(std::string name);
    
    void startNewDoc();
    void writeLights(std::vector<Light*> &lights);
    void writeChannels(std::vector<InputChannelRef> &channels);
    void writeInt(std::string name, int value);
    void writeToFile(fs::path path);
    
protected:
    XmlTree mDoc;
};

#endif /* ConfigManager_h */
