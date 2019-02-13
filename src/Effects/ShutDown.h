
#ifndef PHOTONICDIRECTOR_ShutDown
#define PHOTONICDIRECTOR_ShutDown

#include "../Effects.h"
#include "CinderImGui.h"
#include "cinder/Perlin.h"

namespace photonic {

    class ShutDown : public Effect {
    public:
        enum Inputs {
            kInput_ShutdownTime = 1,
            kInput_PerlinSpeed = 2,
            kInput_NoiseAmount = 3,
            kInput_RandomNoiseAmount = 4,
            kInput_TriggerChannel = 5,
            kInput_TimeBetweenUpdates = 6,
        };

        ShutDown(std::string name, std::string uuid = "");

        void execute(double dt) override;
        void init() override;
        Stage getStage() override;
        void drawEditGui() override;

        std::string getTypeName() override;
        std::string getTypeClassName() override;

    protected:
        void updateState();
        void sendSoundTrigger(std::string address);
        void sendSoundVolume(float volume);
        Perlin mPerlin;
        Timer mTimer;
        std::string mOscOutAddressTrigger = "/shutdown/trigger/sound";
        std::string mOscOutAddressTriggerFlickerStart = "/shutdown/triggerFlicker/sound";
        std::string mOscOutAddressVolume = "/shutdown/volume/sound";
        bool mIsShutDown;
        float mLastUpdateSecondsAgo;
    };

}

#endif //PHOTONICDIRECTOR_ShutDown
