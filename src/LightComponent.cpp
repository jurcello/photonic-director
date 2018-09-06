//
// Created by Jur de Vries on 04/09/2018.
//

#include "LightComponent.h"

LightComponentRef LightComponent::create(LightComponentDefintion definition, int fixtureChannel) {
    if (definition.type == "tilt") {
        return LightComponentRef(new TiltComponent(definition.componentChannel, fixtureChannel));
    }
    else if  (definition.type == "pan") {
        return LightComponentRef(new PanComponent(definition.componentChannel, fixtureChannel));
    }
    return LightComponentRef();
}

void LightComponent::setFixtureChannel(int channel) {
    mFixtureChannel = channel;
}

void LightComponent::updateDmx(DmxOutput *dmxOutput) {

}

LightComponentGuiRef LightComponent::getGui() {
    return LightComponentGuiRef(new LightComponentGui(*this));
}

LightComponent::LightComponent(int componentChannel, int fixtureChannel)
: mComponentChannel(componentChannel), mFixtureChannel(fixtureChannel)
{
}

void PanComponent::updateDmx(DmxOutput *dmxOutput) {
    int courseChannel = mFixtureChannel + mComponentChannel - 1;
    int fineChannel = courseChannel + 1;
    float course = (mPanning / 360) * 256;
    float fine = (course - cinder::math<float>::floor(course)) * 256;
    dmxOutput->setChannelValue(courseChannel, (int) std::floor(course));
    dmxOutput->setChannelValue(fineChannel, (int) std::floor(fine));
}

LightComponentGuiRef PanComponent::getGui() {
    return LightComponentGuiRef(new PanComponentGui(*this));
}

void PanComponent::setPanning(float panning) {
    mPanning = panning;
}

float PanComponent::getPanning() {
    return mPanning;
}

void TiltComponent::updateDmx(DmxOutput *dmxOutput) {
}

void TiltComponent::setTilt(float tilt) {
    mTilt = tilt;
}

float TiltComponent::getTilt() {
    return mTilt;
}

LightComponentGuiRef TiltComponent::getGui() {
    return LightComponentGuiRef(new TiltComponentGui(*this));
}


//////////////////////////////////////////////////
// Gui
//////////////////////////////////////////////////
LightComponentGui::LightComponentGui(LightComponent& component)
: mComponent(component)
{
}

void LightComponentGui::draw() {
}


void PanComponentGui::draw() {
    PanComponent& component = (PanComponent&) mComponent;
    ui::Text("This is the pan component gui.");
    static float panning;
    panning = component.getPanning();
    if (ui::SliderFloat("Pan (Degrees)", &panning, 0.0f, 360.0f)) {
        component.setPanning(panning);
    }
}

void TiltComponentGui::draw() {
    auto component = (TiltComponent&) mComponent;
    ui::Text("This is the tilt component gui.");
    static float tilt;
    tilt = component.getTilt();
    if (ui::InputFloat("Tilt (Degrees)", &tilt)) {
        component.setTilt(tilt);
    }
}



//////////////////////////////////////////////////
// End Gui
//////////////////////////////////////////////////