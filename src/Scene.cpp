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

void Scene::setEffectsStillOn(std::list<EffectRef> effects) {
    mEffectsStillOn.clear();
    for (auto effect: effects) {
        if (std::find(mEffectsOn.begin(), mEffectsOn.end(), effect) == mEffectsOn.end()) {
            mEffectsStillOn.push_back(effect);
        }
    }
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

void Scene::deActivate() {
    for (const auto &effect: mEffectsOn) {
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
        updateEffectsStillOn();
    }
}

void photonic::SceneList::createScene(std::string sceneName, std::string description) {
    auto newScene = Scene::create(sceneName);
    newScene->description = description;
    addScene(newScene);
}

void photonic::SceneList::removeScene(photonic::SceneRef scene) {
    if (scene == *mSceneIterator) {
        mSceneIterator = mScenes.begin();
    }
    mScenes.remove(scene);
    updateEffectsStillOn();
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

void SceneList::activateScene(std::string name) {
    SceneRef sceneToActivate = nullptr;
    for (auto it = mScenes.begin(); it != mScenes.end(); it++) {
        if ((*it)->name == name) {
            sceneToActivate = *it;
            break;
        }
    }
    if (sceneToActivate != nullptr) {
        reset();
        while (*mSceneIterator != sceneToActivate) {
            nextScene();
        }
    }
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
        // Deactivate the current scene.
        // Then move the currentScene iterator 1 back.
        (*mSceneIterator)->deActivate();
        std::advance(mSceneIterator, -1);
    }
    // In order to arrive at the same state,
    // start the whole sequence from the first scene.
    for (auto iterator = mScenes.begin(); iterator != mSceneIterator; iterator++) {
        (*iterator)->activate();
    }
    (*mSceneIterator)->activate();
}

void photonic::SceneList::reset() {
    while (mSceneIterator != mScenes.begin()) {
        previousScene();
    }
    mSceneIterator = mScenes.begin();
    isActive = false;
}

void SceneList::removeAllScenes() {
    mScenes.clear();
    reset();
}

void SceneList::onEffectRemove(EffectRef &effect) {
    for (const auto &scene : mScenes) {
        if (scene->hasEffectOff(effect)) {
            scene->removeEffectOff(effect);
        }
        if (scene->hasEffectOn(effect)) {
            scene->removeEffectOn(effect);
        }
    }
}

void SceneList::updateEffectsStillOn() {
    std::list<EffectRef> effectsStillOn;
    for (const auto &scene : mScenes) {
        for (const auto &effect: scene->getEffectsOn()) {
            effectsStillOn.push_back(effect);
        }
        for (const auto &effect: scene->getEffectsOff()) {
            effectsStillOn.remove(effect);
        }
        scene->setEffectsStillOn(effectsStillOn);
    }
}

void photonic::SceneList::listenToOsc(const osc::Message &message) {
    if (message.getAddress() == oscAddress) {
        if (message.getArgType(0) == osc::ArgType::INTEGER_32) {
            int direction = message.getArgInt32(0);
            if (direction == 0) {
                previousScene();
            }
            else {
                nextScene();
            }
        }
        else if (message.getArgType(0) == osc::ArgType::STRING) {
            if (message.getArgString(0) == oscStringMessageNext) {
                nextScene();
            }
            else {
                activateScene(message.getArgString(0));
            }
        }
    }
}

void photonic::SceneList::reorderScene(const photonic::SceneRef scene, int newPos) {
    mScenes.remove(scene);
    auto iterator = mScenes.begin();
    std::advance(iterator, newPos);
    mScenes.emplace(iterator, scene);
    updateEffectsStillOn();
    // Because the order of the list changed, most probably the sceneIterator now contains
    // an invalid value. Therefor perform a reset.
    reset();
}


photonic::SceneListUI::SceneListUI(photonic::SceneListRef sceneList)
:mSceneList(sceneList)
{
}

void photonic::SceneListUI::drawGui() {
    ImGui::ScopedWindow window("Scenes");
    ui::InputText("osc address for triggering", &mSceneList->oscAddress);
    ui::InputText("osc message for next", &mSceneList->oscStringMessageNext);
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
        static std::string sceneDescription;
        ui::InputText("Name", &sceneName);
        ui::InputTextMultiline("Description", &sceneDescription);
        if (ui::Button("Done")) {
            mSceneList->createScene(sceneName, sceneDescription);
            sceneDescription = "";
            sceneName = "";
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
            if (ui::IsItemHovered()) {
                std::string effectOnList = "Will turn on:";
                for (const auto effect : scene->mEffectsOn) {
                    effectOnList += "\n- " + effect->getName();
                }
                std::string effectOffList = "Will turn off:";
                for (const auto effect : scene->mEffectsOff) {
                    effectOffList += "\n- " + effect->getName();
                }
                std::string effectStillOnList = "Are still on:";
                for (const auto effect : scene->mEffectsStillOn) {
                    effectStillOnList += "\n- " + effect->getName();
                }
                ui::SetTooltip("%s\n%s\n%s\n%s", scene->description.c_str(), effectOnList.c_str(), effectOffList.c_str(), effectStillOnList.c_str());
            }
            if (ui::IsItemClicked()) {
                sceneToEdit = scene;
            }
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
        ui::InputTextMultiline("Description", &scene->description);

        ui::ListBoxHeader("Effects to enable");
        int effectOnId = 0;
        for (auto it = scene->mEffectsOn.begin(); it != scene->mEffectsOn.end(); ) {
            auto effect = *it;
            ui::PushID(effectOnId);
            ui::Text("%s", effect->getName().c_str());
            ui::SameLine();
            if (ui::Button("Remove")) {
                it = scene->removeEffectOn(effect);
                mSceneList->updateEffectsStillOn();
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
                mSceneList->updateEffectsStillOn();
            }
        }

        ui::ListBoxHeader("Effects to disable");
        int effectOffId = 0;
        for (auto it = scene->mEffectsOff.begin(); it != scene->mEffectsOff.end(); ) {
            auto effect = *it;
            ui::PushID(effectOffId);
            ui::Text("%s", effect->getName().c_str());
            ui::SameLine();
            if (ui::Button("Remove")) {
                it = scene->removeEffectOff(effect);
                mSceneList->updateEffectsStillOn();
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
                mSceneList->updateEffectsStillOn();
            }
        }
        ui::ListBoxHeader("Effects still on");
        int effectStillOnId = 0;
        bool effectAddedToOff = false;
        for (auto it = scene->mEffectsStillOn.begin(); it != scene->mEffectsStillOn.end(); it++) {
            auto effect = *it;
            ui::Text("%s", effect->getName().c_str());
            ui::SameLine();
            if (ui::Button("Add to effects off")) {
                scene->addEffectOff(effect);
                effectAddedToOff = true;
            }
            ui::PopID();
            effectStillOnId++;
        }
        ui::ListBoxFooter();
        if (effectAddedToOff) {
            mSceneList->updateEffectsStillOn();
        }

        if (ui::Button("Done")) {
            scene = nullptr;
        }
    }

}
