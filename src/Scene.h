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
        friend class SceneListUI;

    public:
        static SceneRef create(std::string name, std::string uuid = "");

        std::string getUuid();
        void addEffectOn(EffectRef effect);
        std::list<EffectRef>::iterator removeEffectOn(EffectRef effect);
        bool hasEffectOn(EffectRef effect);

        void addEffectOff(EffectRef effect);
        std::list<EffectRef>::iterator removeEffectOff(EffectRef effect);
        bool hasEffectOff(EffectRef effect);

        std::list<EffectRef> getEffectsOn();
        std::list<EffectRef> getEffectsOff();

        void listenToOsc(const osc::Message &message);

        void activate();
        void deActivate();

        std::string oscTriggerAddress;
        std::string name;
        std::string description;


    protected:
        std::string mUuid;
        std::list<EffectRef> mEffectsOn;
        std::list<EffectRef> mEffectsOff;

    private:
        Scene();
    };

    class SceneList {
    public:
        SceneList();
        void addScene(SceneRef scene);
        void createScene(std::string sceneName, std::string description = "");
        void removeScene(SceneRef scene);
        bool hasScene(SceneRef scene);
        void nextScene();
        void previousScene();
        SceneRef getActiveScene();
        void reset();
        void listenToOsc(const osc::Message &message);
        void reorderScene(const SceneRef scene, int newPos);
        void removeAllScenes();
        void onEffectRemove(EffectRef effect);

        std::list<SceneRef> mScenes;
        std::string oscAddress = "/scenes/trigger";
        std::string oscStringMessageNext = "next";

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
        void drawSceneUI(SceneRef &scene);

        SceneListRef mSceneList;


    };

    typedef std::shared_ptr<SceneListUI> SceneListUIRef;

}
#endif //PHOTONICDIRECTOR_SCENE_H
