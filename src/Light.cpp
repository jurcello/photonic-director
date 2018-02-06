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

Light::Light(vec3 cPosition, std::string cType, std::string uuid)
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

bool Light::setDmxChannel(int dmxChannel)
{
    // If there is a checker defined, check it here.
    if (mDmxOutput) {
        mDmxOutput->releaseChannels(mUuid);
        if (mDmxOutput->registerChannel(dmxChannel, mUuid)) {
            mDmxChannel = dmxChannel;
            return true;
        }
        return false;
    }
    else {
        mDmxChannel = dmxChannel;
    }
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

int Light::getMNumChannels() const {
    return mNumChannels;
}

Light* Light::setNumChannels(int numChannels) {
    numChannels = numChannels;
    return this;
}

int Light::getColorChannelPosition() const {
    return mColorChannelPosition;
}

Light* Light::setColorChannelPosition(int colorChannelPosition) {
    mColorChannelPosition = colorChannelPosition;
    return this;
}

int Light::getIntensityChannelPosition() const {
    return mIntensityChannelPosition;
}

Light* Light::setIntensityChannelPosition(int intensityChannelPosition) {
    mIntensityChannelPosition = intensityChannelPosition;
    return this;
}

int Light::getCorrectedDmxValue() {
    return (int) roundf(mDmxOffsetIntentsityValue + (256 - mDmxOffsetIntentsityValue) * intensity);
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
}



LightRef LightFactory::create(vec3 position,
                              std::string type,
                              std::string uuid,
                              int numDmxChannels,
                              int colorChannelPosition,
                              int intensityChannelPosition)
{
    if (type.empty()) {
        type = getDefaultType();
    }
    LightRef light = LightRef(new Light(position, type, uuid));
    light->setColorChannelPosition(colorChannelPosition)
            ->setNumChannels(numDmxChannels)
            ->setNumChannels(intensityChannelPosition)
            ->injectDmxOutput(mDmxOut);
    return light;
}

std::vector<std::string> LightFactory::getAvailableTypes() {
    return std::vector<std::string>();
}

std::string LightFactory::getDefaultType() {
    return "SimpleColor";
}
