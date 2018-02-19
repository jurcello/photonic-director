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
#include "Output.h"

using namespace photonic;

class ConfigManager {
public:
    ConfigManager();
    void readFromFile(fs::path path);
    void readLights(std::vector<LightRef> &lights, LightFactory* lightFactory);
    void readChannels(std::vector<InputChannelRef> &channels);
    void readEffects(std::vector<EffectRef> &effects, const std::vector<LightRef> &lights, std::vector<InputChannelRef> &channels);
    int readInt(std::string name);
    void readParam(std::unique_ptr<XmlTree> &paramNode, Parameter *param, std::vector<InputChannelRef> &channels);
    
    void startNewDoc();
    void writeLights(std::vector<LightRef> &lights);
    void writeChannels(std::vector<InputChannelRef> &channels);
    void writeEffects(std::vector<EffectRef> &effects);
    void writeInt(std::string name, int value);
    void writeToFile(fs::path path);
    
    void writeParameter(XmlTree &paramsNode, Parameter* paramm, int index);
    
protected:
    XmlTree mDoc;
};

#endif /* ConfigManager_h */
