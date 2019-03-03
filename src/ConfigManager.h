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
#include "Scene.h"

using namespace photonic;

class ConfigManager {
public:
    ConfigManager();
    void readFromFile(fs::path path);
    void readLights(std::vector<LightRef> &lights, LightFactory* lightFactory);
    void readChannels(std::vector<InputChannelRef> &channels);
    void readEffects(std::list<EffectRef> &effects, std::vector<LightRef> &lights, std::vector<InputChannelRef> &channels);
    void readScenes(SceneListRef &sceneList, std::list<EffectRef> &effects);
    template <class T>
    T readValue(std::string name, T defaultValue) {
        if (mDoc.hasChild(name)) {
            XmlTree node = mDoc.getChild(name);
            return node.getValue<T>();
        }
        return defaultValue;
    }
    void readParam(std::unique_ptr<XmlTree> &paramNode, Parameter *param, std::vector<InputChannelRef> &channels, std::vector<LightRef> &lights);
    
    void startNewDoc();
    void writeLights(std::vector<LightRef> &lights);
    void writeChannels(std::vector<InputChannelRef> &channels);
    void writeEffects(std::list<EffectRef> &effects);
    void writeScenes(std::list<SceneRef> &scenes);
    template <class T>
    void writeValue(std::string name, T value) {
        XmlTree valueNode;
        valueNode.setTag(name);
        valueNode.setValue(value);
        mDoc.push_back(valueNode);
    };
    void writeToFile(fs::path path);
    
    void writeParameter(XmlTree &paramsNode, Parameter* paramm, int index);
    
protected:
    XmlTree mDoc;
};

#endif /* ConfigManager_h */
