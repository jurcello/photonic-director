//
//  Light.cpp
//  OpenGlTest
//
//  Created by Jur de Vries on 03/10/2017.
//

#include "Light.h"

using namespace cinder;
using namespace cinder::app;
using namespace photonic;

Light::Light(vec3 cPosition, vec4 cColor, float cIntensity)
: position(vec4(cPosition, 1.0f)), color(cColor), intensity(cIntensity)
{
    mUuid = generate_uuid();
}

Light::Light(vec3 cPosition, vec4 cColor, float cIntensity, std::string uuid)
: position(vec4(cPosition, 1.0f)), color(cColor), intensity(cIntensity), mUuid(uuid)
{
}

void Light::setPosition(vec3 newPosition)
{
    position = vec4(newPosition, 1.0f);
}

vec3 Light::getPosition()
{
    return vec3(position.x, position.y, position.z);
}

void Light::setEffectIntensity(int effectId, float targetIntensity)
{
    effectIntensities[effectId] = targetIntensity;
}

float Light::getEffetcIntensity(int effectId)
{
    return effectIntensities[effectId];
}

void Light::setEffectColor(int effectId, ColorA color)
{
    effectColors[effectId] = color;
}

ColorA Light::getEffectColor(int effectId)
{
    return effectColors[effectId];
}

LightBufferData::LightBufferData(vec3 cPosition, vec4 cColor, float cIntensity)
: position(vec4(cPosition, 1.0f)), color(cColor), intensity(cIntensity)
{
}

LightBufferData::LightBufferData(Light light)
: position(light.position), color(light.color), intensity(light.intensity)
{    
}
