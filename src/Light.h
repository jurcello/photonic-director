//
//  Light.hpp
//  OpenGlTest
//
//  Created by Jur de Vries on 03/10/2017.
//

#ifndef Light_h
#define Light_h

#include <stdio.h>
#include "cinder/gl/gl.h"
#include "Utils.h"

using namespace cinder;

struct Light
{
    // This needs to be aligned so we have 2 blocks of 16 bites.
    ColorA color;
    vec4 position;
    float intensity;
    std::string mUuid;
    
    std::map<int, float> effectIntensities;
    std::map<int, ColorA> effectColors;
    
    Light(vec3 cPosition, vec4 cColor, float cIntensity);
    Light(vec3 cPosition, vec4 cColor, float cIntensity, std::string uuid);
    
    void setPosition(vec3 newPosition);
    vec3 getPosition();
    
    // Effect setters and getters.
    void setEffectIntensity(int effectId, float targetIntensity);
    float getEffetcIntensity(int effectId);
    void setEffectColor(int effectId, ColorA color);
    ColorA getEffectColor(int effectId);
    
    
};

// Struct for sending light data to the buffer.
struct LightBufferData
{
    ColorA color;
    vec4 position;
    float intensity;
    vec3 dummy;
  
    LightBufferData(vec3 cPosition, vec4 cColor, float cIntensity);
    LightBufferData(Light light);
};

#endif /* Light_h */
