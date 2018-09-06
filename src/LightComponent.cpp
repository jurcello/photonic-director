//
// Created by Jur de Vries on 04/09/2018.
//

#include "LightComponent.h"

LightComponentRef LightComponent::create(LightComponentDefintion definition, int fixtureChannel) {
    if (definition.type == "tilt") {
        return LightComponentRef(new TiltComponent(definition, fixtureChannel));
    }
    else if  (definition.type == "pan") {
        return LightComponentRef(new PanComponent(definition, fixtureChannel));
    }
    else if  (definition.type == "command") {
        return LightComponentRef(new CommandComponent(definition, fixtureChannel));
    }
    return LightComponentRef();
}

void LightComponent::setFixtureChannel(int channel) {
    mFixtureChannel = channel;
}

int LightComponent::getChannel() {
    return mFixtureChannel + mComponentChannel - 1;
}

std::string LightComponent::getName() {
    return mName;
}

void LightComponent::updateDmx(DmxOutput *dmxOutput) {

}

LightComponentGuiRef LightComponent::getGui() {
    return LightComponentGuiRef(new LightComponentGui(*this));
}

LightComponent::LightComponent(LightComponentDefintion definition, int fixtureChannel)
: mComponentChannel(definition.componentChannel), mFixtureChannel(fixtureChannel), mName(definition.name)
{
}

//////////// Pan ////////////

void PanComponent::updateDmx(DmxOutput *dmxOutput) {
    int courseChannel = getChannel();
    int fineChannel = courseChannel + 1;
    float course = (mPanning / 440) * 256;
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

//////////// Tilt ////////////
void TiltComponent::updateDmx(DmxOutput *dmxOutput) {
    int courseChannel = getChannel();
    int fineChannel = courseChannel + 1;
    float course = (mTilt / 306) * 256;
    float fine = (course - cinder::math<float>::floor(course)) * 256;
    dmxOutput->setChannelValue(courseChannel, (int) std::floor(course));
    dmxOutput->setChannelValue(fineChannel, (int) std::floor(fine));

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

//////////// Command ////////////
CommandComponent::CommandComponent(LightComponentDefintion definition, int fixtureChannel)
: LightComponent(definition,fixtureChannel), mCurrentValue(0)
{
    mCommands = definition.commands;
    for (auto command : mCommands) {
        mAvailableCommands.push_back(command.first);
    }
}

void CommandComponent::updateDmx(DmxOutput *dmxOutput) {
    dmxOutput->setChannelValue(getChannel(), mCurrentValue);
}

void CommandComponent::execute(std::string command) {
    mCurrentCommand = mCommands.at(command);
    this->execute(command, mCurrentCommand.min);
}

void CommandComponent::execute(std::string command, int value) {
    mCurrentCommand = mCommands.at(command);
    mCurrentValue = value;
    mCurrentCommandIndex = 0;
    for (auto command : mAvailableCommands) {
        if (command == mCurrentCommand.name) {
            break;
        }
        mCurrentCommandIndex++;
    }
}

Command CommandComponent::getCurrentCommand() {
    return mCurrentCommand;
}

int CommandComponent::getCurrentCommandIndex() {
    return mCurrentCommandIndex;
}

int CommandComponent::getCurrentValue() {
    return mCurrentValue;
}

std::vector<std::string> CommandComponent::getAvailableCommands() {
    return mAvailableCommands;
}

LightComponentGuiRef CommandComponent::getGui() {
    return LightComponentGuiRef(new CommandComponentGui(*this));
}

//////////////////////////////////////////////////
// Gui
//////////////////////////////////////////////////
LightComponentGui::LightComponentGui(LightComponent& component)
: mComponent(component)
{
}

void LightComponentGui::draw(int id) {
    ui::Text("%s", mComponent.getName().c_str());
}


void PanComponentGui::draw(int id) {
    LightComponentGui::draw(id);
    PanComponent& component = (PanComponent&) mComponent;
    static float panning;
    panning = component.getPanning();
    ui::PushID(id);
    if (ui::SliderFloat("Pan (Degrees)", &panning, 0.0f, 440.0f)) {
        component.setPanning(panning);
    }
    ui::PopID();
}

void TiltComponentGui::draw(int id) {
    LightComponentGui::draw(id);
    TiltComponent& component = (TiltComponent&) mComponent;
    static float tilt;
    tilt = component.getTilt();
    ui::PushID(id);
    if (ui::SliderFloat("Tilt (Degrees)", &tilt, 0.0f, 306.0f)) {
        component.setTilt(tilt);
    }
    ui::PopID();
}

void CommandComponentGui::draw(int id) {
    LightComponentGui::draw(id);
    CommandComponent& component = (CommandComponent&) mComponent;
    static int currentCommandIndex;
    currentCommandIndex = component.getCurrentCommandIndex();
    std::vector<std::string> availableCommands = component.getAvailableCommands();
    ui::PushID(id);
    if (ui::Combo("Command", &currentCommandIndex, availableCommands)) {
        component.execute(availableCommands[currentCommandIndex]);
    }
    Command currentCommand = component.getCurrentCommand();
    if (currentCommand.min != currentCommand.max) {
        static int currentValue;
        currentValue = component.getCurrentValue();
        if (ui::SliderInt("Value", &currentValue, currentCommand.min, currentCommand.max)) {
            component.execute(availableCommands[currentCommandIndex], currentValue);
        }
    }

    ui::PopID();
}


//////////////////////////////////////////////////
// End Gui
//////////////////////////////////////////////////