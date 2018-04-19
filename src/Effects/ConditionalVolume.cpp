//
// Created by Jur de Vries on 19/04/2018.
//

#include "ConditionalVolume.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

photonic::ConditionalVolume::ConditionalVolume(std::string name, std::string uuid)
        : Effect(name, uuid), mIsOn(false), mWasOn(false), mIsFading(false), mFadeFactor(0.0f)
{
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_VolumeChannel, vec4(0.0f, 1.0f, 0.0f, 1.0f), "Input channel");
    registerParam(Parameter::Type::kType_Channel, kInput_ConditionChannel, "Condition channel");
    registerParam(Parameter::Type::kType_Int, kInput_Min, 40, "Minimum value");
    registerParam(Parameter::Type::kType_Int, kInput_Max, 50, "Maximum value");
    registerParam(Parameter::Type::kType_Float, kInput_OutsideFade, 0.2f, "Fede time when condition not met");
    registerParam(Parameter::Type::kType_Color, kInput_BaseColor, ColorA(Color::gray(0.5f)), "Base Color");
    registerParam(Parameter::Type::kType_Color, kInput_EffectColor, ColorA(Color::gray(0.5f)), "Effect Color");
}

void photonic::ConditionalVolume::execute(double dt) {
    Effect::execute(dt);
    if (mParams[kInput_ConditionChannel]->channelRef && mParams[kInput_VolumeChannel]->channelRef) {
        updateStatus();

        if (mIsOn || !mTimer.isStopped()) {
            float intensity = mParams[kInput_VolumeChannel]->getMappedChannelValue();
            intensity *= mFadeFactor;

            for (LightRef light: mLights) {
                light->setEffectIntensity(mUuid, intensity);
                ColorA endColor = interPolateColors(mParams[kInput_BaseColor]->colorValue, mParams[kInput_EffectColor]->colorValue, intensity);
                light->setEffectColor(mUuid, endColor);
            }
        }
    }
}

void photonic::ConditionalVolume::updateStatus() {

    if (mParams[kInput_ConditionChannel]->channelRef->getIntValue() >= mParams[kInput_Min]->intValue &&
        mParams[kInput_ConditionChannel]->channelRef->getIntValue() <= mParams[kInput_Max]->intValue) {
        mIsOn = true;
    }
    else {
        mIsOn = false;
    }
    if (mIsOn != mWasOn) {
        mIsFading = true;
        mTimer.start(0.0);
        mWasOn = mIsOn;
    }
    else {
        float fadeFactor = 1.0f;
        if (mTimer.getSeconds() > mParams[kInput_OutsideFade]->floatValue) {
            mIsFading = false;
            mTimer.stop();
        }
        else {
            fadeFactor = (float) mTimer.getSeconds() / mParams[kInput_OutsideFade]->floatValue;
        }
        mFadeFactor = mIsOn ? fadeFactor : 1.0f - fadeFactor;
    }
}

std::string photonic::ConditionalVolume::getTypeName() {
    return "Conditional volume";
}

std::string photonic::ConditionalVolume::getTypeClassName() {
    return "ConditionalVolume";
}

REGISTER_TYPE(ConditionalVolume)