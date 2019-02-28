
#ifndef PHOTONICDIRECTOR_ShutDownHuman
#define PHOTONICDIRECTOR_ShutDownHuman

#include "../Effects.h"
#include "CinderImGui.h"
#include "cinder/Perlin.h"

namespace photonic {

    class ShutDownHuman : public Effect {
    public:
        enum Inputs {
            kInput_FirstStageTime = 1,
            kInput_SecondStageTime = 2,
            kInput_ThirdStageTime = 3,
            kInput_MaxOutputFirst = 4,
            kInput_MaxOutputThird = 5,
            kInput_TriggerChannel = 6,
            kInput_PerlinSpeed = 7,
            kInput_NoiseAmount = 8,
            kInput_RandomNoiseAmount = 9,
            kInput_SecondStageNoiseAmount = 10,
            kInput_FirstStageEaseComponent = 11,
            kInput_SecondsStageDropPercentage = 12,

        };
        enum EffectStage {
            idle,
            first,
            second,
            third
        };

        ShutDownHuman(std::string name, std::string uuid = "");

        void execute(double dt) override;
        Stage getStage() override;

        void drawEditGui() override;
        std::string getTypeName() override;

        std::string getTypeClassName() override;

    protected:
        EffectStage mCurrentStage;
        bool mIsShutdown;
        Timer mTimer;
        Perlin mPerlin;
        std::string mOscOutTriggerFirstStage = "/shutdownHuman/trigger/firststage";
        std::string mOscOutTriggerSecondStage = "/shutdownHuman/trigger/secondstage";
        std::string mOscOutTriggerThirdStage = "/shutdownHuman/trigger/thirdstage";
        std::string mOscOutOffToggle = "/shutdownHuman/toggle/off";

        void updateState();
        void setSwitchedOffLightIntensity(LightRef light);
        void setLightIntensities(float intensity);
        void muteAll();
        void blowupIntensities(float ratio, float max = 1.0f);
        void addNoise();
        void sendTrigger(std::string address, int value);
        void saveLightIntensities();

    };

}

#endif //PHOTONICDIRECTOR_ShutDownHuman
