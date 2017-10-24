//
//  Effects.cpp
//  PhotonicDirector
//
//  Created by Jur de Vries on 24/10/2017.
//

#include "Effects.h"

using namespace cinder::app;

InputChannelRef InputChannel::create(std::string address)
{
    return InputChannelRef(new InputChannel(address));
}

InputChannel::InputChannel(std::string address)
: mAddress(address), mValue(0.f)
{
}

void InputChannel::setAdrress(std::string address)
{
    mAddress = address;
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

std::string InputChannel::getAddress()
{
    return mAddress;
}
