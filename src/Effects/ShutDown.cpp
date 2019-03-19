
#include "ShutDown.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

ShutDown::ShutDown(std::string name, std::string uuid)
: Effect(name, uuid), mIsShutDown(false), mLastUpdateSecondsAgo(0.f)
{
    registerParam(Parameter::Type::kType_Float, kInput_ShutdownTime, 1.0f, "Shutdown time");
    registerParam(Parameter::Type::kType_Float, kInput_PerlinSpeed, 1.0f, "Perlin speed");
    registerParam(Parameter::Type::kType_Float, kInput_NoiseAmount, 0.2f, "Perlin Noise Amount");
    registerParam(Parameter::Type::kType_Float, kInput_RandomNoiseAmount, 0.2f, "Random Noise Amount");
    registerParam(Parameter::Type::kType_Float, kInput_TimeBetweenUpdates, 0.0f, "Time between updates (during flickering)");

    Parameter* triggerChannel = new Parameter(Parameter::Type::kType_OscTrigger, "Trigger shutdown");
    triggerChannel->triggerValue = false;
    triggerChannel->oscAdress = "/shutdown/trigger";
    mParams[kInput_TriggerChannel] = triggerChannel;
}

void ShutDown::execute(double dt) {
    Effect::execute(dt);
    // TODO: this is a quick fix.
    // Maybe we need an event which tells if the effect is enabled.
    if (mFadeValue > 0 && mFadeValue < 1.f) {
        mParams[kInput_TriggerChannel]->triggerValue = false;
    }
}

void ShutDown::executePost(double dt) {
    Effect::executePost(dt);
    updateState();
    if (isTurnedOn && !mTimer.isStopped()) {
        if (mParams[kInput_TimeBetweenUpdates]->floatValue == 0.f || mLastUpdateSecondsAgo > mParams[kInput_TimeBetweenUpdates]->floatValue) {
            auto elapsedTime = (float) mTimer.getSeconds();
            // The random noise will be used to let the lights flikker: on or of.
            float randomNoise = ((rand() % 256) / 256.f);
            randomNoise = randomNoise <  mParams[kInput_RandomNoiseAmount]->floatValue ? 1.f : 0.f;
            sendSoundVolume(randomNoise);

            for (const auto &light : mLights) {
                float perlinNoise = mPerlin.noise(light->getPosition().x, light->getPosition().y, elapsedTime * mParams[kInput_PerlinSpeed]->floatValue / 10.f) * mParams[kInput_NoiseAmount]->floatValue;
                // Create random noise containing max 256 steps.
                light->intensity -= math<float>::abs(perlinNoise + randomNoise);
                light->color = light->color * light->intensity;
            }
            mLastUpdateSecondsAgo = 0;
        }
        else if (mParams[kInput_TimeBetweenUpdates]->floatValue > 0) {
            mLastUpdateSecondsAgo += dt;
        }
    }
    else if (isTurnedOn && mIsShutDown) {
        for (const auto &light : mLights) {
            light->intensity = 0;
            light->color = ColorA::black();
        }
    }
}

void ShutDown::updateState() {
    if (mStatus != photonic::Effect::Status::kStatus_On) {
        return;
    }
    if (mParams[kInput_TriggerChannel]->triggerValue && mTimer.isStopped() && !mIsShutDown) {
        mTimer.start();
        mIsShutDown = true;
        sendSoundTrigger(mOscOutAddressTriggerFlickerStart);
        return;
    }
    if (!mTimer.isStopped() && (mTimer.getSeconds() > mParams[kInput_ShutdownTime]->floatValue)) {
        mTimer.stop();
        sendSoundVolume(0.f);
        sendSoundTrigger(mOscOutAddressTrigger);
        sendTrigger(mOscOutOffToggle, 0);
    }
    if (!mParams[kInput_TriggerChannel]->triggerValue) {
        if (mIsShutDown) {
            // Send signal to enable sound again.
            sendTrigger(mOscOutOffToggle, 127);
        }
        mIsShutDown = false;
    }
}

void ShutDown::sendSoundTrigger(std::string address) {
    if (mOscSender != nullptr) {
        osc::Message message(address);
        message.append(127);
        mOscSender->send(message);
    }
}

void ShutDown::sendTrigger(std::string address, int value) {
    if (mOscSender != nullptr) {
        osc::Message message(address);
        message.append(value);
        mOscSender->send(message);
    }
}

void ShutDown::sendSoundVolume(float volume) {
    if (mOscSender != nullptr) {
        osc::Message message(mOscOutAddressVolume);
        message.append(volume);
        mOscSender->send(message);
    }
}

void ShutDown::init() {
    mPerlin.setSeed(clock());
}

Effect::Stage ShutDown::getStage() {
    return Stage::kStage_After;
}

void ShutDown::drawEditGui() {
    std::string osc = "Osc address for trigger out: " + mOscOutAddressTrigger;
    ui::Text("%s", osc.c_str());
    std::string oscFlicker = "Osc address for trigger flciker out: " + mOscOutAddressTriggerFlickerStart;
    ui::Text("%s", oscFlicker.c_str());
    std::string oscVolume = "Osc address for sound volume out: " + mOscOutAddressVolume;
    ui::Text("%s", oscVolume.c_str());
    std::string oscOffToggle = "Osc address for off toggle: " + mOscOutOffToggle;
    ui::Text("%s", oscOffToggle.c_str());

}

std::string ShutDown::getTypeName() {
    return "ShutDown";
}

std::string ShutDown::getTypeClassName() {
    return "ShutDown";
}


REGISTER_TYPE(ShutDown)