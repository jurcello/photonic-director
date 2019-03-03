//
// Created by Jur de Vries on 2019-03-01.
//

#include "Scene.h"

const char* SCENE_SELECT_DRAG_DROP = "SCENE_SEL";
extern const char* EFFECT_SELECT_DRAG_DROP;

using namespace photonic;

photonic::Scene::Scene()
:name("untitled")
{
}

SceneRef Scene::create(std::string name, std::string uuid) {
    if (uuid == "") {
        uuid = generate_uuid(); 
    }
    Scene *const newScene = new Scene();
    newScene->name = name;
    newScene->mUuid = uuid;
    return SceneRef(newScene);
}

std::string Scene::getUuid() {
    return mUuid;
}

void photonic::Scene::addEffectOn(photonic::EffectRef effect) {
    if (! hasEffectOn(effect)) {
        mEffectsOn.push_back(effect);
    }
}

std::list<EffectRef>::iterator photonic::Scene::removeEffectOn(photonic::EffectRef effect) {
    for (auto it = mEffectsOn.begin(); it != mEffectsOn.end(); it++) {
        if (*it == effect) {
            return mEffectsOn.erase(it);
        }
    }
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

std::list<EffectRef>::iterator photonic::Scene::removeEffectOff(photonic::EffectRef effect) {
    for (auto it = mEffectsOff.begin(); it != mEffectsOff.end(); it++) {
        if (*it == effect) {
            return mEffectsOff.erase(it);
        }
    }
}

bool photonic::Scene::hasEffectOff(photonic::EffectRef effect) {
    auto it = std::find(mEffectsOff.begin(), mEffectsOff.end(), effect);
    return it != mEffectsOff.end();
}

std::list<EffectRef> Scene::getEffectsOn() {
    return mEffectsOn;
}

std::list<EffectRef> Scene::getEffectsOff() {
    return mEffectsOff;
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
    auto newScene = Scene::create(sceneName);
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
    (*mSceneIterator)->activate();
}

void photonic::SceneList::reset() {
    mSceneIterator = mScenes.begin();
    isActive = false;
}

void SceneList::removeAllScenes() {
    mScenes.clear();
    reset();
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

void photonic::SceneList::reorderScene(const photonic::SceneRef scene, int newPos) {
    mScenes.remove(scene);
    auto iterator = mScenes.begin();
    std::advance(iterator, newPos);
    mScenes.emplace(iterator, scene);
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

    // Drag and drop variables for ordering.
    int moveTo = 0;
    SceneRef sceneToMove = nullptr;
    int sceneOrderId = 0;

    static SceneRef sceneToEdit = nullptr;
    if (! ui::IsWindowCollapsed()) {
        ui::Text("Scenes");
        for (auto it = mSceneList->mScenes.begin(); it != mSceneList->mScenes.end(); ) {
            ui::PushID(sceneOrderId);
            auto scene = *it;
            bool active = (mSceneList->getActiveScene() == scene) && mSceneList->isActive;

            // Start drag and drop functionality.
            ui::Button(":::");
            ui::SameLine();
            ImGuiDragDropFlags srcFlags = 0;
            if (ui::BeginDragDropSource(srcFlags)) {
                ImGui::SetDragDropPayload(SCENE_SELECT_DRAG_DROP, &scene, sizeof(SceneRef));
                ui::Text("Drag to reorder;");
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                ImGuiDragDropFlags targetFlags = 0;
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(SCENE_SELECT_DRAG_DROP, targetFlags))
                {
                    sceneToMove = *(SceneRef*)payload->Data;
                    moveTo = sceneOrderId;
                }
                ImGui::EndDragDropTarget();
            }

            float fadeValue[] = {(float) active};
            ui::PlotHistogram("", fadeValue, 1, 0, NULL, 0.0f, 1.0f, ImVec2(20, 18));
            ui::SameLine();
            ui::Text("%s", scene->name.c_str());
            ui::SameLine();
            if (ui::Button("Edit")) {
                sceneToEdit = scene;
            }
            ui::SameLine();
            if (ui::Button("Remove")) {
                it = mSceneList->mScenes.erase(it);
                ui::PopID();
                continue;
            }
            ui::PopID();
            it++;
            sceneOrderId++;
        }
        if (sceneToMove != nullptr) {
            mSceneList->reorderScene(sceneToMove,  moveTo);
        }
    }
    if (sceneToEdit != nullptr) {
        drawSceneUI(sceneToEdit);
    }
}

void photonic::SceneListUI::drawSceneUI(SceneRef &scene) {
    ui::ScopedWindow window("Edit scene");
    if (! ui::IsWindowCollapsed()) {
        ui::Text("%s", scene->name.c_str());
        ui::InputText("Name", &scene->name);

        ui::ListBoxHeader("Effects to enable");
        int effectOnId = 0;
        for (auto it = scene->mEffectsOn.begin(); it != scene->mEffectsOn.end(); ) {
            auto effect = *it;
            ui::PushID(effectOnId);
            ui::Text("%s", effect->getName().c_str());
            if (ui::Button("Remove")) {
                it = scene->removeEffectOn(effect);
                ui::PopID();
                continue;
            }
            it++;
            ui::PopID();
            effectOnId++;
        }
        ui::ListBoxFooter();
        if (ui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EFFECT_SELECT_DRAG_DROP, target_flags)) {
                EffectRef effect = *(EffectRef*)payload->Data;
                scene->addEffectOn(effect);
            }
        }

        ui::ListBoxHeader("Effects to disable");
        int effectOffId = 0;
        for (auto it = scene->mEffectsOff.begin(); it != scene->mEffectsOff.end(); ) {
            auto effect = *it;
            ui::PushID(effectOffId);
            ui::Text("%s", effect->getName().c_str());
            if (ui::Button("Remove")) {
                it = scene->removeEffectOff(effect);
                ui::PopID();
                continue;
            }
            it++;
            ui::PopID();
            effectOffId++;
        }
        ui::ListBoxFooter();
        if (ui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EFFECT_SELECT_DRAG_DROP, target_flags)) {
                EffectRef effect = *(EffectRef*)payload->Data;
                scene->addEffectOff(effect);
            }
        }

        if (ui::Button("Done")) {
            scene = nullptr;
        }
    }

}
