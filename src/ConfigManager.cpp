//
//  ConfigManager.cpp
//  OpenGlTest
//
//  Created by Jur de Vries on 14/10/2017.
//

#include "ConfigManager.h"

ConfigManager::ConfigManager()
{
    
}

void ConfigManager::readFromFile(fs::path path)
{
    try {
        mDoc = XmlTree(loadFile(path));
    } catch (Exception e) {
        ci::app::console() << "ERROR loading file " << toString(e.what()) << std::endl;
    }

}

void ConfigManager::readLights(std::vector<Light *> &lights)
{
    // Overwrite the current lights.
    if (lights.size() > 0) {
        for (Light* light : lights) {
            delete light;
        }
    }
    lights.clear();
    auto lightNodes = mDoc.getChild("lights");
    for (auto lightNode : lightNodes) {
        // Color.
        ColorA color;
        XmlTree colorNode = lightNode.getChild("color");
        color.r = colorNode.getAttributeValue<float>("r");
        color.g = colorNode.getAttributeValue<float>("g");
        color.b = colorNode.getAttributeValue<float>("b");
        color.a = colorNode.getAttributeValue<float>("a");
        // Position.
        vec3 position;
        position.x = lightNode.getChild("position").getAttributeValue<float>("x");
        position.y = lightNode.getChild("position").getAttributeValue<float>("y");
        position.z = lightNode.getChild("position").getAttributeValue<float>("z");
        // Intensity.
        float intensity = lightNode.getChild("intensity").getValue<float>();
        Light* newLight = new Light(position, color, intensity);
        lights.push_back(newLight);
    }
}

int ConfigManager::readInt(std::string name)
{
    if (mDoc.hasChild(name)) {
        XmlTree node = mDoc.getChild(name);
        return node.getValue<int>();
    }
    // TODO: better handling of absence of nodes.
    return 0;
}

void ConfigManager::startNewDoc()
{
    std::string beginXmlString("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
    mDoc = XmlTree(beginXmlString);
}

void ConfigManager::writeLights(std::vector<Light *> &lights)
{
    XmlTree lightsNode;
    lightsNode.setTag("lights");
    for (Light* light : lights) {
        XmlTree lightNode;
        lightNode.setTag("light");
        XmlTree position;
        position.setTag("position");
        position.setAttribute("x", light->position.x);
        position.setAttribute("y", light->position.y);
        position.setAttribute("z", light->position.z);
        position.setAttribute("w", light->position.w);
        lightNode.push_back(position);
        
        XmlTree color;
        color.setTag("color");
        color.setAttribute("r", light->color.r);
        color.setAttribute("g", light->color.g);
        color.setAttribute("b", light->color.b);
        color.setAttribute("a", light->color.a);
        lightNode.push_back(color);
        
        XmlTree intensity;
        intensity.setTag("intensity");
        intensity.setValue(light->intensity);
        lightNode.push_back(intensity);
        lightsNode.push_back(lightNode);
    }
    mDoc.push_back(lightsNode);
}

void ConfigManager::writeInt(std::string name, int value)
{
    XmlTree valueNode;
    valueNode.setTag(name);
    valueNode.setValue(value);
    mDoc.push_back(valueNode);
}

void ConfigManager::writeToFile(fs::path path)
{
    mDoc.write(writeFile(path));
}
