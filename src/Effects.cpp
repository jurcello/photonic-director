//
//  Effects.cpp
//  PhotonicDirector
//
//  Created by Jur de Vries on 24/10/2017.
//

#include "Effects.h"

using namespace cinder::app;

InputChannelRef InputChannel::create(std::string name, std::string address)
{
    return InputChannelRef(new InputChannel(name, address));
}

InputChannel::InputChannel(std::string name, std::string address)
: mName(name), mAddress(address), mValue(0.f)
{
    uuid = std::to_string(time(0));
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
    console() << "Value received: " << value << std::endl;
}

float InputChannel::getValue()
{
    return mValue;
}

std::string InputChannel::getUuid()
{
    return uuid;
}

std::string InputChannel::getAddress()
{
    return mAddress;
}

std::string InputChannel::getName() const
{
    return mName;
}
