//
// Created by Jur de Vries on 04/09/2018.
//

#ifndef PHOTONICDIRECTOR_LIGHTCOMPONENT_H
#define PHOTONICDIRECTOR_LIGHTCOMPONENT_H

#include "Output.h"
#include "cinder/app/App.h"


class LightComponent;
typedef std::shared_ptr<LightComponent> LightComponentRef;

struct LightComponentDefintion;


class LightComponent {
public:
    static LightComponentRef create(LightComponentDefintion definition, int fixtureChannel);
    virtual void updateDmx(DmxOutput *dmxOutput) = 0;

    LightComponent(int componentChannel, int fixtureChannel);
    void setFixtureChannel(int channel);

protected:
    int mFixtureChannel;
    int mComponentChannel;
};

struct LightComponentDefintion
{
    int componentChannel;
    std::string type;
};

class PanComponent: public LightComponent {
public:
    using LightComponent::LightComponent;
    void updateDmx(DmxOutput *dmxOutput) override;
};

class TiltComponent: public LightComponent {
public:
    using LightComponent::LightComponent;
    void updateDmx(DmxOutput *dmxOutput) override;
};

#endif //PHOTONICDIRECTOR_LIGHTCOMPONENT_H
