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
#include "CinderImGui.h"

using namespace cinder;

class Light;
typedef std::shared_ptr<Light> LightRef;

class Light
{
public:
    // The fact that a lot of member variables are public, is due to the reason that I want
    // to experiment with using public variables when no input protection is needed.
    // For creative coding speed is important and the getter and setters might need more work than needed.
    static int initNameNumber;
    // Todo: create getters and setters for these variables.
    ColorA color;
    float intensity;
    int mDmxOffsetIntentsityValue;

    std::string mName;
    vec4 position;
    
    Light(vec3 cPosition, std::string cType, std::string uuid = "");
    
    void setPosition(vec3 newPosition);
    vec3 getPosition();
    std::string getUuid();
    
    bool setDmxChannel(int dmxChannel);
    int getDmxChannel();
    int getMNumChannels() const;
    Light* setNumChannels(int numChannels);
    int getColorChannelPosition() const;
    Light* setColorChannelPosition(int colorChannelPosition);
    int getIntensityChannelPosition() const;
    Light* setIntensityChannelPosition(int intensityChannelPosition);
    int getCorrectedDmxValue();

    void injectDmxOutput(DmxOutput *dmxOutput);
    
    // Effect setters and getters.
    void setEffectIntensity(std::string effectId, float targetIntensity);
    float getEffetcIntensity(std::string effectId);
    void setEffectColor(std::string effectId, ColorA color);
    ColorA getEffectColor(std::string effectId);
    
    ~Light();
    
protected:
    std::map<std::string, float> effectIntensities;
    std::map<std::string, ColorA> effectColors;

    std::string mUuid;
    std::string mType;
    int mDmxChannel;
    int mNumChannels;
    int mColorChannelPosition;
    int mIntensityChannelPosition;
    DmxOutput* mDmxOutput;
};

class LightFactory {
public:
    explicit LightFactory(DmxOutput* dmxOutput);

    std::vector<std::string> getAvailableTypes();
    std::string getDefaultType();

    LightRef create(vec3 position, std::string type = "", std::string uuid = "", int numDmxChannels = 1, int colorChannelPosition = 0, int intensityChannelPosition = 0);

protected:
    DmxOutput* mDmxOut;
};

// Struct for sending light data to the buffer.
struct LightBufferData
{
    ColorA color;
    vec4 position;
    float intensity;
    vec3 dummy;
  
    LightBufferData(vec3 cPosition, vec4 cColor, float cIntensity);
    explicit LightBufferData(LightRef light);
};

#endif /* Light_h */
