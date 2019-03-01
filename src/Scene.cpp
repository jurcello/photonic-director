//
// Created by Jur de Vries on 2019-03-01.
//

#include "Scene.h"

photonic::Scene::Scene()
:name("untitled")
{
}

photonic::Scene::Scene(std::string name)
:name(name)
{
}

void photonic::Scene::addEffectOn(photonic::EffectRef effect) {
    if (! hasEffectOn(effect)) {
        mEffectsOn.push_back(effect);
    }
}

void photonic::Scene::removeEffectOn(photonic::EffectRef effect) {
    mEffectsOn.remove(effect);
}

bool photonic::Scene::hasEffectOn(photonic::EffectRef effect) {
    auto it = std::find(mEffectsOn.begin(), mEffectsOn.end(), effect);
    return it != mEffectsOn.end();
}

void photonic::Scene::addEffectOff(photonic::EffectRef effect) {
    if (! hasEffectOff(effect)) {
        mEffectsOff.push_back(effect);
    }
}

void photonic::Scene::removeEffectOff(photonic::EffectRef effect) {
    mEffectsOff.remove(effect);
}

bool photonic::Scene::hasEffectOff(photonic::EffectRef effect) {
    auto it = std::find(mEffectsOff.begin(), mEffectsOff.end(), effect);
    return it != mEffectsOff.end();
}

void photonic::Scene::listenToOsc(const osc::Message &message) {
    if (message.getAddress() == oscTriggerAddress) {
        switch (message.getArgType(0)) {
            case osc::ArgType::FLOAT: {
                auto value = message.getArgFloat(0);
                if (value == 1.f) {
                    activate();
                }
                break;
            }

            case osc::ArgType::INTEGER_32: {
                auto intValue = message.getArgInt32(0);
                if (intValue == 1) {
                    activate();
                }
                break;
            }

        }
    }
}

void photonic::Scene::activate() {
    for (const auto &effect: mEffectsOn) {
        effect->isTurnedOn = true;
    }
    for (const auto &effect: mEffectsOff) {
        effect->isTurnedOn = false;
    }
}

photonic::SceneList::SceneList()
:isActive(false)
{
    mSceneIterator = mScenes.begin();
}

void photonic::SceneList::addScene(photonic::SceneRef scene) {
    if (! hasScene(scene)) {
        mScenes.push_back(scene);
        if (mScenes.size() == 1) {
            mSceneIterator = mScenes.begin();
        }
    }
}

void photonic::SceneList::createScene(std::string sceneName) {
    auto newScene = SceneRef(new Scene(sceneName));
    addScene(newScene);
}

void photonic::SceneList::removeScene(photonic::SceneRef scene) {
    if (scene == *mSceneIterator) {
        mSceneIterator = mScenes.begin();
    }
    mScenes.remove(scene);
}

bool photonic::SceneList::hasScene(photonic::SceneRef scene) {
    auto it = std::find(mScenes.begin(), mScenes.end(), scene);
    return it != mScenes.end();

}

photonic::SceneRef photonic::SceneList::getActiveScene() {
    if (mScenes.size() == 0) {
        return nullptr;
    }
    return *mSceneIterator;
}

void photonic::SceneList::nextScene() {
    if (isActive) {
        std::advance(mSceneIterator, 1);
        if (mSceneIterator == mScenes.end()) {
            mSceneIterator = mScenes.begin();
            isActive = false;
            return;
        }
    }
    else {
        isActive = true;
        mSceneIterator = mScenes.begin();
    }
    (*mSceneIterator)->activate();
}

void photonic::SceneList::previousScene() {
    if (! isActive) {
        (*mSceneIterator)->activate();
        isActive = true;
        return;
    }
    if (mSceneIterator != mScenes.begin()) {
        std::advance(mSceneIterator, -1);
    }
}

void photonic::SceneList::reset() {
    mSceneIterator = mScenes.begin();
    isActive = false;
}

void photonic::SceneList::listenToOsc(const osc::Message &message) {
    if (message.getAddress() == oscAddress && message.getArgType(0) == osc::ArgType::INTEGER_32) {
        int direction = message.getArgInt32(0);
        if (direction == 0) {
            previousScene();
        }
        else {
            nextScene();
        }

    }
}

photonic::SceneListUI::SceneListUI(photonic::SceneListRef sceneList)
:mSceneList(sceneList)
{
}

void photonic::SceneListUI::drawGui() {
    ImGui::ScopedWindow window("Scenes");
    ui::InputText("osc address for triggering", &mSceneList->oscAddress);
    if (ui::Button("Next")) {
        mSceneList->nextScene();
    }
    ui::SameLine();
    if (ui::Button("Previous")) {
        mSceneList->previousScene();
    }
    ui::SameLine();
    if (ui::Button("Reset")) {
        mSceneList->reset();
    }

    if (ui::Button("Add Scene")) {
        ui::OpenPopup("Add scene");
    }
    if (ui::BeginPopupModal("Add scene")) {
        static std::string sceneName;
        ui::InputText("Name", &sceneName);
        if (ui::Button("Done")) {
            mSceneList->createScene(sceneName);
            ui::CloseCurrentPopup();
        }
        ui::SameLine();
        if (ui::Button("Cancel")) {
            ui::CloseCurrentPopup();
        }
        ui::EndPopup();
    }
    ui::Separator();
    ui::NewLine();
    static SceneRef sceneToEdit = nullptr;
    if (! ui::IsWindowCollapsed()) {
        ui::ListBoxHeader("Edit scenes", mSceneList->mScenes.size(), 20);
        for (auto it = mSceneList->mScenes.begin(); it != mSceneList->mScenes.end(); ) {
            auto scene = *it;
            bool active = (mSceneList->getActiveScene() == scene) && mSceneList->isActive;
            float fadeValue[] = {(float) active};
            ui::PlotHistogram("", fadeValue, 1, 0, NULL, 0.0f, 1.0f, ImVec2(20, 18));
            ui::SameLine();
            if (ui::Selectable(scene->name.c_str()), sceneToEdit == scene) {
                sceneToEdit = scene;
            }
            ui::SameLine();
            if (ui::Button("Remove")) {
                it = mSceneList->mScenes.erase(it);
                continue;
            }
            it++;
        }
        ui::ListBoxFooter();
    }


}
