//
//  Effects.cpp
//  PhotonicDirector
//
//  Created by Jur de Vries on 24/10/2017.
//

#include "Effects.h"
#include "Utils.h"
#include "CinderImGui.h"

using namespace cinder::app;
using namespace ph;

//////////////////////////////////////////////////////////////////
/// Start channels.
//////////////////////////////////////////////////////////////////

InputChannelRef InputChannel::create(std::string name, std::string address, std::string uuid)
{
    return InputChannelRef(new InputChannel(name, address, uuid));
}

InputChannel::InputChannel(std::string name, std::string address, std::string uuid)
: mName(name), mAddress(address), mValue(0.f), mVec2Value(vec2(0.f)), mVec3Value(vec3(0.f)), mUuid(uuid), mIntValue(0), mSmoothing(1), mCurrentSmoothing(1)
{
    if (mUuid == "")
        mUuid = generate_uuid();
}

void InputChannel::setAdrress(std::string address)
{
    mAddress = address;
}

void InputChannel::setName(const std::string name)
{
    mName = name;
}

// TODO: Change to template.
void InputChannel::setValue(double value)
{
    updateCurrentSmooting();
    if (mCurrentSmoothing > 1) {
        mValue = (mValue * (mCurrentSmoothing -1) + (float) value) / mCurrentSmoothing;
        return;
    }
    mValue = (float) value;
}

void InputChannel::setValue(float value)
{
    updateCurrentSmooting();
    if (mCurrentSmoothing > 1) {
        mValue = (mValue * (mCurrentSmoothing -1) + value) / mCurrentSmoothing;
        return;
    }
    mValue = value;
}

void InputChannel::setValue(int value) {
    updateCurrentSmooting();
    if (mCurrentSmoothing > 1) {
        mIntValue = (mIntValue * (mCurrentSmoothing -1) + value) / mCurrentSmoothing;
        return;
    }

    mIntValue = value;
}

void InputChannel::setValue(vec2 value) {
    updateCurrentSmooting();
    if (mCurrentSmoothing > 1) {
        mVec2Value = (mVec2Value * (float) (mCurrentSmoothing -1) + value) / (float) mCurrentSmoothing;
        return;
    }

    mVec2Value = value;
}

void InputChannel::setValue(vec3 value) {
    updateCurrentSmooting();
    if (mCurrentSmoothing > 1) {
        mVec3Value = (mVec3Value * (float) (mCurrentSmoothing -1) + value) / (float) mCurrentSmoothing;
        return;
    }
    mVec3Value = value;
}

float InputChannel::getValue()
{
    return mValue;
}

int InputChannel::getIntValue()
{
    return mIntValue;
}

vec2 InputChannel::getVec2Value() {
    return mVec2Value;
}

vec3 InputChannel::getVec3Value() {
    return mVec3Value;
}

void InputChannel::setType(InputChannel::Type type) {
    mType = type;
}

InputChannel::Type InputChannel::getType() {
    return mType;
}

std::string InputChannel::getUuid()
{
    return mUuid;
}

std::string InputChannel::getAddress()
{
    return mAddress;
}

std::string InputChannel::getName() const
{
    return mName;
}

void InputChannel::setSmoothing(int smoothing) {
    if (smoothing < 1) {
        return;
    }
    mSmoothing = smoothing;
    mCurrentSmoothing = 1;
}

int InputChannel::getSmoothing() {
    return mSmoothing;
}

void InputChannel::updateCurrentSmooting() {
    if (mCurrentSmoothing < mSmoothing) {
        mCurrentSmoothing++;
    }
}

//////////////////////////////////////////////////////////////////
/// Start parameters.
//////////////////////////////////////////////////////////////////
Parameter::Parameter(Type type, std::string description)
: description(description), type(type)
{
}

Parameter::Parameter()
: description("Default"), type(Parameter::Type::kType_Float)
{
}

void Parameter::setValue(float value) {
    floatValue = value;
}

void Parameter::setValue(int value) {
    intValue = value;
}

void Parameter::setValue(ColorA value) {
    colorValue = value;
}

void Parameter::setValue(vec3 value) {
    vec3Value = value;
}

void Parameter::setValue(vec4 minMax) {
    minIn = minMax.x;
    maxIn = minMax.y;
    min = minMax.z;
    max = minMax.w;
}

