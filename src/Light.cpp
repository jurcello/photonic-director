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
                     int numChannels, ColorA editColor)
        : name(name), machineName(machineName), numChannels(numChannels), colorChannelPosition(colorChannelPosition),
          intensityChannelPosition(intensityChannelPosition), editColor(editColor) {}


Light::Light(vec3 cPosition, LightType *cType, std::string uuid)
: position(vec4(cPosition, 1.0f)), mType(cType), color(Color::white()), intensity(0.0), mDmxChannel(0), mDmxOutput(nullptr), mDmxOffsetIntentsityValue(0)
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
    if (mDmxOutput) {
        if (!mDmxOutput->checkRangeAvailable(dmxChannel, mType->numChannels, mUuid)) {
            return false;
        }
        mDmxOutput->releaseChannels(mUuid);
        for (int i = dmxChannel; i < dmxChannel + mType->numChannels; i++) {
            mDmxOutput->registerChannel(i, mUuid);
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
    mDmxOutput->releaseChannels(mUuid);
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

void Light::updateDmx() {
    if (mDmxChannel > 0) {
        // NOTE THAT FOR ALL CHANNEL POSITIONS WE USE HUMAN READABLE NUMBERS.
        // I.E. COUNTING STARTS AT ONE.
        // If there is a color channel, update that channel.
        if (getColorChannelPosition() > 0) {
            int redChannel = mDmxChannel + getColorChannelPosition() - 1;
            int greenChannel = redChannel + 1;
            int blueChannel = redChannel + 2;
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
            mDmxOutput->setChannelValue(channel, 255);
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
////// Factory
/////////////////////////////////////////////////////
LightFactory::LightFactory(DmxOutput *dmxOutput)
: mDmxOut(dmxOutput)
{
    // Create some types.
    // TODO: create them form a file later on.
    mLightTypes.push_back(new LightType("Single Channel (dimmer)", "single_channel", 0, 0, 1, cinder::ColorA(252.0f/256.0f, 211.0f/256.0f, 3.0f/256.0f, 0)));
    mLightTypes.push_back(new LightType("Simple Color (3 channels)", "simple_color", 1, 0, 3, cinder::ColorA(1.0f, 0, 0, 0)));
    mLightTypes.push_back(new LightType("Advanced Color (6 channels)", "advanced_color", 1, 4, 6, cinder::ColorA(1.0f, 0, 1.0f, 0)));
}



LightRef LightFactory::create(vec3 position, LightType *type, std::string uuid)
{
    if (type == nullptr) {
        type = getDefaultType();
    }
    LightRef light = LightRef(new Light(position, type, uuid));
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
    LightRef light = LightRef(new Light(position, newType, uuid));
    light->injectDmxOutput(mDmxOut);
    return light;
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
