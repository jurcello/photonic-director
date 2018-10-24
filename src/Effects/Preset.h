//
// Created by Jur de Vries on 23/10/2018.
//

#ifndef PHOTONICDIRECTOR_PRESET_H
#define PHOTONICDIRECTOR_PRESET_H

#include "../Effects.h"
#include "CinderImGui.h"
#include "../Light.h"

namespace photonic {
    class Preset : public Effect {
    public:
        explicit Preset(std::string name, std::string uuid = "");
        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;
        void drawEditGui() override;
        EffectXmlSerializerRef getXmlSerializer() override;

        std::vector<LightControl> getLightControls();
        void setLightControls(std::vector<LightControl> controls);

    protected:
        std::vector<LightControl> mLightControls;

        void repopulateLightControls();
    };


    class PresetXmlSerializer : public EffectXmlSerializer {
    public:
        explicit PresetXmlSerializer(Effect* cEffect);

        void writeEffect(XmlTree &xmlNode) override;
        void readEffect(XmlTree &xmlNode, const std::vector<LightRef> &lights, std::vector<InputChannelRef> &channels) override;

    };
}


#endif //PHOTONICDIRECTOR_PRESET_H
