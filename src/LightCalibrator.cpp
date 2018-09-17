//
// Created by Jur de Vries on 09/02/2018.
//

#include "LightCalibrator.h"

void LightCalibrator::setLights(std::vector<LightRef> *lights) {
    mLights = lights;
}

void LightCalibrator::setOscSender(osc::SenderUdp *OscSender) {
    mSender = OscSender;
}

LightCalibrator::LightCalibrator()
: mIsCalibrating(false), currentPosition(vec3(0.0f)), mCurrentIntensity(0)
{}

void LightCalibrator::receiveOscMessage(const osc::Message &message) {
    if (mIsCalibrating) {
        bool send = false;
        if (mCurrentLight != nullptr) {
            if (message.getAddress() == "/lightCalib/xz") {
                currentPosition.x = message.getArgFloat(0);
                currentPosition.z = message.getArgFloat(1);
                send = true;
            }
            if (message.getAddress() == "/lightCalib/y") {
                currentPosition.y = message.getArgFloat(0);
                send = true;
            }
            if (message.getAddress() == "/lightCalib") {
                currentPosition.x = message.getArgFloat(0);
                currentPosition.y = message.getArgFloat(1);
                currentPosition.z = message.getArgFloat(2);
            }
            mCurrentLight->setPosition(currentPosition);
        }
        if ((message.getAddress() == "/pfffmaaktnietuitwat" || message.getAddress() == "/lightCalib/next") && message.getArgFloat(0) == 1) {
            mTimer.stop();
            mTimer.start(0.0f);
            mCurrentLightIterator++;
            // Skip the lights without DMX channel.
            while (mCurrentLightIterator != mLights->end() && (*mCurrentLightIterator)->getDmxChannel() == 0) {
                mCurrentLightIterator++;
            }
            if (mCurrentLightIterator == mLights->end()) {
                mIsCalibrating = false;
                mCurrentLight = nullptr;
                mTimer.stop();
            }
            else {
                mCurrentLight = *mCurrentLightIterator;
                currentPosition = mCurrentLight->getPosition();
                send = true;
            }
        }
        if (send) {
            broadcastCurrentLight();
        }
    }
}

void LightCalibrator::start() {
    mIsCalibrating = true;
    mCurrentLightIterator = mLights->begin();
    mCurrentLight = *mCurrentLightIterator;
    currentPosition = mCurrentLight->getPosition();
    broadcastCurrentLight();
    mTimer.start(0.0f);
}

void LightCalibrator::broadcastCurrentLight() {
    if (mCurrentLight && mSender) {
        osc::Message message("/lightCalib/name");
        message.append(mCurrentLight->mName);
        mSender->send(message);

        osc::Message messageXZ("/lightCalib/xz");
        messageXZ.append(currentPosition.x);
        messageXZ.append(currentPosition.z);
        mSender->send(messageXZ);

        osc::Message messageY("/lightCalib/y");
        messageY.append(currentPosition.y);
        mSender->send(messageY);
    }
}

bool LightCalibrator::isCalibrating() {
    return mIsCalibrating;
}

LightRef LightCalibrator::getCurrentLight() {
    return mCurrentLight;
}

float LightCalibrator::getCurrentIntensity() {
    mCurrentIntensity = mTimer.getSeconds() / 2.0f;
    if (mCurrentIntensity > 1) {
        mCurrentIntensity = 1.0f;
    }
    return mCurrentIntensity;
}
