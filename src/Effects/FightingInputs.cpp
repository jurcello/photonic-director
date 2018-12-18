//
// Created by Jur de Vries on 13/12/2018.
//

#include "FightingInputs.h"

using namespace cinder;
using namespace photonic;
using namespace ci;

FightingInputs::FightingInputs(std::string name, std::string uuid)
: Effect(name, uuid), mVictimRadius(0.0f), mVictimOwnLampIntensityFactor(1.f)
{
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_VictimChannel, vec4(0.0f, 1.0f, 0.0f, 1.0f), "Victim channel");
    registerParam(Parameter::Type::kType_Channel_MinMax, kInput_AttackerChannel, vec4(0.0f, 1.0f, 0.0f, 1.0f), "Attacker channel");
    registerParam(Parameter::Type::kType_Color, kInput_VictimColor, ColorA(Color::gray(0.5f)), "Victim Color");
    registerParam(Parameter::Type::kType_Color, kInput_AttackerColor, ColorA(Color::gray(0.5f)), "Attacker Color");
    registerParam(Parameter::Type::kType_Light, kInput_VictimLamp, "Victim lamp");
    registerParam(Parameter::Type::kType_Float, kInput_VictimIncreaseSpeed, 1.f, "Victim increase speed");
    registerParam(Parameter::Type::kType_Float, kInput_AttackerDecreaseSpeed, 1.f, "Attacker descrease speed");
    registerParam(Parameter::Type::kType_Float, kInput_AttackerStartRadius, 1.f, "Attacker start radius");
    registerParam(Parameter::Type::kType_Float, kInput_DropOff, 1.0f, "Dropoff");
}

void FightingInputs::init() {
    mAttackerRadius = mParams[kInput_AttackerStartRadius]->floatValue;
    mVictimRadius = 0.f;
    mVictimOwnLampIntensityFactor = 1.0f;
}

void FightingInputs::execute(double dt) {
    Effect::execute(dt);
    if (mParams[kInput_VictimLamp]->lightRef == nullptr) {
        return;
    }
    float dropOff = mParams[kInput_DropOff]->floatValue;
    UpdateRadius(dt);
    updateVictimLamp();

    for (const auto light : mLights) {
        if (light->getUuid() != mParams[kInput_VictimLamp]->lightRef->getUuid()) {
            float victimIntensity = 0;
            float attackerIntensity = 0;
            float distance = glm::distance(mParams[kInput_VictimLamp]->lightRef->getPosition(), light->getPosition());

            // Victim
            if (distance < mVictimRadius ) {
                victimIntensity = mParams[kInput_VictimChannel]->getMappedChannelValue();
            }
            else {
                float distanceFromRadius = mVictimRadius - distance;
                victimIntensity = mParams[kInput_VictimChannel]->getMappedChannelValue() / math<float>::pow(1 + distanceFromRadius, dropOff);
            }

            // Attacker
            if (distance > mAttackerRadius ) {
                attackerIntensity = mParams[kInput_AttackerChannel]->getMappedChannelValue();
            }
            else {
                float distanceFromRadius = mAttackerRadius - distance;
                attackerIntensity = mParams[kInput_AttackerChannel]->getMappedChannelValue() / math<float>::pow(1 + distanceFromRadius, dropOff);
            }

            // The attacker diminshes the victim intensity.
            victimIntensity -= attackerIntensity;
            if (victimIntensity < 0) {
                victimIntensity = 0;
            }

            // Set the color.
            Color victimColor = mParams[kInput_VictimColor]->colorValue * victimIntensity;
            Color attackerColor = mParams[kInput_AttackerColor]->colorValue * attackerIntensity;
            ColorA finalColor = interPolateColors(victimColor, attackerColor, attackerIntensity);
            light->setEffectColor(mUuid, finalColor);

            // Set the final intensity.
            float finalIntensity = math<float>::max(attackerIntensity + victimIntensity, 1.f);
            light->setEffectIntensity(mUuid, finalIntensity);
        }
    }
}

void FightingInputs::updateVictimLamp() {
    float negativeRadiusCausingZero = 3.f;
    if (mAttackerRadius < 0) {
        // Note that in these calculations the attacker radius is smaller than zero.
        mVictimOwnLampIntensityFactor = (negativeRadiusCausingZero + mAttackerRadius) / negativeRadiusCausingZero;
        if (mVictimOwnLampIntensityFactor < 0) {
            mVictimOwnLampIntensityFactor = 0;
        }
    }
    if (mParams[kInput_VictimLamp]->lightRef->isColorEnabled()) {
        mParams[kInput_VictimLamp]->lightRef->setEffectColor(mUuid, mParams[kInput_VictimColor]->colorValue * mVictimOwnLampIntensityFactor);
    }
    mVictimLampIntensity = mVictimOwnLampIntensityFactor * mParams[kInput_VictimChannel]->getMappedChannelValue();
    mParams[kInput_VictimLamp]->lightRef->setEffectIntensity(mUuid, mVictimLampIntensity);

}

void FightingInputs::UpdateRadius(double dt) {
    mAttackerRadius -= mParams[kInput_AttackerDecreaseSpeed]->floatValue * dt * mParams[kInput_AttackerChannel]->getMappedChannelValue();
    if (mAttackerRadius < mVictimRadius) {
        mVictimRadius = mAttackerRadius;
        return;
    }
    mVictimRadius += mParams[kInput_VictimIncreaseSpeed]->floatValue * dt * mParams[kInput_VictimChannel]->getMappedChannelValue();
}


void FightingInputs::drawEditGui() {
    ui::InputFloat("Victim radius", &mVictimRadius);
    ui::InputFloat("Attacker radius", &mAttackerRadius);
    ui::InputFloat("Victim lamp intensity", &mVictimLampIntensity);
    ui::InputFloat("Victim intensity factor", &mVictimOwnLampIntensityFactor);
}

std::string FightingInputs::getTypeName() {
    return "Fighting Inputs";
}

std::string FightingInputs::getTypeClassName() {
    return "FightingInputs";
}

REGISTER_TYPE(FightingInputs)