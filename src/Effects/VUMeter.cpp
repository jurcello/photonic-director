//
// Created by  on 2019-03-09.
//

#include "VUMeter.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

VUMeter::VUMeter(std::string name, std::string uuid)
: CloseInLocation(name, uuid) {
}

void VUMeter::execute(double dt) {
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
        if (distance < mCurrentRadius) {
            intensity = mParams[kInput_Intensity]->floatValue;
        }
        else {
            float distanceFromRadius = math<float>::abs(mCurrentRadius - distance);
            intensity = mParams[kInput_Intensity]->floatValue / math<float>::pow(1 + distanceFromRadius, dropOff);
        }
        light->setEffectIntensity(mUuid, intensity);
        light->setEffectColor(mUuid, mParams[kInput_EffectColor]->colorValue * intensity);
    }
}


std::string VUMeter::getTypeName() {
    return "VUMeter";
}

std::string VUMeter::getTypeClassName() {
    return "VUMeter";
}


REGISTER_TYPE(VUMeter)