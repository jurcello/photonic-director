//
// Created by Jur de Vries on 14/03/2018.
//

#include "Droplet.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

photonic::Droplet::Droplet(std::string name, std::string uuid)
: Effect(name, uuid), mInactiveTime(0.0f)
{
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_InputChannel, vec4(0.0f, 1.0f, 0.0f, 1.0f), "Input channel");
    registerParam(Parameter::Type::kType_Float, kInput_Theshold, 0.2f, "Input threshold");
    registerParam(Parameter::Type::kType_Float, kInput_MinIntensity, 0.5f, "Minimal intensity");
    registerParam(Parameter::Type::kType_Float, kInput_MaxIntensity, 1.0f, "Maximal intensity");
    registerParam(Parameter::Type::kType_Float, kInput_DropInterval, 0.2f, "Drop interval");
    registerParam(Parameter::Type::kType_Float, kInput_DropIntervalRandomness, 0.2f, "Drop interval randomness");
    registerParam(Parameter::Type::kType_Float, kInput_DecreaseSpeed, 1.0f, "Maximal duration of a drop");
    registerParam(Parameter::Type::kType_Color, kInput_BaseColor, ColorA(Color::gray(0.0f)), "Base Color");
    registerParam(Parameter::Type::kType_Color, kInput_EffectColor, ColorA(Color::gray(5.0f)), "Effect Color");
}

void photonic::Droplet::execute(double dt) {
    Effect::execute(dt);

    float stepSize = (float) dt / mParams[kInput_DecreaseSpeed]->floatValue;

    if (mParams[kInput_InputChannel]->channelRef) {
        if (mTimer.isStopped() && mParams[kInput_InputChannel]->getMappedChannelValue() > mParams[kInput_Theshold]->floatValue) {
            auto targetLight = getRandomLight();
            // Set the intensity of the randomLight.
            float startIntensity = mParams[kInput_MinIntensity]->floatValue + mRand.nextFloat() / (mParams[kInput_MaxIntensity] - mParams[kInput_MinIntensity]);
            mStartValues.insert(std::pair<LightRef, float>(targetLight, startIntensity));
            mEndValues.insert(std::pair<LightRef, float>(targetLight, 0.0f));
            mCurrentValues.insert(std::pair<LightRef, float>(targetLight, startIntensity));

            targetLight->setEffectIntensity(mUuid, startIntensity);
            auto startColor = interPolateColors(mParams[kInput_BaseColor]->colorValue, mParams[kInput_EffectColor]->colorValue, startIntensity);
            targetLight->setEffectColor(mUuid, startColor);
            // Start the timer.
            mTimer.start(0.0f);
            float delta = mParams[kInput_DropIntervalRandomness]->floatValue * mParams[kInput_DropInterval]->floatValue;
            if (delta > mParams[kInput_DropInterval]->floatValue) {
                delta = mParams[kInput_DropInterval]->floatValue;
            }
            mInactiveTime = mRand.nextFloat(mParams[kInput_DropInterval]->floatValue - delta , mParams[kInput_DropInterval]->floatValue + delta);
        }
        // Now loop over the lights and update values if necessary.
        for (auto light : mLights) {
            auto it = mCurrentValues.find(light);
            if (it != mCurrentValues.end()) {
                float currentValue = it->second - stepSize;
                it->second = currentValue;
                const float endValue = mEndValues.find(light)->second;
                const float startValue = mStartValues.find(light)->second;
                if (currentValue <= endValue) {
                    currentValue = endValue;
                    // Remove the light from the maps.
                    mStartValues.erase(light);
                    mCurrentValues.erase(light);
                    mEndValues.erase(light);
                }
                auto color = interPolateColors(mParams[kInput_BaseColor]->colorValue, mParams[kInput_EffectColor]->colorValue, currentValue);
                light->setEffectIntensity(mUuid, currentValue);
                light->setEffectColor(mUuid, color);
            }
        }

        if (mTimer.getSeconds() > mInactiveTime) {
            mTimer.stop();
        }
    }

}

std::string photonic::Droplet::getTypeName() {
    return "Droplet";
}

std::string photonic::Droplet::getTypeClassName() {
    return "Droplet";
}

LightRef Droplet::getRandomLight() {
    int position = mRand.nextInt(0, mLights.size());
    return mLights[position];
}

REGISTER_TYPE(Droplet)