
#include "ShutDownHuman.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

ShutDownHuman::ShutDownHuman(std::string name, std::string uuid)
: Effect(name, uuid), mIsShutdown(false)
{
    registerParam(Parameter::Type::kType_Float, kInput_FirstStageTime, 1.0f, "First stage duration");
    registerParam(Parameter::Type::kType_Float, kInput_SecondStageTime, 1.0f, "Second stage duration");
    registerParam(Parameter::Type::kType_Float, kInput_ThirdStageTime, 1.0f, "Thrid stage duration");
    registerParam(Parameter::Type::kType_Float, kInput_MaxOutputFirst, 1.0f, "Output boost first stage");
    registerParam(Parameter::Type::kType_Float, kInput_MaxOutputThird, 1.0f, "Output boost third stage");
    registerParam(Parameter::Type::kType_Float, kInput_PerlinSpeed, 1.0f, "Perlin speed");
    registerParam(Parameter::Type::kType_Float, kInput_NoiseAmount, 0.2f, "Perlin Noise Amount");
    registerParam(Parameter::Type::kType_Float, kInput_RandomNoiseAmount, 0.2f, "Random Noise Amount");
    registerParam(Parameter::Type::kType_Float, kInput_SecondStageNoiseAmount, 0.2f, "Second stage relative noise (1 is unity)");



    Parameter* triggerChannel = new Parameter(Parameter::Type::kType_OscTrigger, "Trigger shutdown");
    triggerChannel->triggerValue = false;
    triggerChannel->oscAdress = "/shutdownHuman/trigger";
    mParams[kInput_TriggerChannel] = triggerChannel;
}


void ShutDownHuman::execute(double dt) {
    Effect::execute(dt);
    updateState();
    if (mIsShutdown) {
        switch (mCurrentStage) {
            case EffectStage::first:
                blowupIntensities(
                        ((float)mTimer.getSeconds() / mParams[kInput_FirstStageTime]->floatValue) *
                        mParams[kInput_MaxOutputFirst]->floatValue,
                        mParams[kInput_MaxOutputFirst]->floatValue
                );
                break;

            case EffectStage::second: {
                const float intensity = ((
                                                 1 - (float)mTimer.getSeconds() / mParams[kInput_SecondStageTime]->floatValue
                                         ) * 0.9f + 0.1f);
                setLightIntensities(intensity);
            }
                break;

            case EffectStage::third:
                blowupIntensities(
                        ((float)mTimer.getSeconds() / mParams[kInput_ThirdStageTime]->floatValue) *
                        mParams[kInput_MaxOutputThird]->floatValue,
                        mParams[kInput_MaxOutputThird]->floatValue

                );
                break;

            default:
                muteAll();

        }
        if (mCurrentStage != EffectStage::idle) {
            addNoise();
        }
    }
}

Effect::Stage ShutDownHuman::getStage() {
    return Stage::kStage_After;
}

void ShutDownHuman::updateState() {
    if (!mIsShutdown && mParams[kInput_TriggerChannel]->triggerValue) {
        mIsShutdown = true;
        mCurrentStage = EffectStage::first;
        sendTrigger(mOscOutTriggerFirstStage, 127);
        mTimer.start();
        return;
    }
    if (mIsShutdown && mCurrentStage == EffectStage::first) {
        if (mTimer.getSeconds() > mParams[kInput_FirstStageTime]->floatValue) {
            mTimer.start(0.0);
            mCurrentStage = EffectStage::second;
            sendTrigger(mOscOutTriggerSecondStage, 127);
        }
        return;
    }
    if (mIsShutdown && mCurrentStage == EffectStage::second) {
        if (mTimer.getSeconds() > mParams[kInput_SecondStageTime]->floatValue) {
            mTimer.start(0.0);
            sendTrigger(mOscOutTriggerThirdStage, 127);
            mCurrentStage = EffectStage::third;
        }
        return;
    }
    if (mIsShutdown && mCurrentStage == EffectStage::third) {
        if (mTimer.getSeconds() > mParams[kInput_ThirdStageTime]->floatValue) {
            mTimer.stop();
            sendTrigger(mOscOutOffToggle, 0);
            mCurrentStage = EffectStage::idle;
        }
        return;
    }
    if (mIsShutdown && mCurrentStage == EffectStage::idle && !mParams[kInput_TriggerChannel]->triggerValue) {
        sendTrigger(mOscOutOffToggle, 127);
        mIsShutdown = false;
    }
}

void ShutDownHuman::setLightIntensities(float intensity) {
    for (const auto &light : mLights) {
        light->intensity = light->intensity < intensity ? light->intensity : intensity;
        if (light->isColorEnabled()) {
            // Wash out color.
            light->color *= 1 + intensity;
        }
    }
}

void ShutDownHuman::muteAll() {
    for (const auto &light : mLights) {
        light->intensity = 0.f;
        if (light->isColorEnabled()) {
            light->color = ColorA::black();
        }
    }
}

void ShutDownHuman::blowupIntensities(float ratio, float max) {
    for (const auto &light : mLights) {
        if (light->intensity == 0.f) {
            light->intensity = 0.2f;
        }
        float newIntensity = light->intensity * 1 + math<float>::sqrt(ratio);
        if (newIntensity > max) {
            newIntensity = max;
        }
        light->intensity = newIntensity;

    }
}

void ShutDownHuman::addNoise() {
    float randomNoise = ((rand() % 256) / 256.f) * mParams[kInput_RandomNoiseAmount]->floatValue;
    for (const auto &light : mLights) {
        float perlinNoise =
                mPerlin.noise(light->getPosition().x, light->getPosition().x, (float)mTimer.getSeconds() * mParams[kInput_PerlinSpeed]->floatValue / 10.f) *
                mParams[kInput_NoiseAmount]->floatValue;
        float noise = randomNoise + perlinNoise;
        if (mCurrentStage == EffectStage::second) {
            noise *= mParams[kInput_SecondStageNoiseAmount]->floatValue;
        }
        light->intensity += noise;
        if (light->isColorEnabled()) {
            light->color += noise;
        }
    }
}

void ShutDownHuman::sendTrigger(std::string address, int value) {
    if (mOscSender != nullptr) {
        osc::Message message(address);
        message.append(value);
        mOscSender->send(message);
    }
}

void ShutDownHuman::drawEditGui() {
    std::string oscFirst = "Osc address for trigger first stage: " + mOscOutTriggerFirstStage;
    ui::Text("%s", oscFirst.c_str());
    std::string oscSecond = "Osc address for trigger second stage: " + mOscOutTriggerSecondStage;
    ui::Text("%s", oscSecond.c_str());
    std::string oscThird = "Osc address for trigger third stage: " + mOscOutTriggerThirdStage;
    ui::Text("%s", oscThird.c_str());
    std::string oscOffToggle = "Osc address for off toggle: " + mOscOutOffToggle;
    ui::Text("%s", oscOffToggle.c_str());
}

std::string ShutDownHuman::getTypeName() {
    return "ShutDownHuman";
}

std::string ShutDownHuman::getTypeClassName() {
    return "ShutDownHuman";
}


REGISTER_TYPE(ShutDownHuman)