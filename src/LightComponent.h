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

struct Command;

struct LightComponentDefintion;

class LightComponentGui;
typedef std::shared_ptr<LightComponentGui> LightComponentGuiRef;

class LightComponent {
public:
    static LightComponentRef create(LightComponentDefintion definition, int fixtureChannel);
    virtual void updateDmx(DmxOutput *dmxOutput);
    virtual LightComponentGuiRef getGui();

    LightComponent(LightComponentDefintion definition, int fixtureChannel);
    void setFixtureChannel(int channel);
    int getChannel();
    std::string getName();

    virtual float getStoreValue();
    virtual void restoreFromStoreValue(float value);

    std::string id;
    std::string controlledBy;

protected:
    int mFixtureChannel;
    int mComponentChannel;
    std::string mName;
};

struct LightComponentDefintion
{
    int componentChannel;
    std::string type;
    std::string name;
    std::string id;
    std::map<std::string, Command> commands;
};

class PanComponent: public LightComponent {
public:
    using LightComponent::LightComponent;
    void updateDmx(DmxOutput *dmxOutput) override;
    void setPanning(float panning);
    float getPanning();
    LightComponentGuiRef getGui() override;

    float getStoreValue() override;
    void restoreFromStoreValue(float value) override;

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

    float getStoreValue() override;
    void restoreFromStoreValue(float value) override;


protected:
    float mTilt = 0;
    float mLimit = 27.0f;
};

struct Command {
    std::string name;
    int min;
    int max;
};

class CommandComponent: public LightComponent {
public:
    CommandComponent(LightComponentDefintion definition, int fixtureChannel);
    void updateDmx(DmxOutput *dmxOutput) override;
    void execute(std::string command, int value);
    void execute(std::string command);
    Command getCurrentCommand();
    int getCurrentCommandIndex();
    int getCurrentValue();
    std::vector<std::string> getAvailableCommands();
    LightComponentGuiRef getGui() override;

    float getStoreValue() override;
    void restoreFromStoreValue(float value) override;

protected:
    std::map<std::string, Command> mCommands;
    std::vector<std::string> mAvailableCommands;
    Command mCurrentCommand;
    int mCurrentCommandIndex;
    int mCurrentValue;

    void updateCurrentCommandIndex();
};

class ChannelComponent: public LightComponent {
public:
    using LightComponent::LightComponent;
    void updateDmx(DmxOutput *dmxOutput) override;
    void setValue(int value);
    int getValue();
    LightComponentGuiRef getGui() override;

    float getStoreValue() override;
    void restoreFromStoreValue(float value) override;


protected:
    int mValue = 0;
};

//////////////////////////////////////////////////
// Gui
//////////////////////////////////////////////////
class LightComponentGui {
public:
    explicit LightComponentGui(LightComponent& component);
    virtual void draw(int id);

protected:
    LightComponent& mComponent;
};

class PanComponentGui: public LightComponentGui {
public:
    using LightComponentGui::LightComponentGui;
    void draw(int id) override;
};

class TiltComponentGui: public LightComponentGui {
public:
    using LightComponentGui::LightComponentGui;
    void draw(int id) override;
};

class CommandComponentGui: public LightComponentGui {
public:
    using LightComponentGui::LightComponentGui;
    void draw(int id) override;
};

class ChannelComponentGui: public LightComponentGui {
public:
    using LightComponentGui::LightComponentGui;
    void draw(int id) override;
};

//////////////////////////////////////////////////
// End Gui
//////////////////////////////////////////////////




#endif //PHOTONICDIRECTOR_LIGHTCOMPONENT_H
