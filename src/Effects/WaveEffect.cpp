//
// Created by Jur de Vries on 12/02/2018.
//

#include "WaveEffect.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

WaveEffect::WaveEffect(std::string name, std::string uuid)
        :Effect(name, uuid), mPlaneNormal(vec3(1.f, 0.f, 0.f))
{
    // Create the inputs.
    registerParam(Parameter::Type::kType_Color, kInput_BaseColor, ColorA(Color::gray(0.5f)), "Base Color");

    registerParam(Parameter::Type::kType_Color, kInput_EffectColor, ColorA(Color::gray(0.5f)), "Effect Color");

    registerParam(Parameter::Type::kType_Vector3, kInput_StartPoint, vec3(-3.0f, 0.0f, 0.0f), "Start Point");

    registerParam(Parameter::Type::kType_Vector3, kInput_EndPoint, vec3(-3.0f, 0.0f, 0.0f), "End Point");

    registerParam(Parameter::Type::kType_Vector3, kInput_Direction, vec3(1.0f, 0.0f, 0.0f), "Plane Orientation (Normal)");

    registerParam(Parameter::Type::kType_Float, kInput_Width, 0.5f, "Width");

    registerParam(Parameter::Type::kType_Float, kInput_Speed, 0.5f, "Speed");

    registerParam(Parameter::Type::kType_Float, kInput_NoiseAmount, 0.2f, "Noise Amount");

    registerParam(Parameter::Type::kType_Float, kInput_NoiseSpeed, 1.0f, "Noise Speed");
}

void WaveEffect::init() {
    Effect::init();
    mLastPosition = mParams[kInput_StartPoint]->vec3Value;
    mPerlin.setSeed(clock());
    mTimer.start();
}

void WaveEffect::execute(double dt) {
    Effect::execute(dt);
    mPlaneNormal = glm::normalize(mParams[kInput_Direction]->vec3Value);
    // Update the current position of the wave.
    auto elapsedTime = (float) mTimer.getSeconds();
    vec3 currentWavePosition = getCurrentWavePosition(dt);
    if (isTurnedOn) {
        for (const auto &light : mLights) {
            double distanceToWave = getDistanceToWave(currentWavePosition, light->getPosition());
            double intensity = getBellIntensity(distanceToWave, mParams[kInput_Width]->floatValue);
            // Add some perlin noise to the wave to have a more wave like appearance.
            float noise = mPerlin.noise(light->getPosition().x, light->getPosition().y, elapsedTime * mParams[kInput_NoiseSpeed]->floatValue / 10.f) * mParams[kInput_NoiseAmount]->floatValue;
            intensity += noise;
            bool baseIsBlack = mParams[kInput_BaseColor]->colorValue.r < 0.05f && mParams[kInput_BaseColor]->colorValue.g < 0.05f && mParams[kInput_BaseColor]->colorValue.b < 0.05f;
            ColorA color = interPolateColors(mParams[kInput_BaseColor]->colorValue, mParams[kInput_EffectColor]->colorValue, intensity);
            light->setEffectColor(mUuid, color);
            auto lightIntensity = (float) (light->isColorEnabled() && ! baseIsBlack ? 1.0f : intensity);
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