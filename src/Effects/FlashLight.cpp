//
// Created by Jur de Vries on 19/02/2018.
//

#include "FlashLight.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

FlashLight::FlashLight(std::string name, std::string uuid)
: Effect(name, uuid), mEyeLocation(vec3(0.0f)), mViewDirection(vec3(0.f))
{
    Parameter* radius = new Parameter(Parameter::Type::kType_Float, "Radius");
    radius->floatValue = 0.5f;
    mParams[kInput_Radius] = radius;

    Parameter* dropOff = new Parameter(Parameter::Type::kType_Float, "Drop Off");
    dropOff->floatValue = 0.5f;
    mParams[kInput_DropOff] = dropOff;

    Parameter* effectColor = new Parameter(Parameter::Type::kType_Color, "Effect Color");
    effectColor->colorValue = ColorA(Color::gray(0.8f));
    mParams[kInput_EffectColor] = effectColor;

    Parameter* intensity = new Parameter(Parameter::Type::kType_Float, "Intensity");
    intensity->floatValue = 1.0f;
    mParams[kInput_Intensity] = intensity;

    Parameter* eyeLocation = new Parameter(Parameter::Type::kType_Channel, "Eye location");
    eyeLocation->channelRef = nullptr;
    mParams[kInput_EyeLocation] = eyeLocation;

    Parameter* viewDirection = new Parameter(Parameter::Type::kType_Channel, "View direction");
    viewDirection->channelRef = nullptr;
    mParams[kInput_ViewDirection] = viewDirection;

}

void FlashLight::execute(double dt) {
    Effect::execute(dt);

}

std::string FlashLight::getTypeName() {
    return "Flash light";
}

std::string FlashLight::getTypeClassName() {
    return "FlashLight";
}

REGISTER_TYPE(FlashLight)
