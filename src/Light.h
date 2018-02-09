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

struct LightType {
    enum RgbType {
        RGB,
        RBG,
    };
    LightType(const std::string &name, std::string machineName, int colorChannelPosition, int intensityChannelPosition,
                  int numChannels, ColorA editColor, RgbType rgbType = RGB);

    std::string name;
    std::string machineName;
    int numChannels;
    int colorChannelPosition;
    int intensityChannelPosition;
    ColorA editColor;
    RgbType rgbType;
};

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
    
    Light(vec3 cPosition, LightType *cType, std::string uuid = "");
    
    void setPosition(vec3 newPosition);
    vec3 getPosition();
    std::string getUuid();
    LightType* getLightType();
    
    bool setDmxChannel(int dmxChannel);
    int getDmxChannel();
    int getNumChannels() const;
    int getColorChannelPosition() const;
    int getIntensityChannelPosition() const;
    int getCorrectedDmxValue();

    void injectDmxOutput(DmxOutput *dmxOutput);
    void updateDmx();
    
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
    LightType* mType;
    int mDmxChannel;
    int mNumChannels;
    int mColorChannelPosition;
    int mIntensityChannelPosition;
    DmxOutput* mDmxOutput;
};

class LightFactory {
public:
    explicit LightFactory(DmxOutput* dmxOutput);

    std::vector<std::string> getAvailableTypeNames();
    std::vector<LightType*> getAvailableTypes();
    LightType* getDefaultType();

    LightRef create(vec3 position, LightType *type, std::string uuid);
    LightRef create(vec3 position, std::string type, std::string uuid);

    virtual ~LightFactory();

protected:
    DmxOutput* mDmxOut;
    std::vector<LightType*> mLightTypes;

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
