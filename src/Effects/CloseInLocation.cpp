//
// Created by Jur de Vries on 04/05/2018.
//

#include "CloseInLocation.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

photonic::CloseInLocation::CloseInLocation(std::string name, std::string uuid)
: Effect(name, uuid), mCurrentRadius(10.0f), mTargetRadius(10.0f)
{
    registerParam(Parameter::Type::kType_Vector3, kInput_Location, vec3(1.f), "Location");
    registerParam(Parameter::Type::kType_Channel, kInput_LocationChannel, "Location Channel");
    registerParam(Parameter::Type::kType_Float, kInput_Intensity, 1.0f, "Intensity");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_IntensityChannel, vec4(0.f, 1.0f, 0.0f, 1.f), "Intensity Channel");
    registerParam(Parameter::Type::kType_Float, kInput_DropOff, 1.0f, "Dropoff");
    registerParam(Parameter::Type::kType_Float, kInput_Radius, 3.0f, "Radius");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_RadiusChannel, vec4(0.0f, 1.0f, 0.0f, 2.0f), "Radius Channel");
    registerParam(Parameter::Type::kType_Color, kInput_EffectColor, vec3(0.5f), "Effect Color");
    registerParam(Parameter::Type::kType_Float, kInput_ForwardSpeed, 5.0f, "Forward speed");
    registerParam(Parameter::Type::kType_Float, kInput_RetreatSpeed, 1.0f, "Retreat speed");
}

void photonic::CloseInLocation::execute(double dt) {
    Effect::execute(dt);
    if (mParams[kInput_LocationChannel]->channelRef) {
        mParams[kInput_Location]->vec3Value = mParams[kInput_LocationChannel]->channelRef->getVec3Value();
    }
    if (mParams[kInput_IntensityChannel]->channelRef) {
        mParams[kInput_Intensity]->floatValue = mParams[kInput_IntensityChannel]->getMappedChannelValue();
    }
    if (mParams[kInput_RadiusChannel]->channelRef) {
        mParams[kInput_Radius]->floatValue = mParams[kInput_RadiusChannel]->getMappedChannelValue();
    }

    float dropOff = mParams[kInput_DropOff]->floatValue;

    updateRadius(dt);

    mLocation = mParams[kInput_Location]->vec3Value;

    for (const auto light : mLights) {
        float intensity = 0;
        float distance = glm::distance(light->getPosition(), mLocation);
        if (distance > mCurrentRadius) {
            intensity = mParams[kInput_Intensity]->floatValue;
        }
        else {
            float distanceFromRadius = mCurrentRadius - distance;
            intensity = mParams[kInput_Intensity]->floatValue / math<float>::pow(1 + distanceFromRadius, dropOff);
        }
        light->setEffectIntensity(mUuid, intensity);
        light->setEffectColor(mUuid, mParams[kInput_EffectColor]->colorValue * intensity);
    }
}

void CloseInLocation::updateRadius(double dt) {
    // Set the target radius.
    mTargetRadius = mParams[kInput_Radius]->floatValue;
    // Update the currentRadius.
    if (mCurrentRadius > mTargetRadius) {
        mCurrentRadius -= dt * mParams[kInput_ForwardSpeed]->floatValue;
        if (mCurrentRadius < mTargetRadius) {
            mCurrentRadius = mTargetRadius;
        }
    }
    else if (mCurrentRadius < mTargetRadius) {
        mCurrentRadius += dt * mParams[kInput_RetreatSpeed]->floatValue;
        if (mCurrentRadius > mTargetRadius) {
            mCurrentRadius = mTargetRadius;
        }
    }
    else {
        mCurrentRadius = mTargetRadius;
    }
}

std::string photonic::CloseInLocation::getTypeName() {
    return "Close in location";
}

std::string photonic::CloseInLocation::getTypeClassName() {
    return "CloseInLocation";
}

void photonic::CloseInLocation::visualize() {
    Effect::visualize();
}

REGISTER_TYPE(CloseInLocation)