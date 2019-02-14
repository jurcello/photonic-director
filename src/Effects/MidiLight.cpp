
#include "MidiLight.h"

using namespace cinder;
using namespace photonic;
using namespace ci;


MidiLightInformation::MidiLightInformation(LightControl control, uint8_t note, float fadetime)
:lightcontrol(control), midiNote(note), fadeoutTime(fadetime), fadeout(false)
{
}

MidiLight::MidiLight(std::string name, std::string uuid)
: Effect(name, uuid), mLastMidiNote(0), mUseModulo(false)
{
    registerParam(Parameter::Type::kType_Float, kInput_NoiseAmount, 0.1f, "Noise Amount");
    registerParam(Parameter::Type::kType_Float, kInput_NoiseSpeed, 10.0f, "Noise Speed");

}

void MidiLight::init() {
    mTimer.start();
}

void MidiLight::execute(double dt) {
    Effect::execute(dt);
    auto elapsedTime = (float) mTimer.getSeconds();
    if (mMidiLightInformation.size() !=  mLights.size()) {
        repopulateLightInformation();
    }
    // fade the lights.
    for (auto &information: mMidiLightInformation) {
        float intensity = information.lightcontrol.light->getEffetcIntensity(mUuid);
        if (intensity > 0) {
            if (information.fadeout) {
                intensity -= dt / information.fadeoutTime;
                if (intensity <= 0) {
                    intensity = 0;
                    information.fadeout = false;
                }
            }
            // Add some noise.
            float noise = mPerlin.noise(
                    information.lightcontrol.light->getPosition().x,
                    information.lightcontrol.light->getPosition().y,
                    elapsedTime * mParams[kInput_NoiseSpeed]->floatValue / 10.f) * mParams[kInput_NoiseAmount]->floatValue;
            intensity *= (1 + noise);
            information.lightcontrol.light->setEffectIntensity(mUuid, intensity);
        }
    }
}

void MidiLight::listenToMidi(const smf::MidiMessage *message) {
    if (message->isNote()) {
        int keyNumber = message->getKeyNumber();
        if (mUseModulo) {
            keyNumber = (keyNumber % 12) + 1;
        }
        mLastMidiNote = keyNumber;
        float intensity = 0.f;
        if (message->isNoteOn()) {
            intensity = message->getVelocity() / 128.f;
        }
        else {
            intensity = 0.f;
        }
        for (auto &information: mMidiLightInformation) {
            if (keyNumber == information.midiNote) {
                if (intensity == 0.f && information.fadeoutTime > 0) {
                    information.fadeout = true;
                }
                else {
                    information.fadeout = false;
                    information.lightcontrol.light->setEffectIntensity(mUuid, intensity);
                }
                if (information.lightcontrol.light->isColorEnabled()) {
                    information.lightcontrol.light->setEffectColor(mUuid, information.lightcontrol.color);
                }
            }
        }
    }
}

void MidiLight::drawEditGui() {
    ui::Text("Last key: %i", mLastMidiNote);
    ui::Text("Last key mod: %i", (mLastMidiNote % 12) + 1);
    ui::Checkbox("Use modulo (only octaves)", &mUseModulo);
    int id = 0;
    for (auto &control: mMidiLightInformation) {
        ui::PushID(id);
        ui::Text("%s", control.lightcontrol.light->mName.c_str());
        ui::SameLine();
        if (control.lightcontrol.light->isColorEnabled()) {
            ui::ColorEdit4("", &control.lightcontrol.color[0]);
        }
        ui::InputInt("Midi note value", &control.midiNote);
        ui::InputFloat("Fadeout time", &control.fadeoutTime);
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
            mMidiLightInformation.emplace_back(MidiLightInformation(LightControl(light, ColorA::black(), 0.0f), 0, 0.0f));
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
    xmlNode.setAttribute("useModulo", effect->mUseModulo);
    if (! lightInformationData.empty()) {
        XmlTree parentNode;
        parentNode.setTag("lightInformationData");
        for (auto &lightInformation : lightInformationData) {
            XmlTree controlNode;
            controlNode.setTag("lightInformation");
            controlNode.setAttribute("lightUuid", lightInformation.lightcontrol.light->getUuid());
            controlNode.setAttribute("midiNote", lightInformation.midiNote);
            controlNode.setAttribute("fadeoutTime", lightInformation.fadeoutTime);
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
    auto effect = (MidiLight*) mEffect;
    if (xmlNode.hasAttribute("useModule")) {
        effect->mUseModulo = xmlNode.getAttributeValue<bool>("useModulo");
    }
    if (xmlNode.hasChild("lightInformationData")) {
        auto lightInformationData = effect->getLightInformation();
        for (auto &informationNode: xmlNode.getChild("lightInformationData").getChildren()) {
            std::string lightUuid = informationNode->getAttributeValue<std::string>("lightUuid");
            auto it = std::find_if(lights.begin(), lights.end(), [&lightUuid](const LightRef lightCandidate){ return lightCandidate->getUuid() == lightUuid;});
            if (it != lights.end()) {
                LightRef light = *it;
                float midiNote = informationNode->getAttributeValue<float>("midiNote");
                float fadeoutTime = informationNode->getAttributeValue<float>("fadeoutTime");
                ColorA color = Color::black();
                if (light->isColorEnabled()) {
                    float r = informationNode->getAttributeValue<float>("r");
                    float g = informationNode->getAttributeValue<float>("g");
                    float b = informationNode->getAttributeValue<float>("b");
                    float a = informationNode->getAttributeValue<float>("a");
                    color = ColorA(r, g, b, a);
                }
                lightInformationData.push_back(MidiLightInformation(LightControl(light, color, 0.0f), midiNote, fadeoutTime));
            }
        }
        effect->setLightInformation(lightInformationData);
    }
}


REGISTER_TYPE(MidiLight)