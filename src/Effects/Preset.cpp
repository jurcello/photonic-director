//
// Created by Jur de Vries on 23/10/2018.
//

#include "Preset.h"


using namespace cinder;
using namespace photonic;
using namespace ci;

Preset::Preset(std::string name, std::string uuid)
: Effect(name, uuid)
{
}

void Preset::execute(double dt) {
    Effect::execute(dt);
    if (mLightControls.size() !=  mLights.size()) {
        repopulateLightControls();
    }
    for (const auto &control: mLightControls) {
        if (control.light->isColorEnabled()) {
            control.light->setEffectColor(mUuid, control.color);
        }
        control.light->setEffectIntensity(mUuid, control.intensity);
    }
}

void Preset::drawEditGui() {
    int id = 0;
    for (auto &control: mLightControls) {
        ui::PushID(id);
        ui::Text("%s", control.light->mName.c_str());
        ui::SameLine();
        ui::SliderFloat("Intensity", &control.intensity, 0.0f, 1.0f);
        if (control.light->isColorEnabled()) {
            ui::ColorEdit4("", &control.color[0]);
            ui::PopID();
            id++;
        }
    }
}

void Preset::repopulateLightControls() {
    std::vector<LightControl> oldControls = mLightControls;
    mLightControls.clear();
    for (const auto light: mLights) {
        auto found = std::find_if(oldControls.begin(), oldControls.end(), [light](LightControl c) { return c.light->getUuid() == light->getUuid();});
        if (found != oldControls.end()) {
            mLightControls.push_back(*found);
        }
        else {
            mLightControls.emplace_back(LightControl(light, ColorA::black(), 0.0f));
        }
    }
}

std::string Preset::getTypeName() {
    return "Preset";
}

std::string Preset::getTypeClassName() {
    return "Preset";
}

REGISTER_TYPE(Preset)