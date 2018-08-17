//
// Created by Jur de Vries on 17/08/2018.
//

#include "UnityConnector.h"

using namespace cinder;

UnityConnector::UnityConnector()
:mAddress("192.168.1.8"),
mPort(8089)
{
}

void UnityConnector::initialize(std::string address, int port, vector<LightRef>* lights, LightFactory* lightFactory) {
    mAddress = address;
    mPort = port;
    mLights = lights;
    mLightFactory = lightFactory;
}

// TODO: This is a rather ugly architecture. We need another way to remove the lights from the effects.
void UnityConnector::sync(std::function<void(const LightRef)> cleanFunction) {
    // Empty the synced uuid vector.
    mSyncedUuids.clear();

    std::string fullUrl = "http://" + mAddress + ":" + std::to_string(mPort);
    Url url(fullUrl, true);
    UrlOptions options;
    options.setTimeout(1.0f);
    options.setIgnoreCache(true);
    auto content = loadUrl(url, options);
    JsonTree lightsResults;
    // Load the url.
    lightsResults = JsonTree(content);
    app::console() << "Number of items" << lightsResults.hasChildren() << std::endl;
    JsonTree lights;
    lights = lightsResults.getChild("lights");
    for (auto light: lights) {
        updateCreateLight(light);
    }
    cleanLights(cleanFunction);
}

void UnityConnector::updateCreateLight(JsonTree lightNode) {
    app::console() << "found light" << lightNode.getChild("uuid").getValue() << std::endl;

    // Values.
    std::string uuid = lightNode.getChild("uuid").getValue();
    std::string name = lightNode.getChild("name").getValue();
    std::string type = lightNode.getChild("type").getValue();
    std::string oscAddress = lightNode.getChild("oscAddress").getValue();
    vec3 position;
    position.x = lightNode.getChild("position").getChild("x").getValue<float>();
    position.y = lightNode.getChild("position").getChild("y").getValue<float>();
    position.z = lightNode.getChild("position").getChild("z").getValue<float>();
    ColorA color;
    color.r = lightNode.getChild("color").getChild("r").getValue<float>();
    color.g = lightNode.getChild("color").getChild("g").getValue<float>();
    color.b = lightNode.getChild("color").getChild("b").getValue<float>();
    color.a = lightNode.getChild("color").getChild("a").getValue<float>();

    mSyncedUuids.push_back(uuid);

    LightRef light;
    auto it = std::find_if(mLights->begin(), mLights->end(), [uuid](const LightRef lightCandidate) { return lightCandidate->getUuid() == uuid;});
    if (it != mLights->end()) {
        light = *it;
    }
    else {
        light = mLightFactory->create(position, type, uuid);
        mLights->push_back(light);
    }
    light->setPosition(position);
    light->mName = name;
    light->color = color;
    light->mOscAdress = oscAddress;
    if (oscAddress.length() > 0) {
        light->mSendOsc = true;
    }
}

void UnityConnector::cleanLights(std::function<void(const LightRef)> cleanFunction) {
    for (auto it = mLights->begin(); it != mLights->end(); ) {
        std::string uuid = (*it)->getUuid();
        if (std::find(mSyncedUuids.begin(), mSyncedUuids.end(), uuid) == mSyncedUuids.end()) {
            cleanFunction(*it);
            it = mLights->erase(it);
            continue;
        }
        it++;
    }
}
