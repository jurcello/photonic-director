
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
    registerParam(Parameter::Type::kType_Float, kInput_FirstStageEaseComponent, 0.2f, "First stage ease component (towards zero more steep)");
    registerParam(Parameter::Type::kType_Float, kInput_SecondsStageDropPercentage, 10.f, "Second stage drop percentage (between 0 and 100)");



    Parameter* triggerChannel = new Parameter(Parameter::Type::kType_OscTrigger, "Trigger shutdown");
    triggerChannel->triggerValue = false;
    triggerChannel->oscAdress = "/shutdownHuman/trigger";
    mParams[kInput_TriggerChannel] = triggerChannel;
}


void ShutDownHuman::execute(double dt) {
    Effect::execute(dt);
}

void ShutDownHuman::executePost(double dt) {
    updateState();
    if (mIsShutdown) {
        switch (mCurrentStage) {
            case EffectStage::first:
                blowupIntensities(
                        (float)mTimer.getSeconds() / mParams[kInput_FirstStageTime]->floatValue,
                        mParams[kInput_MaxOutputFirst]->floatValue
                );
                break;

            case EffectStage::second: {
                const float dropFactor = (100.f - mParams[kInput_SecondsStageDropPercentage]->floatValue) * 0.01f;
                const float intensity = math<float>::clamp(dropFactor, 0.f, 1.f) * (1 - (float)mTimer.getSeconds() / mParams[kInput_SecondStageTime]->floatValue);
                setLightIntensities(intensity);
            }
                break;

            case EffectStage::third:
                blowupIntensities(
                        (float)mTimer.getSeconds() / mParams[kInput_ThirdStageTime]->floatValue,
                        mParams[kInput_MaxOutputThird]->floatValue

                );
                break;

            default:
                muteAll();

        }
        if (mCurrentStage != EffectStage::idle) {
            addNoise();
        }
        saveLightIntensities();
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

void ShutDownHuman::blowupIntensities(float ratio, float max) {

    //        const float increase = max * (1.f - math<float>::pow(2 * math<float>::abs(ratio - 0.5f), 0.7f));
    // Following formula might look very difficult.
    // Just draw it in grapher to see the function.
    // Basically it is a function that returns 0 at ratio is 0 and 1.
    // At 0.5 it reaches one. The behavior towards one is controlled by the ease component.
    const float power = mParams[kInput_FirstStageEaseComponent]->floatValue;
    const float normalease = (math<float>::pow(.5f, power) - math<float>::pow(math<float>::abs(ratio - 0.5f), power)) / math<float>::pow(.5f, power);
    const float increase = max * normalease;
    app::console() << "Ratio " << ratio << ", increase: " << increase << std::endl;
    for (const auto &light : mLights) {
        setSwitchedOffLightIntensity(light);
        float newIntensity = light->intensity + light->intensity * increase;
        if (newIntensity > max) {
            newIntensity = max;
        }
        light->intensity = newIntensity;
        if (light->isColorEnabled()) {
            light->color += normalizeColor(light->color) * increase;
        }

    }
}

void ShutDownHuman::setLightIntensities(float intensity) {
    for (const auto &light : mLights) {
        setSwitchedOffLightIntensity(light);
        light->intensity *= intensity;
        if (light->isColorEnabled()) {
            // Wash out color.
            light->color *= intensity;
        }
    }
}

void ShutDownHuman::setSwitchedOffLightIntensity(LightRef light) {
    if (light->intensity == 0.f) {
        light->intensity = 0.2f;
    }
    if (light->color == ColorA::black()) {
        light->color = ColorA::gray(0.4);
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

void ShutDownHuman::saveLightIntensities() {
    for (const auto light: mLights) {
        light->setEffectIntensity(mUuid, light->intensity);
        if (light->isColorEnabled()) {
            light->setEffectColor(mUuid, light->color);
        }
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
    std::string stage = "idle";
    switch (mCurrentStage) {
        case EffectStage::first:
            stage = "first";
            break;
        case EffectStage::second:
            stage = "second";
            break;
        case EffectStage::third:
            stage = "third";
            break;
    }
    ui::Text("Stage: %s", stage.c_str());
}

std::string ShutDownHuman::getTypeName() {
    return "ShutDownHuman";
}

std::string ShutDownHuman::getTypeClassName() {
    return "ShutDownHuman";
}


REGISTER_TYPE(ShutDownHuman)