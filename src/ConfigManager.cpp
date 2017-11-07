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
        // Uuid.
        std::string uuid = lightNode.getAttribute("uuid");
        Light* newLight = new Light(position, color, intensity, uuid);
        if (lightNode.hasChild("dmxChannel")) {
            // Dmx channel.
            int dmxChannel = lightNode.getChild("dmxChannel").getValue<int>();
            newLight->setDmxChannel(dmxChannel);
        }
        lights.push_back(newLight);
    }
}

void ConfigManager::readChannels(std::vector<InputChannelRef> &channels)
{
    channels.clear();
    try {
        auto channelNodes= mDoc.getChild("channels");
        for (auto channelNode : channelNodes) {
            std::string name = channelNode.getChild("name").getValue<std::string>();
            std::string address = channelNode.getChild("oscAddress").getValue<std::string>();
            std::string uuid = channelNode.getAttributeValue<std::string>("uuid");
            InputChannelRef newChannel = InputChannel::create(name, address, uuid);
            channels.push_back(newChannel);
        }
    } catch (Exception e){
        // Although the node is not found, we load anyway...
    }
}

void ConfigManager::readEffects(std::vector<EffectRef> &effects, const std::vector<Light *> &lights, const std::vector<InputChannelRef> &channels)
{
    effects.clear();
    if (mDoc.hasChild("effects")) {
        auto effectsNodes = mDoc.getChild("effects");
        for (auto effectNode : effectsNodes) {
            std::string name = effectNode.getChild("name").getValue();
            std::string uuid = effectNode.getAttributeValue<std::string>("uuid");
            // Create the effect.
            EffectRef newEffect = Effect::create(name, uuid);
            if (effectNode.hasChild("channel")) {
                std::string channelUuid = effectNode.getChild("channel").getValue();
                // Get the channel from the uuid.
                auto it = std::find_if(channels.begin(), channels.end(), [&channelUuid](const InputChannelRef &channelCandidate){ return channelCandidate->getUuid() == channelUuid;});
                if (it != channels.end()) {
                    newEffect->setChannel(*it);
                }
            }
            if (effectNode.hasChild("lights")) {
                for (const auto &lightNode : effectNode.getChild("lights").getChildren()) {
                    std::string lightUuid = lightNode->getValue();
                    auto it = std::find_if(lights.begin(), lights.end(), [&lightUuid](const Light* lightCandidate){ return lightCandidate->mUuid == lightUuid;});
                    if (it != lights.end()) {
                        newEffect->addLight(*it);
                    }
                }
            }
            effects.push_back(newEffect);
        }
        
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
        lightNode.setAttribute("uuid", light->mUuid);
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
        
        XmlTree dmxChannel;
        dmxChannel.setTag("dmxChannel");
        dmxChannel.setValue(light->getDmxChannel());
        lightNode.push_back(dmxChannel);
        
        lightsNode.push_back(lightNode);
    }
    mDoc.push_back(lightsNode);
}

void ConfigManager::writeChannels(std::vector<InputChannelRef> &channels) {
    XmlTree channelsNode;
    channelsNode.setTag("channels");
    for (InputChannelRef channel : channels) {
        XmlTree channelNode;
        channelNode.setTag("channel");
        XmlTree nameNode;
        nameNode.setTag("Name");
        nameNode.setValue(channel->getName());
        XmlTree addressNode;
        addressNode.setTag("oscAddress");
        addressNode.setValue(channel->getAddress());
        channelNode.setAttribute("uuid", channel->getUuid());
        channelNode.push_back(nameNode);
        channelNode.push_back(addressNode);
        channelsNode.push_back(channelNode);
    }
    mDoc.push_back(channelsNode);
}

void ConfigManager::writeEffects(std::vector<EffectRef> &effects)
{
    XmlTree effectsNode;
    effectsNode.setTag("effects");
    for (auto effect : effects) {
        XmlTree effectNode;
        effectNode.setTag("effect");
        effectNode.setAttribute("uuid", effect->getUuid());
        XmlTree nameNode;
        nameNode.setTag("name");
        nameNode.setValue(effect->getName());
        effectNode.push_back(nameNode);
        XmlTree channelNode;
        channelNode.setTag("channel");
        channelNode.setValue(effect->getChannel()->getUuid());
        effectNode.push_back(channelNode);
        XmlTree lightsNode;
        lightsNode.setTag("lights");
        for (auto light : effect->getLights()) {
            XmlTree lightNode;
            lightNode.setTag("light");
            lightNode.setValue(light->mUuid);
            lightsNode.push_back(lightNode);
        }
        effectNode.push_back(lightsNode);
        effectsNode.push_back(effectNode);
    }
    mDoc.push_back(effectsNode);
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