float Parameter::getMappedChannelValue() {
    if (channelRef == nullptr) {
        return min;
    }
    float clampedVal = channelRef->getValue();
    if (clampedVal < minIn) {
        clampedVal = minIn;
    }
    else if (clampedVal > maxIn) {
        clampedVal = maxIn;
    }
    return ((clampedVal - minIn) / (maxIn - minIn)) * (max - min) + min;
}
//////////////////////////////////////////////////////////////////
/// Start effects.
//////////////////////////////////////////////////////////////////

std::map<std::string, EffectFactory*> Effect::factories;
std::vector<std::string> Effect::types;

void Effect::registerType(const std::string type, EffectFactory* factory) {
    factories[type] = factory;
    types.push_back(type);
}

std::vector<std::string> Effect::getTypes() {
    return types;
}

EffectRef Effect::create(std::string type, std::string name)
{
    return Effect::factories[type]->create(name);
}

EffectRef Effect::create(std::string type, std::string name, std::string uuid)
{
    return Effect::factories[type]->create(name, uuid);
}

Effect::Effect(std::string name, std::string uuid)
:mName(name), mUuid(uuid), mStatus(kStatus_Off), isTurnedOn(false), fadeInTime(2.0f), fadeOutTime(2.0f), mFadeValue(0.0f), weight(1.0f), oscAddressForOnOff("")
{
    if (uuid == "") {
        mUuid = generate_uuid();
    }
}

Effect::~Effect()
{
    for (auto param: mParams) {
        delete param.second;
    }
}

std::string Effect::getUuid()
{
    return mUuid;
}

std::string Effect::getName()
{
    return mName;
}

std::string Effect::getStatusName() {
    switch (mStatus) {
        case kStatus_Off:
            return "Off";
            break;
            
        case kStatus_On:
            return "On";
            break;
            
        case kStatus_FadingOut:
            return "Fading out";
            break;
            
        case kStatus_FadingIn:
            return "Fading in";
            break;
            
        default:
            break;
    }
    return "Not implemented yet.";
}

photonic::Effect::Status Effect::getStatus() {
    return mStatus;
}



double Effect::getFadeValue() {
    return mFadeValue;
}

void Effect::setName(std::string name)
{
    mName = name;
}

void Effect::addLight(LightRef light)
{
    // First check if the light is present.
    auto it = std::find(mLights.begin(), mLights.end(), light);
    if (it == mLights.end())
        mLights.push_back(light);
}

std::vector<LightRef>::iterator Effect::removeLight(LightRef light)
{
    auto it = std::find(mLights.begin(), mLights.end(), light);
    if (it != mLights.end())
        it = mLights.erase(it);
    return it;
}

void Effect::toggleLight(LightRef light)
{
    auto it = std::find(mLights.begin(), mLights.end(), light);
    if (it != mLights.end()) {
        mLights.erase(it);
    }
    else {
        mLights.push_back(light);
    }    
}

bool Effect::hasLight(LightRef light)
{
    auto it = std::find(mLights.begin(), mLights.end(), light);
    return it != mLights.end();
}

std::vector<LightRef> Effect::getLights()
{
    return mLights;
}

void Effect::setChannel(InputChannelRef channel)
{
    mChannel = channel;
}

InputChannelRef Effect::getChannel()
{
    return mChannel;
}

Parameter* Effect::getParam(int index)
{
    return mParams.at(index);
}

std::map<int, Parameter*>& Effect::getParams()
{
    return mParams;
}

void Effect::drawEditGui() {
    // This might be used in the child classes.
}

