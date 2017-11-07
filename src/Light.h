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
#include "Output.h"

using namespace cinder;

class Light
{
public:
    // The fact that a lot of member variables are public, is due to the reason that I want
    // to experiment with using public variables when no input protection is needed.
    // For creative coding speed is important and the getter and setters might need more work than needed.
    static int initNameNumber;
    ColorA color;
    float intensity;
    std::string mUuid;
    std::string mName;
    vec4 position;
    
    Light(vec3 cPosition, vec4 cColor, float cIntensity);
    Light(vec3 cPosition, vec4 cColor, float cIntensity, std::string uuid);
    
    void setPosition(vec3 newPosition);
    vec3 getPosition();
    
    bool setDmxChannel(int dmxChannel);
    int getDmxChannel();
    
    void injectDmxChecker(DmxOutput* checker);
    
    // Effect setters and getters.
    void setEffectIntensity(std::string effectId, float targetIntensity);
    float getEffetcIntensity(std::string effectId);
    void setEffectColor(std::string effectId, ColorA color);
    ColorA getEffectColor(std::string effectId);
    
    ~Light();
    
protected:
    std::map<std::string, float> effectIntensities;
    std::map<std::string, ColorA> effectColors;
    
    int mDmxChannel;
    DmxOutput* mDmxOutput;

};

// Struct for sending light data to the buffer.
struct LightBufferData
{
    ColorA color;
    vec4 position;
    float intensity;
    vec3 dummy;
  
    LightBufferData(vec3 cPosition, vec4 cColor, float cIntensity);
    LightBufferData(Light* light);
};

#endif /* Light_h */
