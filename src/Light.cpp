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

Light::Light(vec3 cPosition, vec4 cColor, float cIntensity)
: position(vec4(cPosition, 1.0f)), color(cColor), intensity(cIntensity), mDmxChannel(0), mDmxOutput(nullptr), mDmxOffsetIntentsityValue(0)
{
    mUuid = generate_uuid();
    mName = "Lamp " + std::to_string(++initNameNumber);
}

Light::Light(vec3 cPosition, vec4 cColor, float cIntensity, std::string uuid)
: position(vec4(cPosition, 1.0f)), color(cColor), intensity(cIntensity), mUuid(uuid), mDmxChannel(0), mDmxOutput(nullptr), mDmxOffsetIntentsityValue(0)
{
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

int Light::getCorrectedDmxValue()
{
    return (mDmxOffsetIntentsityValue + (256 - mDmxOffsetIntentsityValue) * intensity);
}

void Light::injectDmxChecker(DmxOutput *checker)
{
    mDmxOutput = checker;
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

LightBufferData::LightBufferData(vec3 cPosition, vec4 cColor, float cIntensity)
: position(vec4(cPosition, 1.0f)), color(cColor), intensity(cIntensity)
{
}

LightBufferData::LightBufferData(Light* light)
: position(light->position), color(light->color), intensity(light->intensity)
{    
}
