//
// Created by Jur de Vries on 04/09/2018.
//

#ifndef PHOTONICDIRECTOR_LIGHTCOMPONENT_H
#define PHOTONICDIRECTOR_LIGHTCOMPONENT_H

#include "Output.h"
#include "cinder/app/App.h"
#include "CinderImGui.h"
#include "cinder/CinderMath.h"


class LightComponent;
typedef std::shared_ptr<LightComponent> LightComponentRef;

struct LightComponentDefintion;

class LightComponentGui;
typedef std::shared_ptr<LightComponentGui> LightComponentGuiRef;

class LightComponent {
public:
    static LightComponentRef create(LightComponentDefintion definition, int fixtureChannel);
    virtual void updateDmx(DmxOutput *dmxOutput);
    virtual LightComponentGuiRef getGui();

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
    void setPanning(float panning);
    float getPanning();
    LightComponentGuiRef getGui() override;

protected:
    float mPanning = 0;
};

class TiltComponent: public LightComponent {
public:
    using LightComponent::LightComponent;
    void updateDmx(DmxOutput *dmxOutput) override;
    void setTilt(float tilt);
    float getTilt();
    LightComponentGuiRef getGui() override;

protected:
    float mTilt = 0;
};


//////////////////////////////////////////////////
// Gui
//////////////////////////////////////////////////
class LightComponentGui {
public:
    explicit LightComponentGui(LightComponent& component);
    virtual void draw();

protected:
    LightComponent& mComponent;
};

class PanComponentGui: public LightComponentGui {
public:
    using LightComponentGui::LightComponentGui;
    void draw() override;
};

class TiltComponentGui: public LightComponentGui {
public:
    using LightComponentGui::LightComponentGui;
    void draw() override;
};


//////////////////////////////////////////////////
// End Gui
//////////////////////////////////////////////////




#endif //PHOTONICDIRECTOR_LIGHTCOMPONENT_H
