
#include "MidiLight.h"

using namespace cinder;
using namespace photonic;
using namespace ci;


MidiLightInformation::MidiLightInformation(LightControl control, uint8_t note)
:lightcontrol(control), midiNote(note)
{
}

MidiLight::MidiLight(std::string name, std::string uuid)
: Effect(name, uuid) {
}

void MidiLight::execute(double dt) {
    Effect::execute(dt);
    if (mMidiLightInformation.size() !=  mLights.size()) {
        repopulateLightInformation();
    }
}

void MidiLight::listenToMidi(const smf::MidiMessage *message) {
    if (message->isNote()) {
        float intensity = 0.f;
        if (message->isNoteOn()) {
            app::console() << "Note: " << message->getKeyNumber() << std::endl;
            intensity = message->getVelocity() / 128.f;
        }
        else {
            intensity = 0.f;
        }
        for (auto &information: mMidiLightInformation) {
            if (message->getKeyNumber() == information.midiNote) {
                information.lightcontrol.light->setEffectIntensity(mUuid, intensity);
                if (information.lightcontrol.light->isColorEnabled()) {
                    information.lightcontrol.light->setEffectColor(mUuid, information.lightcontrol.color);
                }
            }
        }
    }
}

void MidiLight::drawEditGui() {
    int id = 0;
    for (auto &control: mMidiLightInformation) {
        ui::PushID(id);
        ui::Text("%s", control.lightcontrol.light->mName.c_str());
        ui::SameLine();
        if (control.lightcontrol.light->isColorEnabled()) {
            ui::ColorEdit4("", &control.lightcontrol.color[0]);
        }
        ui::InputInt("Midi note value", &control.midiNote);
        ui::PopID();
        id++;
    }
}

void MidiLight::repopulateLightInformation() {
    std::vector<MidiLightInformation> oldMidiInformation = mMidiLightInformation;
    mMidiLightInformation.clear();
    for (const auto light: mLights) {
        auto found = std::find_if(oldMidiInformation.begin(), oldMidiInformation.end(), [light](MidiLightInformation c) { return c.lightcontrol.light->getUuid() == light->getUuid();});
        if (found != oldMidiInformation.end()) {
            mMidiLightInformation.push_back(*found);
        }
        else {
            mMidiLightInformation.emplace_back(MidiLightInformation(LightControl(light, ColorA::black(), 0.0f), 0));
        }
    }
}

EffectXmlSerializerRef MidiLight::getXmlSerializer() {
    return EffectXmlSerializerRef(new MidiLightXmlSerializer(this));
}

std::vector<MidiLightInformation> MidiLight::getLightInformation() {
    return mMidiLightInformation;
}

void MidiLight::setLightInformation(std::vector<MidiLightInformation> lightInformation) {
    mMidiLightInformation = lightInformation;
}

std::string MidiLight::getTypeName() {
    return "MidiLight";
}

std::string MidiLight::getTypeClassName() {
    return "MidiLight";
}

MidiLightXmlSerializer::MidiLightXmlSerializer(Effect *cEffect) : EffectXmlSerializer(cEffect) {

}

void MidiLightXmlSerializer::writeEffect(XmlTree &xmlNode) {
    EffectXmlSerializer::writeEffect(xmlNode);
    auto effect = (MidiLight*) mEffect;
    auto lightInformationData = effect->getLightInformation();
    if (! lightInformationData.empty()) {
        XmlTree parentNode;
        parentNode.setTag("lightInformationData");
        for (auto &lightInformation : lightInformationData) {
            XmlTree controlNode;
            controlNode.setTag("lightInformation");
            controlNode.setAttribute("lightUuid", lightInformation.lightcontrol.light->getUuid());
            controlNode.setAttribute("midiNote", lightInformation.midiNote);
            if (lightInformation.lightcontrol.light->isColorEnabled()) {
                controlNode.setAttribute("r", lightInformation.lightcontrol.color.r);
                controlNode.setAttribute("g", lightInformation.lightcontrol.color.g);
                controlNode.setAttribute("b", lightInformation.lightcontrol.color.b);
                controlNode.setAttribute("a", lightInformation.lightcontrol.color.a);
            }
            parentNode.push_back(controlNode);
        }
        xmlNode.push_back(parentNode);
    }
}

void MidiLightXmlSerializer::readEffect(XmlTree &xmlNode, const std::vector<LightRef> &lights,
                                                   std::vector<InputChannelRef> &channels) {
    EffectXmlSerializer::readEffect(xmlNode, lights, channels);
    if (xmlNode.hasChild("lightInformationData")) {
        auto effect = (MidiLight*) mEffect;
        auto lightInformationData = effect->getLightInformation();
        for (auto &informationNode: xmlNode.getChild("lightInformationData").getChildren()) {
            std::string lightUuid = informationNode->getAttributeValue<std::string>("lightUuid");
            auto it = std::find_if(lights.begin(), lights.end(), [&lightUuid](const LightRef lightCandidate){ return lightCandidate->getUuid() == lightUuid;});
            if (it != lights.end()) {
                LightRef light = *it;
                float midiNote = informationNode->getAttributeValue<float>("midiNote");
                ColorA color = Color::black();
                if (light->isColorEnabled()) {
                    float r = informationNode->getAttributeValue<float>("r");
                    float g = informationNode->getAttributeValue<float>("g");
                    float b = informationNode->getAttributeValue<float>("b");
                    float a = informationNode->getAttributeValue<float>("a");
                    color = ColorA(r, g, b, a);
                }
                lightInformationData.push_back(MidiLightInformation(LightControl(light, color, 0.0f), midiNote));
            }
        }
        effect->setLightInformation(lightInformationData);
    }
}


REGISTER_TYPE(MidiLight)