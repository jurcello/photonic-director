//
// Created by Jur de Vries on 22/02/2018.
//

#include "VolumeLocation.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

photonic::VolumeLocation::VolumeLocation(std::string name, std::string uuid)
: Effect(name, uuid), mLocation(vec3(1.0f)), mStaticLocation(vec3(1.0f))
{

    Parameter* intensity = new Parameter(Parameter::Type::kType_Float, "Intensity");
    intensity->floatValue = 1.0f;
    mParams[kInput_Intensity] = intensity;

    Parameter* dropOff = new Parameter(Parameter::Type::kType_Float, "Drop Off");
    dropOff->floatValue = 3.0f;
    mParams[kInput_DropOff] = dropOff;

    Parameter* radius = new Parameter(Parameter::Type::kType_Float, "Radius");
    radius->floatValue = 0.5f;
    mParams[kInput_Radius] = radius;

    Parameter* location = new Parameter(Parameter::Type::kType_Channel, "Location channel");
    location->channelRef = nullptr;
    mParams[kInput_LocationChannel] = location;

    Parameter* baseColor = new Parameter(Parameter::Type::kType_Color, "Base Color");
    baseColor->colorValue = ColorA(Color::gray(0.8f));
    mParams[kInput_BaseColor] = baseColor;

    Parameter* effectColor = new Parameter(Parameter::Type::kType_Color, "Effect Color");
    effectColor->colorValue = ColorA(Color::gray(0.8f));
    mParams[kInput_EffectColor] = effectColor;

    Parameter* triggerChannel = new Parameter(Parameter::Type::kType_OscTrigger, "Location fixed");
    triggerChannel->triggerValue = false;
    triggerChannel->oscAdress = "/trigger";
    mParams[kInput_TriggerChannel] = triggerChannel;

}

void photonic::VolumeLocation::execute(double dt) {
    Effect::execute(dt);
    if (mParams[kInput_LocationChannel]->channelRef != nullptr && mChannel != nullptr) {
        // First read the location.
        auto locationChannel = mParams[kInput_LocationChannel]->channelRef;
        mLocation = locationChannel->getVec3Value();
        // Override the location if only 2 dimensional info is transferred.
        if (locationChannel->getType() == InputChannel::Type::kType_Dim2) {
            mLocation = vec3(locationChannel->getVec2Value().x, 1.0f, locationChannel->getVec2Value().y);
        }
        // Update the static location.
        if (mParams[kInput_TriggerChannel]->triggerValue) {
            mLocation = mStaticLocation;
        }
        else {
            mStaticLocation = mLocation;
        }

        float dropOff = mParams[kInput_DropOff]->floatValue;
        float radius = mParams[kInput_Radius]->floatValue;

        for (const auto light : mLights) {
            float intensity = 0.f;
            float distance = glm::distance(light->getPosition(), mLocation);
            // TODO: This code is used more than one. Reuse!
            if (distance < radius) {
                intensity = mParams[kInput_Intensity]->floatValue * mChannel->getValue();
            }
            else {
                float distanceFromRadius = distance - radius;
                intensity = (mParams[kInput_Intensity]->floatValue * mChannel->getValue()) / math<float>::pow(1 + distanceFromRadius, dropOff);
            }
            ColorA color = interPolateColors(mParams[kInput_BaseColor]->colorValue, mParams[kInput_EffectColor]->colorValue, intensity);
            light->setEffectIntensity(mUuid, intensity);
            light->setEffectColor(mUuid, color);
        }
    }
}

void VolumeLocation::visualize() {
    Effect::visualize();
    if (mParams[kInput_LocationChannel]->channelRef != nullptr) {
        auto colorShader = gl::ShaderDef().color();
        auto shader = gl::getStockShader( colorShader );
        shader->bind();
        gl::ScopedLineWidth width(10.0f);
        gl::ScopedColor color(1.0f, 1.0f, 0.0f, 0.5f);
        gl::drawSphere(mLocation, 0.05f);
    }
}

std::string photonic::VolumeLocation::getTypeName() {
    return "Volume on location";
}

std::string photonic::VolumeLocation::getTypeClassName() {
    return "VolumeLocation";
}

REGISTER_TYPE(VolumeLocation)
