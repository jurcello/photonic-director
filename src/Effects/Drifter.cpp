//
// Created by Jur de Vries on 20/06/2018.
//

#include "Drifter.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

photonic::Drifter::Drifter(std::string name, std::string uuid)
: Effect(name, uuid), mCurrentPosition(vec3(0.0f)), mTargetPosition(vec3(0.0f))
{
    registerParam(Parameter::Type::kType_Color, kInput_EffectColor, ColorA::gray(0.5f), "Color");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_InputChannel, vec4(0.0f, 2.0f, 0.0f, 1.0f), "Input channel");
    registerParam(Parameter::Type::kType_Float, kInput_BaseSpeed, 0.0f, "BaseSpeed");
    registerParam(Parameter::Type::kType_Float, kInput_MaxSpeed, 1.0f, "Max speed");
    registerParam(Parameter::Type::kType_Float, kInput_BaseFluctuation, 0.0f, "Base fluctuation");
    registerParam(Parameter::Type::kType_Float, kInput_Radius, 2.0f, "Radius");
    registerParam(Parameter::Type::kType_Float, kInput_DropOff, 2.0f, "Dropoff");
    registerParam(Parameter::Type::kType_Float, kInput_FixedVolume, 0.5f, "Fixed Volume (active when > 0)");
    registerParam(Parameter::Type::kType_Float, kInput_SpeedThreshold, 1.0f, "Speed threshold)");
}

void photonic::Drifter::execute(double dt) {
    Effect::execute(dt);
    calculateCurrentPosition(dt);
    updateTargetPosition();

    float radius = mParams[kInput_Radius]->floatValue;
    float dropOff = mParams[kInput_DropOff]->floatValue;
    for (const auto light: mLights) {
        const ColorA &color = (mParams[kInput_InputChannel]->getMappedChannelValue() > 0) ? mParams[kInput_EffectColor]->colorValue : ColorA::black();
        light->setEffectColor(mUuid, color);
        // Calculate the distance.
        float distance = 0;
        distance = glm::distance(mCurrentPosition, vec3(light->position.x, light->position.y, light->position.z));
        // TODO: This code is used more than one. Reuse!
        float intensity = 0.f;
        float absoluteIntensity = mParams[kInput_FixedVolume]->floatValue > 0.0f ? mParams[kInput_FixedVolume]->floatValue : mParams[kInput_InputChannel]->getMappedChannelValue();
        if (distance < radius) {
            intensity = absoluteIntensity;
        }
        else {
            float distanceFromRadius = distance - radius;
            intensity = absoluteIntensity / math<float>::pow(1 + distanceFromRadius, dropOff);
        }
        light->setEffectIntensity(mUuid, intensity);
    }
}

void photonic::Drifter::init() {
    Effect::init();
    mCurrentPosition = getRandomPosition();
    setNewTargetPosition();
}

std::string photonic::Drifter::getTypeName() {
    return "Drifter";
}

std::string photonic::Drifter::getTypeClassName() {
    return "Drifter";
}

void photonic::Drifter::setNewTargetPosition() {
    mTargetPosition = getRandomPosition();
}

void Drifter::updateTargetPosition() {
    vec3 directionVector = getDirectionVector();
    if (glm::length(directionVector) < 0.1f) {
        setNewTargetPosition();
    }
}

void photonic::Drifter::calculateCurrentPosition(double dt) {
    vec3 directionVector = getDirectionVector();
    vec3 displacement = glm::normalize(directionVector) * getSpeed() * (float) dt;
    mCurrentPosition += displacement;

}

float Drifter::getSpeed() {
    float newSpeed = mParams[kInput_BaseSpeed]->floatValue + mParams[kInput_InputChannel]->getMappedChannelValue() * mParams[kInput_MaxSpeed]->floatValue;
    if (newSpeed < mParams[kInput_SpeedThreshold]->floatValue) {
        return 0.0f;
    }
    return newSpeed;
}

vec3 Drifter::getDirectionVector() {
    return mTargetPosition - mCurrentPosition;
}

vec3 Drifter::getRandomPosition() {
    const LightRef light = mLights[mRand.nextInt(0, mLights.size())];
    vec3 position = vec3(light->position.x, light->position.y, light->position.z);
    return position;
}

REGISTER_TYPE(Drifter)