void Effect::execute(double dt) {
    // First handle the fading.
    // Start with updating the start of the status transitions.
    if (isTurnedOn && mStatus == kStatus_Off) {
        // The status is update to fading.
        mStatus = kStatus_FadingIn;
        mStatusChangeTime = getElapsedSeconds();
        // Initialize the effect.
        init();
    }
    else if (! isTurnedOn && mStatus == kStatus_On) {
        mStatus = kStatus_FadingOut;
        mStatusChangeTime = getElapsedSeconds();
    }
    
    // Start with the end of the status transitions.
    else if ( isTurnedOn && mStatus == kStatus_FadingIn) {
        if (getElapsedSeconds() - mStatusChangeTime > fadeInTime) {
            mStatus = kStatus_On;
            mFadeValue = 1.0;
            mStatusChangeTime = 0.0;
        }
        else {
            mFadeValue = (getElapsedSeconds() - mStatusChangeTime) / fadeInTime;
        }
    }
    else if ( !isTurnedOn && mStatus == kStatus_FadingOut) {
        if (getElapsedSeconds() - mStatusChangeTime > fadeOutTime) {
            mStatus = kStatus_Off;
            mFadeValue = 0.0;
            mStatusChangeTime = 0.0;
        }
        else {
            mFadeValue = 1.0 - (getElapsedSeconds() - mStatusChangeTime) / fadeOutTime;
        }
    }
    // Handle transitions from fadingOut to fadingIn and backwards.
    else if (! isTurnedOn && mStatus == kStatus_FadingIn) {
        // Calculate a new statuschanged time in order to start a smooth transition.
        float timeDiff = (1.f - mFadeValue) * fadeOutTime;
        mStatusChangeTime = getElapsedSeconds() - timeDiff;
        mStatus = kStatus_FadingOut;
    }
    else if (isTurnedOn && mStatus == kStatus_FadingOut) {
        // Calculate a new statuschanged time in order to start a smooth transition.
        float timeDiff =  mFadeValue * fadeInTime;
        mStatusChangeTime = getElapsedSeconds() - timeDiff;
        mStatus = kStatus_FadingIn;
    }
}

void Effect::listenToOsc(const osc::Message &message) {
    // First handle on/off functionality.
    if (message.getAddress() == oscAddressForOnOff) {
        if ((message.getArgType(0) == osc::ArgType::INTEGER_32 && message.getArgInt32(0) == 1) || (message.getArgType(0) == osc::ArgType::FLOAT && message.getArgFloat(0) == 1.0f)) {
            isTurnedOn = true;
        }
        else if ((message.getArgType(0) == osc::ArgType::INTEGER_32 && message.getArgInt32(0) == 0) || (message.getArgType(0) == osc::ArgType::FLOAT && message.getArgFloat(0) == 0.0f)) {
            isTurnedOn = false;
        }
    }
    for (auto &item : mParams) {
        Parameter* param = item.second;
        if (param->type == photonic::Parameter::kType_OscTrigger) {
            if (message.getAddress() == param->oscAdress) {
                if ((message.getArgType(0) == osc::ArgType::INTEGER_32 && message.getArgInt32(0) == 1) || (message.getArgType(0) == osc::ArgType::FLOAT && message.getArgFloat(0) == 1.0f)) {
                    param->triggerValue = true;
                }
                else if ((message.getArgType(0) == osc::ArgType::INTEGER_32 && message.getArgInt32(0) == 0) || (message.getArgType(0) == osc::ArgType::FLOAT && message.getArgFloat(0) == 0.0f)) {
                    param->triggerValue = false;
                }
            }
        }
    }
}

Effect::Stage Effect::getStage() {
    return kStage_Main;
}

bool Effect::hasOutput() {
    return mStatus != kStatus_Off;
}

void Effect::init() {
    // Normally nothing should be done.
}

void Effect::visualize() {
    // No default visualisation.
}

EffectXmlSerializerRef Effect::getXmlSerializer() {
    return EffectXmlSerializerRef(new EffectXmlSerializer(this));
}

//////////////////////////////////////////////////////////////////
/// Xml Serializer
//////////////////////////////////////////////////////////////////
EffectXmlSerializer::EffectXmlSerializer(Effect* cEffect)
: mEffect(cEffect)
{
}

void EffectXmlSerializer::writeEffect(XmlTree &xmlNode) {
    // for now do nothing yet.
    int dummy = 0;
}

void EffectXmlSerializer::readEffect(XmlTree &xmlNode, const std::vector<LightRef> &lights, std::vector<InputChannelRef> &channels) {
    // for now do nothing yet.
}


//////////////////////////////////////////////////////////////////
/// Start SimpleVolume
//////////////////////////////////////////////////////////////////

