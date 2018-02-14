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
}

void photonic::LocationFollow::execute(double dt) {
    float radius = mParams[kInput_Radius]->floatValue;
    app::console() << mChannel->getVec2Value() << std::endl;
    Effect::execute(dt);
    if (mChannel) {
        for (auto light: mLights) {
            const ColorA &color = (mParams[kInput_Intensity]->floatValue > 0) ? mParams[kInput_EffectColor]->colorValue : ColorA::black();
            light->setEffectColor(mUuid, color);
            // Calculate the distance.
            float distance = 0;
            if (mChannel->getType() == InputChannel::Type::kType_Dim2) {
                distance = glm::distance(mChannel->getVec2Value(), vec2(light->position.x, light->position.z));
            }
            if (mChannel->getType() == InputChannel::Type::kType_Dim3) {
                distance = glm::distance(mChannel->getVec3Value(), vec3(light->position.x, light->position.y, light->position.z));
            }
            float power = math<float>::pow(distance, radius);
            if (power < 1) power = 1;
            float intensity = mParams[kInput_Intensity]->floatValue / power;
            light->setEffectIntensity(mUuid, intensity);
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