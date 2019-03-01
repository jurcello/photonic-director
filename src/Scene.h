//
// Created by Jur de Vries on 2019-03-01.
//

#ifndef PHOTONICDIRECTOR_SCENE_H
#define PHOTONICDIRECTOR_SCENE_H

#include <stdio.h>
#include "cinder/app/App.h"
#include "Effects.h"
#include "imgui.h"

namespace photonic {


    class Scene;

    typedef std::shared_ptr<Scene> SceneRef;

    class Scene {
    public:
        Scene();
        Scene(std::string name);
        void addEffectOn(EffectRef effect);
        void removeEffectOn(EffectRef effect);
        bool hasEffectOn(EffectRef effect);

        void addEffectOff(EffectRef effect);
        void removeEffectOff(EffectRef effect);
        bool hasEffectOff(EffectRef effect);

        void listenToOsc(const osc::Message &message);

        void activate();

        std::string oscTriggerAddress;
        std::string name;


    protected:
        std::list<EffectRef> mEffectsOn;
        std::list<EffectRef> mEffectsOff;

    };

    class SceneList {
    public:
        SceneList();
        void addScene(SceneRef scene);
        void createScene(std::string sceneName);
        void removeScene(SceneRef scene);
        bool hasScene(SceneRef scene);
        void nextScene();
        void previousScene();
        SceneRef getActiveScene();
        void reset();
        void listenToOsc(const osc::Message &message);

        std::list<SceneRef> mScenes;
        std::string oscAddress = "/scenes/trigger";

        bool isActive;

    protected:
        std::list<SceneRef>::iterator mSceneIterator;
    };

    typedef std::shared_ptr<SceneList> SceneListRef;

    class SceneListUI {
    public:
        explicit SceneListUI(SceneListRef sceneList);
        void drawGui();

    protected:
        SceneListRef mSceneList;

    };

    typedef std::shared_ptr<SceneListUI> SceneListUIRef;

}
#endif //PHOTONICDIRECTOR_SCENE_H
