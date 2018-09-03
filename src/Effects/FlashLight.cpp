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
    dropOff->floatValue = 3.0f;
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
    if (mParams[kInput_EyeLocation]->channelRef != nullptr && mParams[kInput_ViewDirection]->channelRef != nullptr) {
        auto viewDirectionChannel = mParams[kInput_ViewDirection]->channelRef;
        auto eyePositionChannel = mParams[kInput_EyeLocation]->channelRef;
        vec3 directionNormal = glm::normalize(viewDirectionChannel->getVec3Value());
        vec3 eyeLoc = eyePositionChannel->getVec3Value();

        // Get the radius and the dropOff value.
        float radius = mParams[kInput_Radius]->floatValue;
        float dropOff = mParams[kInput_DropOff]->floatValue;
        // If the channel receives 2D info, override the values.
        if (eyePositionChannel->getType() == InputChannel::Type::kType_Dim2) {
            vec2 viewDirIn = viewDirectionChannel->getVec2Value();
            directionNormal = glm::normalize(vec3(viewDirIn.x, 0.0f, viewDirIn.y));
            vec2 eyeLocIn = eyePositionChannel->getVec2Value();
            eyeLoc = vec3(eyeLocIn.x, 0.0f, eyeLocIn.y);
        }
        mEyeLocation = eyeLoc;
        mViewDirection = directionNormal;

        if (isTurnedOn) {
            for (const auto light : mLights) {
                float intensity = 0;
                if (glm::dot(light->getPosition() - mEyeLocation, mViewDirection) > 0) {
                    float distanceToLine = calculateDistanceToLine(light->getPosition(), mEyeLocation, mViewDirection);
                    float distanceToEye = calculateDistanceToEye(light->getPosition(), mEyeLocation, mViewDirection);

                    float radiusForLight = distanceToEye * radius;
                    if (distanceToLine < radiusForLight) {
                        intensity = mParams[kInput_Intensity]->floatValue;
                    }
                    else {
                        float distanceFromRadius = distanceToLine - radiusForLight;
                        intensity = mParams[kInput_Intensity]->floatValue / math<float>::pow(1 + distanceFromRadius, dropOff);
                    }
                }

                light->setEffectIntensity(mUuid, intensity);
                light->setEffectColor(mUuid, mParams[kInput_EffectColor]->colorValue * intensity);
            }
        }
    }
}

float FlashLight::calculateDistanceToLine(vec3 itemPosition, vec3 eyePosition, vec3 direction) {
    vec3 eyeLightVec = itemPosition - eyePosition;
    vec3 normalProjection = glm::dot(eyeLightVec, direction) * direction;
    return glm::length(eyeLightVec - normalProjection);
}

float FlashLight::calculateDistanceToEye(vec3 itemPosition, vec3 eyePosition, vec3 direction) {
    vec3 eyeLightVec = itemPosition - eyePosition;
    return glm::dot(eyeLightVec, direction);
}

vec3 FlashLight::getNearestPointOnLine(vec3 itemPosition, vec3 eyePosition, vec3 direction) {
    vec3 eyeLightVec = itemPosition - eyePosition;
    vec3 normalProjection = glm::dot(eyeLightVec, direction) * direction;
    return (eyePosition + normalProjection);
}

void FlashLight::visualize() {
    Effect::visualize();
    if (mParams[kInput_EyeLocation]->channelRef != nullptr && mParams[kInput_ViewDirection]->channelRef != nullptr) {
        vec3 start = mEyeLocation;
        vec3 end = start + mViewDirection * 100.0f;
        auto colorShader = gl::ShaderDef().color();
        auto shader = gl::getStockShader( colorShader );
        shader->bind();
        gl::ScopedLineWidth width(10.0f);
        gl::ScopedColor color(1.0f, 1.0f, 0.0f, 0.5f);
        gl::drawLine(start, end);
        gl::drawSphere(mEyeLocation, 0.05f);
        for (const auto &light : mLights) {
            float radiusForLight = calculateDistanceToEye(light->getPosition(), mEyeLocation, mViewDirection) * mParams[kInput_Radius]->floatValue;
            vec3 pointOnLine = getNearestPointOnLine(light->getPosition(),  mEyeLocation, mViewDirection);
            vec3 coneReachPoint = pointOnLine - radiusForLight * (pointOnLine - light->getPosition()) / glm::length(pointOnLine - light->getPosition());
            gl::drawLine(pointOnLine, light->getPosition());
            gl::ScopedColor color2(1.0f, 0.0f, 0.0f, 0.5f);
            gl::drawLine(pointOnLine, coneReachPoint);
        }
    }
}

std::string FlashLight::getTypeName() {
    return "Flash light";
}

std::string FlashLight::getTypeClassName() {
    return "FlashLight";
}

REGISTER_TYPE(FlashLight)
