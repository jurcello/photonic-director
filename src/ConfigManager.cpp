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

void ConfigManager::readLights(std::vector<LightRef> &lights, LightFactory* lightFactory)
{
    // Overwrite the current lights.
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
        LightRef newLight;
        if (lightNode.hasAttribute("type")) { ;
            newLight = lightFactory->create(position, lightNode.getAttribute("type"), uuid);
        }
        else {
            newLight = lightFactory->create(position, NULL, uuid);
        }
        if (lightNode.hasChild("dmxChannel")) {
            // Dmx channel.
            int dmxChannel = lightNode.getChild("dmxChannel").getValue<int>();
            newLight->setDmxChannel(dmxChannel);
        }
        if (lightNode.hasChild("name")) {
            std::string name = lightNode.getChild("name").getValue<std::string>();
            newLight->mName = name;
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

void ConfigManager::readEffects(std::vector<EffectRef> &effects, const std::vector<LightRef> &lights, std::vector<InputChannelRef> &channels)
{
    effects.clear();
    if (mDoc.hasChild("effects")) {
        auto effectsNodes = mDoc.getChild("effects");
        for (auto effectNode : effectsNodes) {
            std::string type = effectNode.getChild("type").getValue();
            std::string name = effectNode.getChild("name").getValue();
            std::string uuid = effectNode.getAttributeValue<std::string>("uuid");
            // Create the effect.
            EffectRef newEffect = Effect::create(type, name, uuid);
            if (effectNode.hasChild("fadetime")) {
                float fadeTime = effectNode.getChild("fadetime").getValue<float>();
                newEffect->fadeTime = fadeTime;
            }
            if (effectNode.hasChild("weight")) {
                float weight = effectNode.getChild("weight").getValue<float>();
                newEffect->weight = weight;
            }
            if (effectNode.hasChild("isActive")) {
                bool isActive = effectNode.getChild("isActive").getValue<bool>();
                newEffect->isTurnedOn = isActive;
            }
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
                    auto it = std::find_if(lights.begin(), lights.end(), [&lightUuid](const LightRef lightCandidate){ return lightCandidate->getUuid() == lightUuid;});
                    if (it != lights.end()) {
                        newEffect->addLight(*it);
                    }
                }
            }
            if (effectNode.hasChild("params")) {
                for (auto &paramNode: effectNode.getChild("params").getChildren()) {
                    int index = paramNode->getAttributeValue<int>("index");
                    Parameter* param = newEffect->getParam(index);
                    readParam(paramNode, param, channels);
                    
                }
            }
            effects.push_back(newEffect);
        }
        
    }
}

void ConfigManager::readParam(std::unique_ptr<XmlTree> &paramNode, Parameter *param, std::vector<InputChannelRef> &channels)
{
    // We only need the type and the value
    Parameter::Type type = (Parameter::Type) paramNode->getAttributeValue<int>("type");
    switch (type) {
        case photonic::Parameter::kType_Float:
            param->floatValue = paramNode->getValue<float>();
            break;
            
        case photonic::Parameter::kType_Int:
            param->intValue = paramNode->getValue<int>();
            break;
            
        case photonic::Parameter::kType_Color:
        {
            float r = paramNode->getAttributeValue<float>("r");
            float g = paramNode->getAttributeValue<float>("g");
            float b = paramNode->getAttributeValue<float>("b");
            float a = paramNode->getAttributeValue<float>("a");
            param->colorValue = ColorA(r, g, b, a);
            break;
        }

        case photonic::Parameter::kType_Vector3:
        {
            float x = paramNode->getAttributeValue<float>("x");
            float y = paramNode->getAttributeValue<float>("y");
            float z = paramNode->getAttributeValue<float>("z");
            param->vec3Value = vec3(x, y, z);
            break;
        }

        case photonic::Parameter::kType_Channel:
        {
            std::string channelUuid = paramNode->getValue();
            auto it = std::find_if(channels.begin(), channels.end(),
                                   [&channelUuid](const InputChannelRef &channelCandidate) {
                                       return channelCandidate->getUuid() == channelUuid;
                                   });
            if (it != channels.end()) {
                param->channelRef = *it;
            }
        }
            break;

        default:
            break;
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

void ConfigManager::writeLights(std::vector<LightRef> &lights)
{
    XmlTree lightsNode;
    lightsNode.setTag("lights");
    for (LightRef light : lights) {
        XmlTree lightNode;
        lightNode.setTag("light");
        lightNode.setAttribute("uuid", light->getUuid());
        lightNode.setAttribute("type", light->getLightType()->machineName);
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
        
        XmlTree name;
        name.setTag("name");
        name.setValue(light->mName);
        lightNode.push_back(name);
        
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
        XmlTree typeNode;
        typeNode.setTag("type");
        typeNode.setValue(effect->getTypeClassName());
        effectNode.push_back(typeNode);
        XmlTree nameNode;
        nameNode.setTag("name");
        nameNode.setValue(effect->getName());
        effectNode.push_back(nameNode);
        XmlTree fadeTimeNode;
        fadeTimeNode.setTag("fadetime");
        fadeTimeNode.setValue(effect->fadeTime);
        XmlTree weightNode;
        weightNode.setTag("weight");
        weightNode.setValue(effect->weight);
        effectNode.push_back(weightNode);
        XmlTree channelNode;
        XmlTree activeNode;
        activeNode.setTag("isActive");
        activeNode.setValue(effect->isTurnedOn);
        effectNode.push_back(activeNode);
        channelNode.setTag("channel");
        if (effect->getChannel()) {
            channelNode.setValue(effect->getChannel()->getUuid());
        }
        effectNode.push_back(channelNode);
        XmlTree lightsNode;
        lightsNode.setTag("lights");
        for (auto light : effect->getLights()) {
            XmlTree lightNode;
            lightNode.setTag("light");
            lightNode.setValue(light->getUuid());
            lightsNode.push_back(lightNode);
        }
        XmlTree paramsNode;
        paramsNode.setTag("params");
        for (auto &param: effect->getParams()) {
            writeParameter(paramsNode, param.second, param.first);
        }
        effectNode.push_back(paramsNode);
        effectNode.push_back(lightsNode);
        effectsNode.push_back(effectNode);
    }
    mDoc.push_back(effectsNode);
}

void ConfigManager::writeParameter(cinder::XmlTree &paramsNode, photonic::Parameter *param, int index)
{
    XmlTree paramnode;
    paramnode.setTag("param");
    paramnode.setAttribute("type", param->type);
    paramnode.setAttribute("description", param->description);
    paramnode.setAttribute("index", index);
    switch (param->type) {
        case photonic::Parameter::kType_Float:
            paramnode.setValue(param->floatValue);
            break;

        case photonic::Parameter::kType_Int:
            paramnode.setValue(param->intValue);
            break;
            
        case photonic::Parameter::kType_Color:
            paramnode.setAttribute("r", param->colorValue.r);
            paramnode.setAttribute("g", param->colorValue.g);
            paramnode.setAttribute("b", param->colorValue.b);
            paramnode.setAttribute("a", param->colorValue.a);
            break;

        case photonic::Parameter::kType_Vector3:
            paramnode.setAttribute("x", param->vec3Value.x);
            paramnode.setAttribute("y", param->vec3Value.y);
            paramnode.setAttribute("z", param->vec3Value.z);
            break;

        case photonic::Parameter::kType_Channel:
            if (param->channelRef != nullptr) {
                paramnode.setValue(param->channelRef->getUuid());
            }
            break;

        default:
            break;
    }
    paramsNode.push_back(paramnode);
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
