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
    else if  (definition.type == "channel") {
        return LightComponentRef(new ChannelComponent(definition, fixtureChannel));
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

float LightComponent::getStoreValue() {
    return 0;
}

void LightComponent::restoreFromStoreValue(float value) {}

void LightComponent::updateDmx(DmxOutput *dmxOutput) {

}

LightComponentGuiRef LightComponent::getGui() {
    return LightComponentGuiRef(new LightComponentGui(*this));
}

LightComponent::LightComponent(LightComponentDefintion definition, int fixtureChannel)
: mComponentChannel(definition.componentChannel), mFixtureChannel(fixtureChannel), mName(definition.name), id(definition.id)
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

float PanComponent::getStoreValue() {
    return mPanning;
}

void PanComponent::restoreFromStoreValue(float value) {
    mPanning = value;
}

//////////// Tilt ////////////
void TiltComponent::updateDmx(DmxOutput *dmxOutput) {
    int courseChannel = getChannel();
    int fineChannel = courseChannel + 1;
    float course = ((mTilt - mLimit) / 306) * 256;
    float fine = (course - cinder::math<float>::floor(course)) * 256;
    dmxOutput->setChannelValue(courseChannel, (int) std::floor(course));
    dmxOutput->setChannelValue(fineChannel, (int) std::floor(fine));

}

void TiltComponent::setTilt(float tilt) {
    if (tilt < mLimit) {
        mTilt = mLimit;
        return;
    }
    if (tilt > 360.0f - mLimit) {
        mTilt = 360.f - mLimit;
        return;
    }
    mTilt = tilt;
}

float TiltComponent::getTilt() {
    return mTilt;
}

LightComponentGuiRef TiltComponent::getGui() {
    return LightComponentGuiRef(new TiltComponentGui(*this));
}

float TiltComponent::getStoreValue() {
    return mTilt;
}

void TiltComponent::restoreFromStoreValue(float value) {
    mTilt = value;
}

//////////// Command ////////////
CommandComponent::CommandComponent(LightComponentDefintion definition, int fixtureChannel)
: LightComponent(definition,fixtureChannel), mCurrentValue(0)
{
    mCommands = definition.commands;
    for (auto command : mCommands) {
        mAvailableCommands.push_back(command.first);
        if (command.second.min == 0) {
            mCurrentCommand = mCommands.at(command.first);
        }
    }
    updateCurrentCommandIndex();
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
    updateCurrentCommandIndex();
}

void CommandComponent::updateCurrentCommandIndex() {
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


float CommandComponent::getStoreValue() {
    return (float) mCurrentValue;
}

void CommandComponent::restoreFromStoreValue(float value) {
    int intValue = (int) value;
    mCurrentValue = intValue;
    for (const auto item: mAvailableCommands) {
      Command command = mCommands.at(item);
      if (intValue >= command.min && intValue <= command.max) {
          mCurrentCommand = command;
          break;
      }
    }
    updateCurrentCommandIndex();
}

void ChannelComponent::updateDmx(DmxOutput *dmxOutput) {
    dmxOutput->setChannelValue(getChannel(), mValue);
}

void ChannelComponent::setValue(int value) {
    mValue = value;
}

int ChannelComponent::getValue() {
    return mValue;
}

LightComponentGuiRef ChannelComponent::getGui() {
    return LightComponentGuiRef(new ChannelComponentGui(*this));
}


//////////////////////////////////////////////////
// Gui
//////////////////////////////////////////////////
LightComponentGui::LightComponentGui(LightComponent& component)
: mComponent(component)
{
}

float ChannelComponent::getStoreValue() {
    return mValue;
}

void ChannelComponent::restoreFromStoreValue(float value) {
    mValue = value;
}

void LightComponentGui::draw(int id) {
    ui::Text("%s", mComponent.getName().c_str());
    if (mComponent.controlledBy != "") {
        ui::SameLine();
        ui::Text(" (Controlled by %s effect)", mComponent.controlledBy.c_str());
    }

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
    if (ui::SliderFloat("Tilt (Degrees)", &tilt, 27.0f, 333.0f)) {
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
//    if (ui::Combo("Command", &currentCommandIndex, availableCommands)) {
//        component.execute(availableCommands[currentCommandIndex]);
//    }
    if (ui::BeginCombo("Command", availableCommands.at(currentCommandIndex).c_str())) {
        for (int index = 0; index < availableCommands.size(); index++) {
            bool is_selected = (currentCommandIndex == index);
            if (ui::Selectable(availableCommands[index].c_str(), is_selected)) {
                component.execute(availableCommands[currentCommandIndex]);
            }
            if (is_selected) {
                ui::SetItemDefaultFocus();
            }
        }
        ui::EndCombo();
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

void ChannelComponentGui::draw(int id) {
    LightComponentGui::draw(id);
    ChannelComponent& component = (ChannelComponent&) mComponent;
    static int value;
    value = component.getValue();
    if (ui::SliderInt("Value", &value, 0, 255)) {
        component.setValue(value);
    }
}

//////////////////////////////////////////////////
// End Gui
//////////////////////////////////////////////////