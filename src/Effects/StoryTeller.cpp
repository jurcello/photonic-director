//
// Created by Jur de Vries on 21/08/2018.
//

#include "StoryTeller.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

StoryTeller::StoryTeller(std::string name, std::string uuid)
: Effect(name, uuid),
mCurrentRotationLeft(0.0f),
mCurrentRotationRight(0.0f),
mCurrentIntensityLeft(0.0f),
mCurrentIntensityRight(0.0f),
mCenter(vec3(0.0f)),
mCenterCalculated(false),
mHasRotationLeft(false),
mHasRotationRight(false)
{
    Parameter* rotationRight = new Parameter(Parameter::Type::kType_Channel, "Rotation/volume right channel");
    rotationRight->channelRef = nullptr;
    mParams[kInput_RotationChannelRight] = rotationRight;

    Parameter* rotationLeft = new Parameter(Parameter::Type::kType_Channel, "Rotation/volume left channel");
    rotationLeft->channelRef = nullptr;
    mParams[kInput_RotationChannelLeft] = rotationLeft;

    registerParam(Parameter::Type::kType_Float, kInput_Intensity, 1.0f, "Intensity");
    registerParam(Parameter::Type::kType_Float, kInput_Width, 0.5f, "Width");
}

void StoryTeller::execute(double dt) {
    Effect::execute(dt);
    updateState();
    calculateCenter();
    updateCurrentRotationsAndVolumes();
    if (mLights.empty()) {
        return;
    }
    vec3 up(0.0f, 1.0f, 0.0f);


    for (const auto light: mLights) {
        float angle = angleBetween(vec3(light->position) - mCenter, up);
        float intensity = 0.0f;
        if (mHasRotationLeft) {
            intensity += mCurrentIntensityLeft * getBellIntensity((double) calculateAngleDistance(angle, mCurrentRotationLeft), (double) mParams[kInput_Width]->floatValue);
        }
        if (mHasRotationRight) {
            intensity += mCurrentIntensityRight * getBellIntensity((double) calculateAngleDistance(angle, mCurrentRotationRight), (double) mParams[kInput_Width]->floatValue);
        }
        light->setEffectIntensity(mUuid, intensity * mParams[kInput_Intensity]->floatValue);
    }

}

float StoryTeller::calculateAngleDistance(float angle1, float angle2) {
    float const rightMinAngle = glm::min(glm::abs(angle1 - angle2), glm::abs(angle1 + 360.0f - angle2));
    float const leftMinAngle = glm::min(glm::abs(angle1 - angle2), glm::abs(angle1 - 360.0f - angle2));
    return glm::min(rightMinAngle, leftMinAngle);
}

float StoryTeller::angleBetween(vec3 vector1, vec3 vector2) {
    glm::vec3 directionVector1 = glm::normalize(vector1);
    glm::vec3 directionVector2 = glm::normalize(vector2);
    const float dotProduct = glm::dot(directionVector1, directionVector2);
    const vec3 crossProduct = glm::cross(directionVector1, directionVector2);
    const float sign = glm::dot(crossProduct, mDirectionReference);
    float radians = glm::acos(dotProduct);
    if (sign > 0) {
        radians = -radians;
    }

    return ((radians * 180.0f) / glm::pi<float>()) + 180.0f;
}

void StoryTeller::init() {
    Effect::init();
}

void StoryTeller::drawEditGui() {
    ui::InputFloat3("Center: ", &mCenter[0]);
    ui::Separator();
    std::string currentRotationText = "Current rotation: " + std::to_string(mCurrentRotationLeft);
    ui::Text("%s", currentRotationText.c_str());
    vec3 up(0.0f, 1.0f, 0.0f);
    for (auto light: mLights) {
        float angle = angleBetween(vec3(light->position) - mCenter, up);
        const float angleDistance = calculateAngleDistance(angle, mCurrentRotationLeft);
        float intensity = getBellIntensity((double) angleDistance, (double) mParams[kInput_Width]->floatValue);
        std::string text = light->mName + ": " + std::to_string(angle) + ", angle distance: " + std::to_string(angleDistance) + ", intensity: " + std::to_string(intensity);
        ui::Text("%s", text.c_str());
    }
}

void StoryTeller::visualize() {
    auto colorShader = gl::ShaderDef().color();
    auto shader = gl::getStockShader( colorShader );
    shader->bind();
    gl::ScopedLineWidth width(1.0f);
    gl::ScopedColor color(1.0f, 1.0f, 0.0f, 0.5f);
    gl::drawSphere(mCenter, 0.05f);
}

void StoryTeller::updateState() {
    // Set center is calculated to false when the effect is off.
    // This way the center can be recalculated when the effect has been turned on.
    if (! isTurnedOn && mCenterCalculated) {
        mCenterCalculated = false;
    }
}

void StoryTeller::updateCurrentRotationsAndVolumes() {
    mHasRotationLeft = false;
    mHasRotationRight = false;
    if (mParams[kInput_RotationChannelLeft]->channelRef) {
        vec2 values = mParams[kInput_RotationChannelLeft]->channelRef->getVec2Value();
        mCurrentRotationLeft = values.x;
        mCurrentIntensityLeft = values.y;
        mHasRotationLeft = true;
    }
    if (mParams[kInput_RotationChannelRight]->channelRef) {
        vec2 values = mParams[kInput_RotationChannelRight]->channelRef->getVec2Value();
        mCurrentRotationRight = values.x;
        mCurrentIntensityRight = values.y;
        mHasRotationRight = true;
    }
}

void StoryTeller::calculateCenter() {
    if (mCenterCalculated) {
        return;
    }
    if (mLights.empty()) {
        return;
    }
    vec3 position(0.0f);
    for (const auto light: mLights) {
        vec3 lightPosition(light->position);
        position += lightPosition;
    }
    position /= mLights.size();
    mCenter = position;
    // Also create a direction vector from the first light.
    mDirectionReference = glm::cross(mCenter, vec3(mLights.front()->position) - mCenter);
    mCenterCalculated = true;

}

std::string StoryTeller::getTypeName() {
    return "StoryTeller";
}

std::string StoryTeller::getTypeClassName() {
    return "StoryTeller";
}

REGISTER_TYPE(StoryTeller)