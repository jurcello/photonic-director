//
// Created by Jur de Vries on 05/03/2018.
//

#include "Ripples.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

photonic::Ripples::Ripples(std::string name, std::string uuid) : Effect(name, uuid)
{
    registerParam(Parameter::Type::kType_Color, kInput_BaseColor, ColorA(Color::gray(0.5f)), "Base color");
    registerParam(Parameter::Type::kType_Color, kInput_EffectColor, ColorA(Color::gray(0.5f)), "Effect color");
    registerParam(Parameter::Type::kType_Float, kInput_EffectVolume, 0.5f, "Volume");
    registerParam(Parameter::Type::kType_Float, kInput_NoiseAmount, 0.5f, "Noise amount");
    registerParam(Parameter::Type::kType_Float, kInput_NoiseSpeed, 1.0f, "Noise speed");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_ExternalVolume, vec4(0.0f, 60.0f, 0.0f, 1.0f), "External volume");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_ExternalNoiseAmount, vec4(0.0f, 60.0f, 0.0f, 1.0f), "External noise amount");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_ExternalNoiseSpeed, vec4(0.0f, 60.0f, 0.0f, 1.0f), "External noise speed");
}

void photonic::Ripples::init() {
    Effect::init();
    mPerlin.setSeed(clock());
    mTimer.start();

}

void photonic::Ripples::execute(double dt) {
    Effect::execute(dt);
    // If there are external inputs, update the inner volumes.
    if (mParams[kInput_ExternalVolume]->channelRef) {
        mParams[kInput_EffectVolume]->floatValue = mParams[kInput_ExternalVolume]->getMappedChannelValue();
    }
    if (mParams[kInput_ExternalNoiseAmount]->channelRef) {
        mParams[kInput_NoiseAmount]->floatValue = mParams[kInput_ExternalNoiseAmount]->getMappedChannelValue();
    }
    if (mParams[kInput_ExternalNoiseSpeed]->channelRef) {
        mParams[kInput_NoiseSpeed]->floatValue = mParams[kInput_ExternalNoiseSpeed]->getMappedChannelValue();
    }
    auto elapsedTime = (float) mTimer.getSeconds();
    if (isTurnedOn) {
        for (const auto &light : mLights) {
            float noise = mPerlin.noise(light->getPosition().x, light->getPosition().y, elapsedTime * mParams[kInput_NoiseSpeed]->floatValue / 10.f) * mParams[kInput_NoiseAmount]->floatValue;
            float intensity = mParams[kInput_EffectVolume]->floatValue + noise;

            bool baseIsBlack = mParams[kInput_BaseColor]->colorValue.r < 0.05f && mParams[kInput_BaseColor]->colorValue.g < 0.05f && mParams[kInput_BaseColor]->colorValue.b < 0.05f;
            ColorA color = interPolateColors(mParams[kInput_BaseColor]->colorValue, mParams[kInput_EffectColor]->colorValue, intensity);
            light->setEffectColor(mUuid, color);
            auto lightIntensity = (float) (light->isColorEnabled() && ! baseIsBlack ? 1.0f : intensity);
            light->setEffectIntensity(mUuid, lightIntensity);
        }
    }
}

std::string photonic::Ripples::getTypeName() {
    return "Ripples";
}

std::string photonic::Ripples::getTypeClassName() {
    return "Ripples";
}

REGISTER_TYPE(Ripples)