//
// Created by Jur de Vries on 13/09/2018.
//

#include "MovingHead.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

MovingHead::MovingHead(std::string name, std::string uuid) : Effect(name, uuid) {
    registerParam(Parameter::Type::kType_Float, kInput_Intensity, 1.0f, "Intensity");
    registerParam(Parameter::Type::kType_Float, kInput_Pan, 1.0f, "Pan");
    registerParam(Parameter::Type::kType_Float, kInput_Tilt, 1.0f, "Tilt");
    registerParam(Parameter::Type::kType_Float, kInput_Focus, 1.0f, "Focus");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_IntensityChannel, vec4(0.0f, 1.0f, 0.0f, 1.0f), "Intensity channel");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_PanChannel, vec4(0.0f, 360.0f, 0.0f, 360.0f), "Pan channel");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_TiltChannel, vec4(-90.0f, 90.0f, 0.0f, 360.0f), "Tilt channel");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_FocusChannel, vec4(0.0f, 1.0f, 0.0f, 1.0f), "Tilt channel");
}

bool MovingHead::supportsLight(LightRef light) {
    auto component = light->getComponentById("tilt");
    if (component == nullptr) {
        return false;
    }
    return true;
}

void MovingHead::execute(double dt) {
    Effect::execute(dt);
    if (mParams[kInput_IntensityChannel]->channelRef) {
        mParams[kInput_Intensity]->floatValue = mParams[kInput_IntensityChannel]->getMappedChannelValue();
    }
    if (mParams[kInput_PanChannel]->channelRef) {
        mParams[kInput_Pan]->floatValue = mParams[kInput_PanChannel]->getMappedChannelValue();
    }
    if (mParams[kInput_TiltChannel]->channelRef) {
        mParams[kInput_Tilt]->floatValue = mParams[kInput_TiltChannel]->getMappedChannelValue();
    }
    if (mParams[kInput_FocusChannel]->channelRef) {
        mParams[kInput_Focus]->floatValue = mParams[kInput_FocusChannel]->getMappedChannelValue();
    }
    float intensity = mParams[kInput_Intensity]->floatValue;
    float pan = mParams[kInput_Pan]->floatValue;
    float tilt = mParams[kInput_Tilt]->floatValue;
    float focus = mParams[kInput_Focus]->floatValue;
    for (const auto light: mLights) {
        light->setEffectIntensity(mUuid, intensity);
        if (this->isTurnedOn) {
            auto panComponent = light->getComponent<PanComponent>();
            auto tiltComponent = light->getComponent<TiltComponent>();
            auto focusComponent = light->getComponentById<ChannelComponent>("focus");
            if (panComponent) {
                panComponent->setPanning(pan);
                panComponent->controlledBy = mName;
            }
            if (tiltComponent) {
                tiltComponent->setTilt(tilt);
                tiltComponent->controlledBy = mName;
            }
            if (focusComponent) {
                focusComponent->setValue(focus);
                focusComponent->controlledBy = mName;
            }
        }
    }
}

std::string MovingHead::getTypeName() {
    return "MovingHead";
}

std::string MovingHead::getTypeClassName() {
    return "MovingHead";
}

REGISTER_TYPE(MovingHead)