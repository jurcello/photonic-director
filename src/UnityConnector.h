//
// Created by Jur de Vries on 17/08/2018.
//

#ifndef PHOTONICDIRECTOR_UNITYCONNECTOR_H
#define PHOTONICDIRECTOR_UNITYCONNECTOR_H

#include <stdio.h>
#include "cinder/app/App.h"
#include "cinder/Json.h"
#include "Light.h"

using namespace std;

class UnityConnector {
public:
    UnityConnector();

    void initialize(std::string address, int port, vector<LightRef>* lights, LightFactory* lightFactory);
    void sync(std::function<void(const LightRef)> cleanFunction);
    void updateCreateLight(JsonTree lightNode);
    void cleanLights(std::function<void(const LightRef)> cleanFunction);


protected:
    vector<LightRef>* mLights;
    vector<string> mSyncedUuids;
    string mAddress;
    int mPort;
    LightFactory* mLightFactory;

};


#endif //PHOTONICDIRECTOR_UNITYCONNECTOR_H
