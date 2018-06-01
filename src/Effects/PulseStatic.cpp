//
// Created by Jur de Vries on 31/05/2018.
//

#include "PulseStatic.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

PulseStatic::PulseStatic(std::string name, std::string uuid)
: Effect(name, uuid), mCurrentValue(0.0f), mTargetValue(0.0f)
{

    Parameter* trigger = new Parameter(Parameter::Type::kType_OscTrigger, "Trigger to start");
    trigger->triggerValue = false;
    trigger->oscAdress = "/trigger";
    mParams[kInput_Trigger] = trigger;

    registerParam(Parameter::Type::kType_Float, kInput_Volume, 0.2f, "Volume");
    registerParam(Parameter::Type::kType_Float, kInput_DecreaseTime, 0.8f, "Decrease Time");
    registerParam(Parameter::Type::kType_Color, kInput_BaseColor, ColorA(Color::gray(0.0f)), "Base Color");
    registerParam(Parameter::Type::kType_Color, kInput_EffectColor, ColorA(Color::gray(5.0f)), "Effect Color");

}

void PulseStatic::execute(double dt) {
    Effect::execute(dt);
    updateInnerState();
    ColorA color = interPolateColors(mParams[kInput_BaseColor]->colorValue, mParams[kInput_EffectColor]->colorValue, mCurrentValue);
    for (const auto light : mLights) {
        light->setEffectIntensity(mUuid, mCurrentValue);
        light->setEffectColor(mUuid, color);
    }
}

void PulseStatic::updateInnerState() {
    float decreaseTime = mParams[kInput_DecreaseTime]->floatValue;
    if (mParams[kInput_Trigger]->triggerValue) {
        mTimer.start(0.0f);
        mCurrentValue = mParams[kInput_Volume]->floatValue;
    }
    if (! mTimer.isStopped()) {
        float ratio = (decreaseTime - (float) mTimer.getSeconds()) / decreaseTime;
        if (ratio < 0) {
            mCurrentValue = 0.0f;
        }
        else {
            mCurrentValue = mParams[kInput_Volume]->floatValue * ratio;
        }
    }
    if (mTimer.getSeconds() > decreaseTime) {
        mCurrentValue = 0.0f;
        mTimer.stop();
    }
}

std::string photonic::PulseStatic::getTypeClassName() {
    return "PulseStatic";
}

std::string photonic::PulseStatic::getTypeName() {
    return "Pulse static";
}

REGISTER_TYPE(PulseStatic)