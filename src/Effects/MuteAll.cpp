//
// Created by Jur de Vries on 13/03/2018.
//

#include "MuteAll.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

photonic::MuteAll::MuteAll(std::string name, std::string uuid)
:Effect(name, uuid)
{
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_Input, vec4(0.0f, 1.0f, 0.0f, 1.0f), "Input channel");
    registerParam(Parameter::Type::kType_Channel, kInput_ExportChannel, "Export channel");
    registerParam(Parameter::Type::kType_Float, kInput_OffTreshold, 0.8f, "Threshold for off");
}

void photonic::MuteAll::execute(double dt) {
    Effect::execute(dt);
    if (mParams[kInput_Input]->channelRef) {
        float value = mParams[kInput_Input]->getMappedChannelValue();
        if (value > mParams[kInput_OffTreshold]->floatValue) {
            // Set the intensity to zero and the color to black.
            const double inverseFadeValue = 1.0f - mFadeValue;
            for (const auto &light : mLights) {
                light->intensity *= inverseFadeValue;
                light->color *= inverseFadeValue;
            }
        }
        // Set the output channel to the fadeValue.
        if (mParams[kInput_ExportChannel]->channelRef) {
            mParams[kInput_ExportChannel]->channelRef->setValue(mFadeValue);
        }

    }
}

Effect::Stage MuteAll::getStage() {
    return Effect::Stage::kStage_After;
}

std::string photonic::MuteAll::getTypeName() {
    return "Mute";
}

std::string photonic::MuteAll::getTypeClassName() {
    return "MuteAll";
}

REGISTER_TYPE(MuteAll)