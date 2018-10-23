//
//  Light.cpp
//  OpenGlTest
//
//  Created by Jur de Vries on 03/10/2017.
//

#include "Light.h"
#include "Utils.h"

using namespace cinder;
using namespace cinder::app;
using namespace photonic;

int Light::initNameNumber = 0;


LightType::LightType(const std::string &name, std::string machineName, int colorChannelPosition, int intensityChannelPosition,
                     int numChannels, ColorA editColor, RgbType rgbType)
        : name(name), machineName(machineName), numChannels(numChannels), colorChannelPosition(colorChannelPosition),
          intensityChannelPosition(intensityChannelPosition), editColor(editColor), rgbType(rgbType) {}


Light::Light(vec3 cPosition, LightType *cType, std::string uuid)
: position(vec4(cPosition, 1.0f)),
  mType(cType),
  color(Color::white()),
  intensity(0.0),
  mDmxChannel(0),
  mDmxOutput(nullptr),
  mDmxOffsetIntentsityValue(0),
  mSendOsc(false),
  mOscAdress("")
{
    if (uuid.empty()) {
        mUuid = generate_uuid();
    }
    else {
        mUuid = uuid;
    }
    mName = "Lamp " + std::to_string(++initNameNumber);
}

void Light::setPosition(vec3 newPosition)
{
    position = vec4(newPosition, 1.0f);
}

vec3 Light::getPosition()
{
    return vec3(position.x, position.y, position.z);
}

std::string Light::getUuid() {
    return mUuid;
}

LightType *Light::getLightType() {
    return mType;
}

bool Light::setDmxChannel(int dmxChannel)
{
    // If there is a checker defined, check it here.
    if (mDmxOutput && dmxChannel > 0) {
        if (!mDmxOutput->checkRangeAvailable(dmxChannel, mType->numChannels, mUuid)) {
            return false;
        }
        mDmxOutput->releaseChannels(mUuid);
        for (int i = dmxChannel; i < dmxChannel + mType->numChannels; i++) {
            mDmxOutput->registerChannel(i, mUuid);
            for (auto component : mComponents) {
                component->setFixtureChannel(dmxChannel);
            }
        }
    }
    mDmxChannel = dmxChannel;
    return true;
}

int Light::getDmxChannel()
{
    return mDmxChannel;
}

void Light::injectDmxOutput(DmxOutput *dmxOutput)
{
    mDmxOutput = dmxOutput;
}

void Light::setEffectIntensity(std::string effectId, float targetIntensity)
{
    effectIntensities[effectId] = targetIntensity;
}

float Light::getEffetcIntensity(std::string effectId)
{
    return effectIntensities[effectId];
}

void Light::setEffectColor(std::string effectId, ColorA color)
{
    effectColors[effectId] = color;
}

Light::~Light()
{
    if (mDmxChannel > 0) {
        mDmxOutput->releaseChannels(mUuid);
    }
}

ColorA Light::getEffectColor(std::string effectId)
{
    return effectColors[effectId];
}

int Light::getNumChannels() const {
    return mType->numChannels;
}

int Light::getColorChannelPosition() const {
    return mType->colorChannelPosition;
}

int Light::getIntensityChannelPosition() const {
    return mType->intensityChannelPosition;
}

int Light::getCorrectedDmxValue() {
    return (int) roundf(mDmxOffsetIntentsityValue + (256 - mDmxOffsetIntentsityValue) * intensity);
}

bool Light::isColorEnabled() {
    return mType->colorChannelPosition > 0;
}


void Light::addComponent(LightComponentRef component) {
    mComponents.push_back(component);
}

LightComponentRef Light::getComponentById(std::string id) {
    for (const auto &component : mComponents) {
        if (component->id == id) {
            return component;
        }
    }
    return nullptr;
}

std::vector<LightComponentRef> Light::getComponents() {
    return mComponents;
}

void Light::update() {
    if (mType->machineName == "relais") {
        intensity = intensity > 0.5f ? 1.0f : 0.0f;
    }
}

void Light::updateDmx() {
    if (mDmxChannel > 0) {
        // NOTE THAT FOR ALL CHANNEL POSITIONS WE USE HUMAN READABLE NUMBERS.
        // I.E. COUNTING STARTS AT ONE.
        // If there is a color channel, update that channel.
        if (getColorChannelPosition() > 0) {
            int redChannel = mDmxChannel + getColorChannelPosition() - 1;
            int greenChannel = redChannel + 1;
            int blueChannel = redChannel + 2;
            // For some strange reason some lights have blue before green.
            if (mType->rgbType == LightType::RgbType::RBG) {
                greenChannel = redChannel + 2;
                blueChannel = redChannel + 1;
            }
            else if (mType->rgbType == LightType::RgbType::GRB) {
                greenChannel = mDmxChannel + getColorChannelPosition() - 1;
                redChannel = greenChannel + 1;
                blueChannel = greenChannel + 2;
            }
            else if (mType->rgbType == LightType::RgbType::BRG) {
                blueChannel = mDmxChannel + getColorChannelPosition() - 1;
                redChannel = blueChannel + 1;
                greenChannel = blueChannel + 2;
            }
            mDmxOutput->setChannelValue(redChannel, color.r);
            mDmxOutput->setChannelValue(greenChannel, color.g);
            mDmxOutput->setChannelValue(blueChannel, color.b);
        }
        else {
            // If there are no color channels, just set the proper DMX channel.
            mDmxOutput->setChannelValue(mDmxChannel, this->getCorrectedDmxValue());
        }
        // If there is an intensity channel, set that one to the max.
        if (getIntensityChannelPosition() > 0) {
            int channel = mDmxChannel + getIntensityChannelPosition() - 1;
            int lightIntensity = intensity * 255;
            mDmxOutput->setChannelValue(channel, lightIntensity);
        }

        // Let the components do their work.
        for (const auto component : mComponents) {
            component->updateDmx(mDmxOutput);
        }
    }

}

