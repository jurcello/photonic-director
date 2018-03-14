//
// Created by Jur de Vries on 14/03/2018.
//

#include "Desaturate.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

photonic::Desaturate::Desaturate(std::string name, std::string uuid)
: Effect(name, uuid)
{
   registerParam(Parameter::Type::kType_Channel_MinMax, kInput_InputChannel, vec4(0.0f, 1.0f, 0.0f, 1.0f), "Input channel");
}

void photonic::Desaturate::execute(double dt) {
    Effect::execute(dt);
    if (mParams[kInput_InputChannel]->channelRef) {
        float desatuateValue = 1.0f - mParams[kInput_InputChannel]->getMappedChannelValue() * mFadeValue;
        for (const auto &light : mLights) {
            auto colorHsv = light->color.get(CM_HSV);
            colorHsv.y = desatuateValue;
            light->color.set(CM_HSV, vec4(colorHsv, 1.0f));
        }
    }
}

std::string photonic::Desaturate::getTypeName() {
    return "Desaturate";
}

std::string photonic::Desaturate::getTypeClassName() {
    return "Desaturate";
}

photonic::Effect::Stage photonic::Desaturate::getStage() {
    return Effect::Stage::kStage_After;
}

REGISTER_TYPE(Desaturate)