SimpleVolumeEffect::SimpleVolumeEffect(std::string name, std::string uuid)
:Effect(name, uuid), mActualVolume(0.0f), mTargetVolume(0.0f)
{
    Parameter* baseColor = new Parameter(Parameter::Type::kType_Color, "Base Color");
    baseColor->colorValue = ColorA(Color::gray(0.5f));
    mParams[kInput_BaseColor] = baseColor;

    Parameter* effectColor = new Parameter(Parameter::Type::kType_Color, "Effect Color");
    effectColor->colorValue = ColorA(Color::gray(0.5f));
    mParams[kInput_EffectColor] = effectColor;

    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_Volume, vec4(0.0f, 60.0f, 0.0f, 1.0f), "Volume");
    registerParam(Parameter::Type::kType_Float, kInput_DecaySpeed, 0.0f, "Decay speed");
}

void SimpleVolumeEffect::execute(double dt) {
    Effect::execute(dt);
    // If the minMax channel is used, use that one.
    if (mParams[kInput_Volume]->channelRef) {
        mChannel = mParams[kInput_Volume]->channelRef;
    }
    if (mChannel != nullptr) {
        float intensity = mChannel->getValue();
        if (mParams[kInput_Volume]->channelRef) {
            intensity = mParams[kInput_Volume]->getMappedChannelValue();
        }
        updateVolumes(intensity, dt);
        for (LightRef light: mLights) {
            if (mChannel) {
                light->setEffectIntensity(mUuid, mActualVolume);
                ColorA endColor = interPolateColors(mParams[kInput_BaseColor]->colorValue, mParams[kInput_EffectColor]->colorValue, mActualVolume);
                light->setEffectColor(mUuid, endColor);
            }
        }
    }
}

void SimpleVolumeEffect::updateVolumes(float input, double dt) {
    if (input > mActualVolume) {
        mActualVolume = input;
    }
    mTargetVolume = input;
    if (mTargetVolume < mActualVolume) {
        if (mParams[kInput_DecaySpeed]->floatValue > 0) {
            mActualVolume -= (1 / mParams[kInput_DecaySpeed]->floatValue) * dt;
            if (mActualVolume < mTargetVolume) {
                mActualVolume = mTargetVolume;
            }
        }
        else {
            mActualVolume = mTargetVolume;
        }
    }
}

ColorA Effect::interPolateColors(ColorA color1, ColorA color2, double intensity) {
    float r = math<float>::max(0.0f, math<float>::min(color1.r + (color2.r - color1.r) * intensity, 1.0f));
    float g = math<float>::max(0.0f, math<float>::min(color1.g + (color2.g - color1.g) * intensity, 1.0f));
    float b = math<float>::max(0.0f, math<float>::min(color1.b + (color2.b - color1.b) * intensity, 1.0f));
    return ColorA(r, g, b);
}

std::string SimpleVolumeEffect::getTypeName() {
    return "Simple volume";
}

bool Effect::supportsLight(LightRef light) {
    return true;
}

std::string SimpleVolumeEffect::getTypeClassName() {
    return "SimpleVolumeEffect";
}

REGISTER_TYPE(SimpleVolumeEffect)

StaticValueEffect::StaticValueEffect(std::string name, std::string uuid)
: Effect(name, uuid)
{
    // Input volume.
    Parameter* newParam = new Parameter(Parameter::Type::kType_Float, "Volume");
    newParam->floatValue = 0.5f;
    mParams[kInput_Volume] = newParam;
    
    // Efefct color.
    Parameter* color = new Parameter(Parameter::Type::kType_Color, "Color");
    color->colorValue = ColorA(Color::gray(0.5f));
    mParams[kInput_Color] = color;
    
}

void StaticValueEffect::execute(double dt) {
    Effect::execute(dt);
    float mStaticVolume = getParam(kInput_Volume)->floatValue;
    ColorA effectColor = mParams[kInput_Color]->colorValue * mParams[kInput_Volume]->floatValue;
    for (auto light: mLights) {
        light->setEffectIntensity(mUuid, mStaticVolume);
        light->setEffectColor(mUuid, effectColor);
    }
}

std::string StaticValueEffect::getTypeName() {
    return "Static volume";
}

std::string StaticValueEffect::getTypeClassName() {
    return "StaticValueEffect";
}

REGISTER_TYPE(StaticValueEffect)

