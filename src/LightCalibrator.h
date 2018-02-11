//
// Created by Jur de Vries on 09/02/2018.
//

#ifndef PHOTONICDIRECTOR_LIGHTCALIBRATOR_H
#define PHOTONICDIRECTOR_LIGHTCALIBRATOR_H

#include "Osc.h"
#include "Light.h"

class LightCalibrator {
public:
    LightCalibrator();

    void receiveOscMessage(const osc::Message &message);
    void setLights(std::vector<LightRef> &lights);
    void setOscSender(osc::SenderUdp* OscSender);
    void start();
    void broadcastCurrentLight();
    bool isCalibrating();
    LightRef getCurrentLight();

    vec3 currentPosition;

protected:
    std::vector<LightRef>::iterator mCurrentLightIterator;
    LightRef mCurrentLight;
    osc::SenderUdp* mSender;
    std::vector<LightRef> mLights;
    bool mIsCalibrating;

};


#endif //PHOTONICDIRECTOR_LIGHTCALIBRATOR_H
