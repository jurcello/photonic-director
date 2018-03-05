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

    Parameter* noiseAmount = new Parameter(Parameter::Type::kType_Float, "Noise Amount");
    noiseAmount->floatValue = 0.2f;
    mParams[kInput_NoiseAmount] = noiseAmount;

    Parameter* noiseSpeed = new Parameter(Parameter::Type::kType_Float, "Noice Speed");
    noiseSpeed->floatValue = 1.0f;
    mParams[kInput_NoiseSpeed] = noiseSpeed;
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
    float elapsedTime = (float) mTimer.getSeconds();
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