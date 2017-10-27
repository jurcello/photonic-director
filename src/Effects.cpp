//
//  Effects.cpp
//  PhotonicDirector
//
//  Created by Jur de Vries on 24/10/2017.
//

#include "Effects.h"

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

EffectRef Effect::create(std::string name)
{
    return EffectRef(new Effect(name));
}

EffectRef Effect::create(std::string name, std::string uuid)
{
    return EffectRef(new Effect(name, uuid));
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

void Effect::execute(float dt) {
    for (Light* light: mLights) {
        if (mChannel)
            light->intensity = mChannel->getValue();
    }
}
