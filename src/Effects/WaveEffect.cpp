//
// Created by Jur de Vries on 12/02/2018.
//

#include "WaveEffect.h"
#include "../Effects.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

WaveEffect::WaveEffect(std::string name, std::string uuid)
        :Effect(name, uuid), mPlaneNormal(vec3(1.f, 0.f, 0.f))
{
    // Create the inputs.
    Parameter* baseColor = new Parameter(Parameter::Type::kType_Color, "Base Color");
    baseColor->colorValue = ColorA(Color::gray(0.5f));
    mParams[kInput_BaseColor] = baseColor;

    Parameter* effectColor = new Parameter(Parameter::Type::kType_Color, "Effect Color");
    effectColor->colorValue = ColorA(Color::gray(0.5f));
    mParams[kInput_EffectColor] = effectColor;

    Parameter* startPoint = new Parameter(Parameter::Type::kType_Vector3, "Start Point");
    startPoint->vec3Value = vec3(-3.0f, 0.0f, 0.0f);
    mParams[kInput_StartPoint] = startPoint;

    Parameter* endPoint = new Parameter(Parameter::Type::kType_Vector3, "End Point");
    endPoint->vec3Value = vec3(-3.0f, 0.0f, 0.0f);
    mParams[kInput_EndPoint] = endPoint;

    Parameter* direction = new Parameter(Parameter::Type::kType_Vector3, "Plane Orientation (Normal)");
    direction->vec3Value = vec3(1.0f, 0.0f, 0.0f);
    mParams[kInput_Direction] = direction;

    Parameter* width = new Parameter(Parameter::Type::kType_Float, "Width");
    width->floatValue = 0.5f;
    mParams[kInput_Width] = width;

    Parameter* speed = new Parameter(Parameter::Type::kType_Float, "Speed");
    speed->floatValue = 0.5f;
    mParams[kInput_Speed] = speed;
}

void WaveEffect::init() {
    Effect::init();
    mLastPosition = mParams[kInput_StartPoint]->vec3Value;
}

void WaveEffect::execute(double dt) {
    Effect::execute(dt);
    mPlaneNormal = glm::normalize(mParams[kInput_Direction]->vec3Value);
    // Update the current position of the wave.
    vec3 currentWavePosition = getCurrentWavePosition(dt);
    if (isTurnedOn) {
        for (const auto light : mLights) {
            double distanceToWave = getDistanceToWave(currentWavePosition, light->getPosition());
            double intensity = getBellIntensity(distanceToWave, mParams[kInput_Width]->floatValue);
            ColorA color = interPolateColors(mParams[kInput_BaseColor]->colorValue, mParams[kInput_EffectColor]->colorValue, intensity);
            light->setEffectColor(mUuid, color);
            auto lightIntensity = (float) (light->isColorEnabled() ? 1.0f : intensity);
            light->setEffectIntensity(mUuid, lightIntensity);
        }
    }
    mLastPosition = currentWavePosition;
}

std::string WaveEffect::getTypeName() {
    return "Wave";
}

std::string WaveEffect::getTypeClassName() {
    return "WaveEffect";
}

ColorA WaveEffect::interPolateColors(ColorA color1, ColorA color2, double intensity) {
    float r = math<float>::max(0.0f, math<float>::min(color1.r + (color2.r - color1.r) * intensity, 1.0f));
    float g = math<float>::max(0.0f, math<float>::min(color1.g + (color2.g - color1.g) * intensity, 1.0f));
    float b = math<float>::max(0.0f, math<float>::min(color1.b + (color2.b - color1.b) * intensity, 1.0f));
    // TODO: Check why this much simpler implementation fails.
    // ColorA newColor = intensity * color1 + (1.0 - intensity) * color2;
    return ColorA(r, g, b);
}

double WaveEffect::getDistanceToWave(vec3 wavePosition, vec3 position) {
    vec3 distanceVector = wavePosition - position;
    float distance = glm::dot(distanceVector, mPlaneNormal);
    return math<float>::abs(distance);
}

vec3 WaveEffect::getCurrentWavePosition(double dt) {
    // Plane movement calculations.
    double distance = dt * mParams[kInput_Speed]->floatValue;
    vec3 movePathVector = mParams[kInput_EndPoint]->vec3Value - mParams[kInput_StartPoint]->vec3Value;
    // Now add this distance to the lastPosition.
    vec3 moveDirectionNormal = glm::normalize(movePathVector);
    vec3 currentPosition = mLastPosition + moveDirectionNormal * (float) distance;
    if (glm::length(currentPosition - mParams[kInput_StartPoint]->vec3Value) > glm::length(movePathVector)) {
        currentPosition -= movePathVector;
    }
    return currentPosition;
}

double WaveEffect::getBellIntensity(double distance, double width) {
//    double relIntensity = Math.exp(-(distance)/(2*Math.pow(sigma, 2)));
    return math<double>::exp(-(distance)/(2*(math<double>::pow(mParams[kInput_Width]->floatValue, 2))));
}

REGISTER_TYPE(WaveEffect)