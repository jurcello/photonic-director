//
// Created by Jur de Vries on 12/02/2018.
//

#ifndef PHOTONICDIRECTOR_WAVEEFFECT_H
#define PHOTONICDIRECTOR_WAVEEFFECT_H

#include "../Effects.h"
#include "cinder/Perlin.h"

namespace photonic {
    class WaveEffect : public Effect {
    public:
        enum Inputs {
            kInput_BaseColor = 1,
            kInput_EffectColor = 2,
            kInput_StartPoint = 3,
            kInput_EndPoint = 4,
            kInput_Direction = 5,
            kInput_Width = 6,
            kInput_Speed = 7,
            kInput_NoiseAmount = 8,
            kInput_NoiseSpeed = 9,
            kInput_SpeedExternal = 10,
        };
        WaveEffect(std::string name, std::string uuid = "");

        virtual void execute(double dt) override;
        void init() override ;
        virtual std::string getTypeName() override;
        virtual std::string getTypeClassName() override;

    protected:

        vec3 mPlaneNormal;
        vec3 mLastPosition;
        Perlin mPerlin;
        Timer mTimer;
        double getBellIntensity(double distance, double width);
        double getDistanceToWave(vec3 wavePosition, vec3 position);
        vec3 getCurrentWavePosition(double dt);
    };
}


#endif //PHOTONICDIRECTOR_WAVEEFFECT_H
