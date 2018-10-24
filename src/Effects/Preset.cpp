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

std::vector<LightControl> Preset::getLightControls() {
    return mLightControls;
}

void Preset::setLightControls(std::vector<LightControl> controls) {
    mLightControls = controls;
}

EffectXmlSerializerRef Preset::getXmlSerializer() {
    return EffectXmlSerializerRef(new PresetXmlSerializer(this));
}

std::string Preset::getTypeName() {
    return "Preset";
}

std::string Preset::getTypeClassName() {
    return "Preset";
}

PresetXmlSerializer::PresetXmlSerializer(Effect *cEffect)
: EffectXmlSerializer(cEffect)
{
}

void PresetXmlSerializer::writeEffect(XmlTree &xmlNode) {
    EffectXmlSerializer::writeEffect(xmlNode);
    auto effect = (Preset*) mEffect;
    auto controls = effect->getLightControls();
    if (! controls.empty()) {
        XmlTree parentNode;
        parentNode.setTag("fixtureControls");
        for (auto &control : controls) {
            XmlTree controlNode;
            controlNode.setTag("fixtureControl");
            controlNode.setAttribute("lightUuid", control.light->getUuid());
            controlNode.setAttribute("intensity", control.intensity);
            if (control.light->isColorEnabled()) {
                controlNode.setAttribute("r", control.color.r);
                controlNode.setAttribute("g", control.color.g);
                controlNode.setAttribute("b", control.color.b);
                controlNode.setAttribute("a", control.color.a);
            }
            parentNode.push_back(controlNode);
        }
        xmlNode.push_back(parentNode);
    }

}

void PresetXmlSerializer::readEffect(XmlTree &xmlNode, const std::vector<LightRef> &lights,
                                     std::vector<InputChannelRef> &channels) {
    EffectXmlSerializer::readEffect(xmlNode, lights, channels);
    if (xmlNode.hasChild("fixtureControls")) {
        auto effect = (Preset*) mEffect;
        auto controls = effect->getLightControls();
        for (auto &controlNode: xmlNode.getChild("fixtureControls").getChildren()) {
            std::string lightUuid = controlNode->getAttributeValue<std::string>("lightUuid");
            auto it = std::find_if(lights.begin(), lights.end(), [&lightUuid](const LightRef lightCandidate){ return lightCandidate->getUuid() == lightUuid;});
            if (it != lights.end()) {
                LightRef light = *it;
                float intensity = controlNode->getAttributeValue<float>("intensity");
                ColorA color = Color::black();
                if (light->isColorEnabled()) {
                    float r = controlNode->getAttributeValue<float>("r");
                    float g = controlNode->getAttributeValue<float>("g");
                    float b = controlNode->getAttributeValue<float>("b");
                    float a = controlNode->getAttributeValue<float>("a");
                    color = ColorA(r, g, b, a);
                }
                controls.push_back(LightControl(light, color, intensity));
            }
        }
        effect->setLightControls(controls);
    }
}


REGISTER_TYPE(Preset)