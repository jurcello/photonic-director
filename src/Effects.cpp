//
//  Effects.cpp
//  PhotonicDirector
//
//  Created by Jur de Vries on 24/10/2017.
//

#include "Effects.h"
#include "Utils.h"

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
: mName(name), mAddress(address), mValue(0.f), mUuid(uuid)
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

void InputChannel::setValue(float value)
{
    mValue = value;
}

float InputChannel::getValue()
{
    return mValue;
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

Effect::Effect(std::string name)
:mName(name), mUuid(generate_uuid())
{
}

Effect::Effect(std::string name, std::string uuid)
:mName(name), mUuid(uuid)
{
}

std::string Effect::getUuid()
{
    return mUuid;
}

std::string Effect::getName()
{
    return mName;
}

void Effect::setName(std::string name)
{
    mName = name;
}

void Effect::addLight(Light* light)
{
    // First check if the light is present.
    auto it = std::find(mLights.begin(), mLights.end(), light);
    if (it == mLights.end())
        mLights.push_back(light);
}

void Effect::removeLight(Light *light)
{
    auto it = std::find(mLights.begin(), mLights.end(), light);
    if (it != mLights.end())
        mLights.erase(it);
}

void Effect::toggleLight(Light *light)
{
    auto it = std::find(mLights.begin(), mLights.end(), light);
    if (it != mLights.end()) {
        mLights.erase(it);
    }
    else {
        mLights.push_back(light);
    }    
}

bool Effect::hasLight(Light *light)
{
    auto it = std::find(mLights.begin(), mLights.end(), light);
    return it != mLights.end();
}

std::vector<Light*> Effect::getLights()
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
//////////////////////////////////////////////////////////////////
/// Start SimpleVolume
//////////////////////////////////////////////////////////////////

void SimpleVolumeEffect::execute(float dt) {
    for (Light* light: mLights) {
        if (mChannel)
            light->setEffectIntensity(mUuid, mChannel->getValue());
    }
}

std::string SimpleVolumeEffect::getTypeName() {
    return "Simple volume";
}

std::string SimpleVolumeEffect::getTypeClassName() {
    return "SimpleVolumeEffect";
}

REGISTER_TYPE(SimpleVolumeEffect)

void StaticValueEffect::execute(float dt) {
    for (auto light: mLights) {
        light->setEffectIntensity(mUuid, 0.5);
    }
}

std::string StaticValueEffect::getTypeName() {
    return "Static volume";
}

std::string StaticValueEffect::getTypeClassName() {
    return "StaticValueEffect";
}

REGISTER_TYPE(StaticValueEffect)

