//
// Created by Jur de Vries on 13/03/2018.
//

#ifndef PHOTONICDIRECTOR_MUTEALL_H
#define PHOTONICDIRECTOR_MUTEALL_H

#include "../Effects.h"

namespace photonic {

    class MuteAll : public Effect {
    public:
        enum Inputs {
            kInput_Input = 1,
            kInput_ExportChannel = 2,
            kInput_OffTreshold = 3,
            kInput_MuteSpeed = 4,
        };

        explicit MuteAll(std::string name, std::string uuid = "");

        void execute(double dt) override;
        std::string getTypeName() override;
        std::string getTypeClassName() override;
        Effect::Stage getStage() override;
    protected:
        double mMuteChangeTime;
        float mMuteFactor;
        bool mMuted;
        Timer mTimer;

    };
}

#endif //PHOTONICDIRECTOR_MUTEALL_H