LightBufferData::LightBufferData(vec3 cPosition, vec4 cColor, float cIntensity)
: position(vec4(cPosition, 1.0f)), color(cColor), intensity(cIntensity)
{
}

LightBufferData::LightBufferData(LightRef light)
: position(light->position), color(light->color), intensity(light->intensity)
{    
}

/////////////////////////////////////////////////////
////// Light control
/////////////////////////////////////////////////////
LightControl::LightControl(LightRef cLight, ColorA cColor, float cIntensity)
: light(cLight), color(cColor), intensity(cIntensity)
{
}

/////////////////////////////////////////////////////
////// Factory
/////////////////////////////////////////////////////
LightFactory::LightFactory(DmxOutput *dmxOutput)
: mDmxOut(dmxOutput)
{
   readFixtures();
}

void LightFactory::readFixtures() {
    // Scan the fixtures folder in the assets path.
    fs::path fixturesDir = getAssetPath("fixtures");
    if (fs::is_directory(fixturesDir)) {
        for (fs::directory_entry file: fs::directory_iterator(fixturesDir)) {
            if (file.path().extension().string() == ".xml") {
                auto definition = XmlTree(loadFile(file.path()));
                XmlTree definitionInfo = definition.getChild("fixtureDefinition");
                std::string name = definitionInfo.getChild("name").getValue<std::string>();
                std::string id = definitionInfo.getChild("id").getValue<std::string>();
                int colorChannelPosition = definitionInfo.getChild("colorChannelPosition").getAttributeValue<int>("value");
                int intensityChannelPostion = definitionInfo.getChild("intensityChannelPostion").getAttributeValue<int>("value");
                int channelAmount = definitionInfo.getChild("channelAmount").getAttributeValue<int>("value");
                ColorA color(1.0f, 1.0f, 1.0f, 0.0f);
                color.r = definitionInfo.getChild("editColor").getAttributeValue<float>("r");
                color.g = definitionInfo.getChild("editColor").getAttributeValue<float>("g");
                color.b = definitionInfo.getChild("editColor").getAttributeValue<float>("b");
                LightType *lightType = new LightType(
                                        name,
                                        id,
                                        colorChannelPosition,
                                        intensityChannelPostion,
                                        channelAmount,
                                        color
                                );
                if (definitionInfo.hasChild("colorType")) {
                    std::string colorType = definitionInfo.getChild("colorType").getValue<std::string>();
                    if (colorType == "RBG") {
                        lightType->rgbType = LightType::RgbType::RBG;
                    }
                }
                if (definitionInfo.hasChild("components")) {
                    for (auto componentInfo : definitionInfo.getChild("components")) {
                        LightComponentDefintion componentDefinition;
                        componentDefinition.componentChannel = componentInfo.getAttributeValue<int>("channel");
                        componentDefinition.type = componentInfo.getAttributeValue<std::string>("type");
                        componentDefinition.name = componentInfo.getAttributeValue<std::string>("name");
                        componentDefinition.id = componentInfo.getAttributeValue<std::string>("id");
                        if (componentInfo.hasChild("commands")) {
                            for (auto commandInfo : componentInfo.getChild("commands")) {
                                std::string name = commandInfo.getAttributeValue<std::string>("name");
                                int value = 0;
                                if (commandInfo.hasAttribute("value")) {
                                    value = commandInfo.getAttributeValue<int>("value");
                                }
                                else if (commandInfo.hasAttribute("min")) {
                                    value = commandInfo.getAttributeValue<int>("min");
                                }
                                Command command;
                                command.min = value;
                                command.name = name;
                                if (commandInfo.hasAttribute("max")) {
                                    command.max = commandInfo.getAttributeValue<int>("max");
                                }
                                else {
                                    command.max = value;
                                }

                                componentDefinition.commands.insert(std::pair<std::string, Command>(name, command));
                            }
                        }
                        lightType->componentDefitions.push_back(componentDefinition);
                    }
                }
                mLightTypes.push_back(lightType);
            }
        }
    }
}

LightRef LightFactory::create(vec3 position, LightType *type, std::string uuid)
{
    if (type == nullptr) {
        type = getDefaultType();
    }
    LightRef light = LightRef(new Light(position, type, uuid));
    for (auto componentDefinition : light->getLightType()->componentDefitions) {
        light->addComponent(LightComponent::create(componentDefinition, 0));
    }
    light->injectDmxOutput(mDmxOut);
    return light;
}

LightRef LightFactory::create(vec3 position, std::string type, std::string uuid) {
    // A very inefficient way of finding the type, but for now it will do.
    LightType* newType = getDefaultType();
    for (auto lightType: mLightTypes) {
        if (lightType->machineName == type) {
            newType = lightType;
            break;
        }
    }
    return LightFactory::create(position, newType, uuid);
}

std::vector<std::string> LightFactory::getAvailableTypeNames() {
    std::vector<std::string> types;
    for (auto type : mLightTypes) {
        types.push_back(type->name);
    }
    return types;
}

std::vector<LightType *> LightFactory::getAvailableTypes() {
    return mLightTypes;
}

LightType* LightFactory::getDefaultType() {
    return mLightTypes[0];
}

LightFactory::~LightFactory() {
    for (auto type : mLightTypes) {
        delete type;
    }
}
