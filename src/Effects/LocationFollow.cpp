//
// Created by Jur de Vries on 14/02/2018.
//

#include "LocationFollow.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"


using namespace cinder;
using namespace photonic;
using namespace ci;


photonic::LocationFollow::LocationFollow(std::string name, std::string uuid)
: Effect(name, uuid)
{
    // Create the inputs.
    Parameter* effectColor = new Parameter(Parameter::Type::kType_Color, "Effect Color");
    effectColor->colorValue = ColorA(Color::gray(0.8f));
    mParams[kInput_EffectColor] = effectColor;

    Parameter* radius = new Parameter(Parameter::Type::kType_Float, "Radius");
    radius->floatValue = 0.5f;
    mParams[kInput_Radius] = radius;

    Parameter* intensity = new Parameter(Parameter::Type::kType_Float, "Intensity");
    intensity->floatValue = 1.0f;
    mParams[kInput_Intensity] = intensity;

    Parameter* dropOff = new Parameter(Parameter::Type::kType_Float, "Drop Off");
    dropOff->floatValue = 1.0f;
    mParams[kInput_DropOff] = dropOff;

    registerParam(Parameter::Type::kType_Float, kInput_FadeOutTime, 0.1f, "Fade out time");

    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_IntensityChannel, vec4(0.f, 1.0f, 0.0f, 1.f), "Intensity Channel");
}

void photonic::LocationFollow::execute(double dt) {
    float radius = mParams[kInput_Radius]->floatValue;
    float stepSize = 0.f;
    if (mParams[kInput_FadeOutTime]->floatValue > 0.f) {
        stepSize = dt / mParams[kInput_FadeOutTime]->floatValue;
    }
    if (mParams[kInput_IntensityChannel]->channelRef != nullptr) {
        mParams[kInput_Intensity]->floatValue = mParams[kInput_IntensityChannel]->getMappedChannelValue();
    }
    Effect::execute(dt);
    if (mChannel) {
        float dropOff = mParams[kInput_DropOff]->floatValue;
        for (const auto light: mLights) {
            const ColorA &color = (mParams[kInput_Intensity]->floatValue > 0) ? mParams[kInput_EffectColor]->colorValue : ColorA::black();
            light->setEffectColor(mUuid, color);
            // Calculate the distance.
            float distance = 0;
            if (mChannel->getType() == InputChannel::Type::kType_Dim2) {
                distance = glm::distance(mChannel->getVec2Value(), vec2(light->position.x, light->position.z));
            }
            else if (mChannel->getType() == InputChannel::Type::kType_Dim3) {
                vec3 location = mChannel->getVec3Value();
                distance = glm::distance(location, vec3(light->position.x, light->position.y, light->position.z));
            }
            // TODO: This code is used more than one. Reuse!
            float intensity = 0.f;
            if (distance < radius) {
                intensity = mParams[kInput_Intensity]->floatValue;
            }
            else {
                float distanceFromRadius = distance - radius;
                intensity = mParams[kInput_Intensity]->floatValue / math<float>::pow(1 + distanceFromRadius, dropOff);
            }
            float lastIntensity = light->getEffetcIntensity(mUuid);

            float lastIntensityDiminished = lastIntensity - stepSize;
            if (intensity < lastIntensityDiminished && stepSize > 0) {
                light->setEffectIntensity(mUuid, lastIntensityDiminished);
            }
            else {
                light->setEffectIntensity(mUuid, intensity);
            }
        }
    }
}

void photonic::LocationFollow::init() {
    Effect::init();
}

std::string photonic::LocationFollow::getTypeName() {
    return "Location Follow";
}

std::string photonic::LocationFollow::getTypeClassName() {
    return "LocationFollow";
}

REGISTER_TYPE(LocationFollow)