//
// Created by Jur de Vries on 29/03/2018.
//

#include "OriginatingWave.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

OriginatingWave::OriginatingWave(std::string name, std::string uuid)
: Effect(name, uuid), mBuffer(1000)
{
    registerParam(Parameter::Type::kType_Vector3, kInput_Location, vec3(1.f), "Location");
    registerParam(Parameter::Type::kType_Float, kInput_Decay, 1.0f, "Decay");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_InputChannel, vec4(0.0f, 1.0f, 0.0f, 1.0f), "Input");
    registerParam(Parameter::Type::kType_Float, kInput_Speed, 1.0f, "Speed");
    registerParam(Parameter::Type::kType_Color, kInput_EffectColor, ColorA(Color::gray(0.5f)), "Effect Color");
}

void OriginatingWave::execute(double dt) {
    Effect::execute(dt);
    float decay = mParams[kInput_Decay]->floatValue;
    if (mParams[kInput_InputChannel]->channelRef) {
        mBuffer.setSpeed(mParams[kInput_Speed]->floatValue);
        const float volume = mParams[kInput_InputChannel]->getMappedChannelValue();
        // Store the volume in the ringbuffer.
        mBuffer.pushValue(volume);

        for (auto light: mLights) {
            // Calculate the distance.
            float distance = 0;
            distance = glm::distance(mParams[kInput_Location]->vec3Value, vec3(light->position.x, light->position.y, light->position.z));
            float lightVolume = mBuffer.getValueAtDistance(distance);
            float power = math<float>::pow(distance*3, decay);
            if (power < 1) power = 1;
            if (power > 10.f) power = 10.f;
            lightVolume /= power;
            const ColorA &color = (lightVolume > 0) ? mParams[kInput_EffectColor]->colorValue : ColorA::black();
            light->setEffectColor(mUuid, color);
            light->setEffectIntensity(mUuid, lightVolume);
        }
    }
}

std::string OriginatingWave::getTypeName() {
    return "Originating Wave";
}

std::string OriginatingWave::getTypeClassName() {
    return "OriginatingWave";
}

ValueBuffer::ValueBuffer(int maxSize)
: mTimeInverval(1.0f/60.f), mSpeed(1.0f)
{
    std::vector<float> newVector(maxSize);
    mValues = newVector;
    mLastPos = 0;
}

void ValueBuffer::pushValue(float value) {
    int newPos = mLastPos + 1;
    while (newPos >= mValues.size()) {
        newPos -= mValues.size();
    }
    mValues.at(newPos) = value;
    mLastPos = newPos;
}

void ValueBuffer::setTimeInterval(float timeInterval) {
    mTimeInverval = timeInterval;
}

float ValueBuffer::getValueAtDistance(float distance) {
    // Calculate the steps for a certain distance.
    auto steps = (int) roundf((distance / mSpeed) * (1 / mTimeInverval));
    if (steps > mValues.size()) {
        return 0.f;
    }
    // Go back in time.
    int position = mLastPos - steps;
    if (position < 0) {
        position += mValues.size();
    }
    return mValues.at(position);
}

void ValueBuffer::setSpeed(float speed) {
    mSpeed = speed;
}

REGISTER_TYPE(OriginatingWave)