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

LightComponent::LightComponent(int componentChannel, int fixtureChannel)
: mComponentChannel(componentChannel), mFixtureChannel(fixtureChannel)
{
}

void PanComponent::updateDmx(DmxOutput *dmxOutput) {
    int firstChannel = mFixtureChannel + mComponentChannel - 1;
    int secondChannel = firstChannel + 1;
    dmxOutput->setChannelValue(firstChannel, 200);
    dmxOutput->setChannelValue(secondChannel, 100);
}

void TiltComponent::updateDmx(DmxOutput *dmxOutput) {
    cinder::app::console() << "Tilt component updatedmx called";
}
