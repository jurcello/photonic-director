
#ifndef PHOTONICDIRECTOR_MidiLight
#define PHOTONICDIRECTOR_MidiLight

#include "../Effects.h"
#include "CinderImGui.h"
#include "../Light.h"
#include "cinder/Perlin.h"

namespace photonic {

    struct MidiLightInformation {
        MidiLightInformation(LightControl control, uint8_t note, float fadetime);

        LightControl lightcontrol;
        int midiNote;
        float fadeoutTime;
        bool fadeout;
        bool shouldHightlight;
    };

    class MidiLight : public Effect {
    public:
        enum Inputs {
            kInput_NoiseAmount = 1,
            kInput_NoiseSpeed = 2,
        };

        explicit MidiLight(std::string name, std::string uuid = "");

        void execute(double dt) override;
        void init() override;

        void drawEditGui() override;
        EffectXmlSerializerRef getXmlSerializer() override;
        std::vector<MidiLightInformation> getLightInformation();
        void setLightInformation(std::vector<MidiLightInformation> lightInformation);
        void listenToMidi(const smf::MidiMessage* message) override;

        std::string getTypeName() override;

        std::string getTypeClassName() override;
        // Part of the interface. Don't set protected.
        bool mUseModulo;
    protected:
        std::vector<MidiLightInformation> mMidiLightInformation;
        int mLastMidiNote;
        bool mShowLampsWhenHovered;

        Timer mTimer;
        Perlin mPerlin;

        void repopulateLightInformation();

    };

    class MidiLightXmlSerializer : public EffectXmlSerializer {
    public:
        explicit MidiLightXmlSerializer(Effect* cEffect);

        void writeEffect(XmlTree &xmlNode) override;
        void readEffect(XmlTree &xmlNode, const std::vector<LightRef> &lights, std::vector<InputChannelRef> &channels) override;

    };
}

#endif //PHOTONICDIRECTOR_MidiLight
