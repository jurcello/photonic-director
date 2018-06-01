//
// Created by Jur de Vries on 29/03/2018.
//

#ifndef PHOTONICDIRECTOR_ORIGINATINGWAVE_H
#define PHOTONICDIRECTOR_ORIGINATINGWAVE_H

#include "../Effects.h"

namespace photonic {
    class ValueBuffer {
    public:
        ValueBuffer(int maxSize);
        void pushValue(float value);
        void setTimeInterval(float timeInterval);
        void setSpeed(float speed);
        float getValueAtDistance(float distance);

    protected:
        float mTimeInverval;
        float mSpeed;
        std::vector<float> mValues;
        int mLastPos;
    };


    class OriginatingWave : public Effect{
    public:
        enum Inputs {
            kInput_Location = 1,
            kInput_Decay = 2,
            kInput_InputChannel = 3,
            kInput_Speed = 4,
            kInput_EffectColor = 5,
            kInput_ValueDecreaseSpeed = 6,
        };

        OriginatingWave(std::string name, std::string uuid = "");

        virtual void execute(double dt) override;
        virtual std::string getTypeName() override;
        virtual std::string getTypeClassName() override;

    protected:
        ValueBuffer mBuffer;
        float mCurrentValue;
        float mTargetValue;

        void updateInteralState(double dt);
    };

}

#endif //PHOTONICDIRECTOR_ORIGINATINGWAVE_H
