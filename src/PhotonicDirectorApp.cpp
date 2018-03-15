#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/Log.h"
#include "CinderImGui.h"
#include "Light.h"
#include "LightCalibrator.h"
#include "Effects.h"
#include "ConfigManager.h"
#include "Visualizer.h"
#include "Osc.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ph;
using protocol = asio::ip::udp;

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

enum gui_status {
    IDLE,
    EDITING_LIGHT,
    ADDING_LIGHT_TO_EFFECT,
    CALIBRATING_LIGHT,
};

struct GuiStatusData {
    EffectRef pickLightEffect;
    LightRef lightToEdit;
    LightRef pickedLight;
    bool drawGui;
    bool visualizeEffects;
    
    gui_status status;
};


class PhotonicDirectorApp : public App {
public:
    
    PhotonicDirectorApp();

    virtual ~PhotonicDirectorApp();

    void setTheme(ImGui::Options &options);
    
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
    void update() override;
    void draw() override;
    void keyDown( KeyEvent event) override;
    void resize() override;
    
    void save();
    void load();
    
    void pickLight();

    void setupOsc(int receivePort, int sendPort);
    
protected:
    ConfigManager config;
    
    vector<LightRef> mLights;
    LightFactory mLightFactory;
    LightCalibrator mLightCalibrator;
    
    // Visualizer.
    Visualizer mVisualizer;
    
    // Osc related stuff.
    osc::ReceiverUdp* mOscReceiver;
    osc::SenderUdp*	mOscSender;
    osc::UdpSocketRef	mOscSocket;
    int mOscReceivePort;
    int mOscSendPort;

    // Channel stuff.
    vector<InputChannelRef> mChannels;
    
    // Effects.
    vector<EffectRef> mEffects;
    double mLastUpdate;
    
    void oscReceive(const osc::Message &message);
    void handleLightOscSending();
    // Gui stuff.
    void drawGui();
    void drawChannelControls();
    void drawLightControls();
    void drawEffectControls();
    void drawDmxInspector();
    GuiStatusData mGuiStatusData;

    // Dmx output.
    DmxOutput mDmxOut;
    
};

PhotonicDirectorApp::PhotonicDirectorApp()
: mOscReceiver(nullptr),
  mOscSender(nullptr),
  mOscSocket(nullptr),
  mOscReceivePort(10000),
  mOscSendPort(10001),
  mLightFactory(&mDmxOut)
{
    mGuiStatusData.pickLightEffect = nullptr;
    mGuiStatusData.lightToEdit = nullptr;
    mGuiStatusData.pickedLight = nullptr;
    mGuiStatusData.drawGui = true;
    mGuiStatusData.visualizeEffects = false;
    
    mGuiStatusData.status = IDLE;
}

void PhotonicDirectorApp::setTheme(ImGui::Options &options) {
    options.childWindowRounding(3.f);
    options.grabRounding(0.f);
    options.windowRounding(0.f);
    options.scrollbarRounding(3.f);
    options.frameRounding(3.f);
    options.windowTitleAlign(ImVec2(0.5f,0.5f));
    
    
    options.color(ImGuiCol_Text,                  ImVec4(0.73f, 0.73f, 0.73f, 1.00f));
    options.color(ImGuiCol_TextDisabled,          ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
    options.color(ImGuiCol_WindowBg,              ImVec4(0.26f, 0.26f, 0.26f, 0.95f));
    options.color(ImGuiCol_ChildWindowBg,         ImVec4(0.28f, 0.28f, 0.28f, 1.00f));
    options.color(ImGuiCol_PopupBg,               ImVec4(0.26f, 0.26f, 0.26f, 1.00f));
    options.color(ImGuiCol_Border,                ImVec4(0.26f, 0.26f, 0.26f, 1.00f));
    options.color(ImGuiCol_BorderShadow,          ImVec4(0.26f, 0.26f, 0.26f, 1.00f));
    options.color(ImGuiCol_FrameBg,               ImVec4(0.16f, 0.16f, 0.16f, 1.00f));
    options.color(ImGuiCol_FrameBgHovered,        ImVec4(0.16f, 0.16f, 0.16f, 1.00f));
    options.color(ImGuiCol_FrameBgActive,         ImVec4(0.16f, 0.16f, 0.16f, 1.00f));
    options.color(ImGuiCol_TitleBg,               ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_TitleBgCollapsed,      ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_TitleBgActive,         ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_MenuBarBg,             ImVec4(0.26f, 0.26f, 0.26f, 1.00f));
    options.color(ImGuiCol_ScrollbarBg,           ImVec4(0.21f, 0.21f, 0.21f, 1.00f));
    options.color(ImGuiCol_ScrollbarGrab,         ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_ScrollbarGrabHovered,  ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_ScrollbarGrabActive,   ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_ComboBg,               ImVec4(0.32f, 0.32f, 0.32f, 1.00f));
    options.color(ImGuiCol_CheckMark,             ImVec4(0.78f, 0.78f, 0.78f, 1.00f));
    options.color(ImGuiCol_SliderGrab,            ImVec4(0.74f, 0.74f, 0.74f, 1.00f));
    options.color(ImGuiCol_SliderGrabActive,      ImVec4(0.74f, 0.74f, 0.74f, 1.00f));
    options.color(ImGuiCol_Button,                ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_ButtonHovered,         ImVec4(0.43f, 0.43f, 0.43f, 1.00f));
    options.color(ImGuiCol_ButtonActive,          ImVec4(0.11f, 0.11f, 0.11f, 1.00f));
    options.color(ImGuiCol_Header,                ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_HeaderHovered,         ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_HeaderActive,          ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_Column,                ImVec4(0.39f, 0.39f, 0.39f, 1.00f));
    options.color(ImGuiCol_ColumnHovered,         ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    options.color(ImGuiCol_ColumnActive,          ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    options.color(ImGuiCol_ResizeGrip,            ImVec4(0.36f, 0.36f, 0.36f, 1.00f));
    options.color(ImGuiCol_ResizeGripHovered,     ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    options.color(ImGuiCol_ResizeGripActive,      ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    options.color(ImGuiCol_CloseButton,           ImVec4(0.59f, 0.59f, 0.59f, 1.00f));
    options.color(ImGuiCol_CloseButtonHovered,    ImVec4(0.98f, 0.39f, 0.36f, 1.00f));
    options.color(ImGuiCol_CloseButtonActive,     ImVec4(0.98f, 0.39f, 0.36f, 1.00f));
    options.color(ImGuiCol_PlotLines,             ImVec4(0.39f, 0.39f, 0.39f, 1.00f));
    options.color(ImGuiCol_PlotLinesHovered,      ImVec4(1.00f, 0.43f, 0.35f, 1.00f));
    options.color(ImGuiCol_PlotHistogram,         ImVec4(0.90f, 0.70f, 0.00f, 1.00f));
    options.color(ImGuiCol_PlotHistogramHovered,  ImVec4(1.00f, 0.60f, 0.00f, 1.00f));
    options.color(ImGuiCol_TextSelectedBg,        ImVec4(0.32f, 0.52f, 0.65f, 1.00f));
    options.color(ImGuiCol_ModalWindowDarkening,  ImVec4(0.20f, 0.20f, 0.20f, 0.50f));
}

void PhotonicDirectorApp::setup()
{
    ImGui::Options options;
    setTheme(options);
    ImGui::initialize(options);
    ImGui::connectWindow(getWindow());
    
    // Setup some initial lights.
    mLights.push_back(mLightFactory.create(vec3(2.0f, 2.0f, 0.0f), nullptr, std::string()));
    mLights.push_back(mLightFactory.create(vec3(1.0f, 1.0f, 1.0f), nullptr, std::string()));
    mLights.push_back(mLightFactory.create(vec3(3.0f, 2.0f, -1.0f), nullptr, std::string()));

    // Setup visualizer.
    mVisualizer.setup(mLights);
    
    // Initialize params.
    setupOsc(mOscReceivePort, mOscSendPort);
    mLightCalibrator.setLights(&mLights);

    mLastUpdate = getElapsedSeconds();
}

void PhotonicDirectorApp::setupOsc(int receivePort, int sendPort)
{
    if (mOscReceiver) {
        mOscReceiver->close();
        delete mOscReceiver;
    }
    if (mOscSender) {
        try {
            mOscSender->close();
            delete mOscSender;
        }
        catch (Exception exception){
            console() << "An exception occured closing the connection";
        }

    }
    if (mOscSocket) {
        mOscSocket = nullptr;
    }
    mOscReceiver = new osc::ReceiverUdp(receivePort);
    // Setup osc to listen to all addresses.
    mOscReceiver->setListener("/*",[&](const osc::Message &message){
        oscReceive(message);
    });
    try {
        mOscReceiver->bind();
    }
    catch (const osc::Exception &ex) {
        CI_LOG_E("Error binding: " << ex.what() << ", val:" << ex.value());
    }
    mOscReceiver->listen([&](asio::error_code error, protocol::endpoint endpoint) -> bool {
        if (error) {
            if (error.value() != 89)
                CI_LOG_E("Error listening: " << error.message() << ", val: " << error.value() << ", endpoint: " << endpoint);
            return false;
        }
        else {
            return true;
        }
    });

    //////////////////////////
    // Setup sender.
    //////////////////////////
    // Us a local port of 31,000 because that one most probably is not used.
    mOscSocket = osc::UdpSocketRef(new protocol::socket(App::get()->io_service(), protocol::endpoint( protocol::v4(), 31000 ) ));
    mOscSocket->set_option( asio::socket_base::broadcast(true) );
    mOscSender = new osc::SenderUdp( mOscSocket, protocol::endpoint( asio::ip::address_v4::broadcast(), sendPort ) );
    mLightCalibrator.setOscSender(mOscSender);

}

void PhotonicDirectorApp::oscReceive(const osc::Message &message)
{
    if (mChannels.size() > 0) {
        for (InputChannelRef channel : mChannels) {
            if (message.getAddress() == channel->getAddress()) {
                // Gather the args. There can be up to 3 args.
                int numArgs = message.getNumArgs();
                float arg1, arg2, arg3;
                if (message.getNumArgs() >= 1) {
                    arg1 = message.getArgFloat(0);
                    channel->setValue(arg1);
                    channel->setType(InputChannel::Type::kType_Dim1);
                    if (numArgs >= 2) {
                        arg2 = message.getArgFloat(1);
                        channel->setValue(vec2(arg1, arg2));
                        channel->setType(InputChannel::Type::kType_Dim2);
                        if (numArgs >= 3) {
                            arg3 = message.getArgFloat(2);
                            channel->setValue(vec3(arg1, arg2, arg3));
                            channel->setType(InputChannel::Type::kType_Dim3);
                        }
                    }
                }
            }
        }
    }
    mLightCalibrator.receiveOscMessage(message);
}

void PhotonicDirectorApp::mouseDown( MouseEvent event )
{
    mVisualizer.mouseDown(event);
    switch (mGuiStatusData.status) {
        case ADDING_LIGHT_TO_EFFECT:
            if (mGuiStatusData.pickedLight != nullptr && mGuiStatusData.pickLightEffect != nullptr) {
                mGuiStatusData.pickLightEffect->toggleLight(mGuiStatusData.pickedLight);
            }
            break;
            
        default:
            if (mGuiStatusData.pickedLight != nullptr) {
                mGuiStatusData.lightToEdit = mGuiStatusData.pickedLight;
            }
            break;
    }
}

void PhotonicDirectorApp::mouseUp(cinder::app::MouseEvent event)
{
    mVisualizer.mouseUp(event);
}

void PhotonicDirectorApp::mouseDrag(cinder::app::MouseEvent event)
{
    mVisualizer.mouseDrag(event);
}

void PhotonicDirectorApp::mouseMove(cinder::app::MouseEvent event)
{
    mVisualizer.mouseMove(event);
}

void PhotonicDirectorApp::keyDown( KeyEvent event)
{
    if (event.getChar() == KeyEvent::KEY_ESCAPE)
        exit(0);
    if (event.getChar() == 'g') {
        mGuiStatusData.drawGui = !mGuiStatusData.drawGui;
    }
}

void PhotonicDirectorApp::save()
{
    std::vector<string> extensions {"xml"};
    fs::path savePath = getSaveFilePath(fs::path(), extensions);
    if (! savePath.empty()) {
        config.startNewDoc();
        config.writeLights(mLights);
        config.writeChannels(mChannels);
        config.writeEffects(mEffects);
        config.writeInt("oscPort", mOscReceivePort);
        config.writeToFile(savePath);
    }
}

void PhotonicDirectorApp::load()
{
    std::vector<string> extensions {"xml"};
    fs::path loadPath = getOpenFilePath(fs::path(), extensions);
    if (! loadPath.empty()) {
        config.readFromFile(loadPath);
        int oscPort = config.readInt("oscPort");
        if (oscPort > 0) {
            mOscReceivePort = oscPort;
            setupOsc(mOscReceivePort, 0);
        }
        // Reset the channelRegistry of the dmx out.
        mDmxOut.clearRegistry();
        config.readLights(mLights, &mLightFactory);
        config.readChannels(mChannels);
        config.readEffects(mEffects, mLights, mChannels);
    }
}

void PhotonicDirectorApp::update()
{
    pickLight();
    
    if (mGuiStatusData.drawGui) {
        drawGui();
    }
    
    double now = getElapsedSeconds();
    double dt = now - mLastUpdate;
    /////////////////////////////////////////////
    // Effect handling.
    /////////////////////////////////////////////
    // Main stage.
    for (auto effect : mEffects) {
        if (effect->getStage() == Effect::Stage::kStage_Main)
        {
            effect->execute(dt);
        }
    }
    // Prepare DMX output.
    mDmxOut.reset();
    
    /////////////////////////////////////////////
    // Light handling: Main stage.
    /////////////////////////////////////////////
    for (const auto light : mLights) {
        if (mLightCalibrator.isCalibrating()) {
            const float calibratedLightIntensity = static_cast<const float>(((sin(getElapsedSeconds()) + 1.0) / 4) + 0.5f);
            light->intensity = (light == mLightCalibrator.getCurrentLight()) ? calibratedLightIntensity : 0.1f;
            light->color = light->getLightType()->editColor;
        } else {
            float endIntensity = 0.f;
            ColorA endColor(0.0f, 0.0f, 0.0f, 1.0f);
            float highestWeight = 1.0f;
            for (const auto effect : mEffects) {
                if (effect->hasOutput() && effect->hasLight(light)) {
                    const float effetcIntensity = light->getEffetcIntensity(effect->getUuid());
                    ////////////////////////////////////////////////////////
                    // Calculate the weight of the effect. The weight should be more if the intensity is more.
                    ////////////////////////////////////////////////////////
                    float effectWeight = 1.0f + effetcIntensity * effect->weight;
                    if (effectWeight > highestWeight) {
                        highestWeight = effectWeight;
                    }
                    endIntensity += effetcIntensity * effect->getFadeValue() * effectWeight;
                    endColor += light->getEffectColor(effect->getUuid()) * effetcIntensity * effect->getFadeValue() * effectWeight;
                }
            }
            endColor /= highestWeight;
            endIntensity /= highestWeight;
            light->intensity = endIntensity;
            // Be sure that the alfa channel is always 1.0.
            endColor.a = 1.0f;
            light->color = endColor;
        }
    }
    // After effects.
    for (auto effect : mEffects) {
        if (effect->getStage() == Effect::Stage::kStage_After)
        {
            effect->execute(dt);
        }
    }

    ///////////////////////////////////////////////////
    // Light handling: updating DMX data.
    ///////////////////////////////////////////////////
    for (const auto light : mLights) {
        light->update();
        light->updateDmx();
    }
    mDmxOut.update();
    handleLightOscSending();
    mLastUpdate = now;
}

void PhotonicDirectorApp::handleLightOscSending() {
    for (auto light : mLights) {
        if (light->mSendOsc && light->mOscAdress[0] == '/') {
            osc::Message message(light->mOscAdress + "/intensity");
            message.append(light->intensity);
            mOscSender->send(message);
            if (light->isColorEnabled()) {
                osc::Message messageR(light->mOscAdress + "/r");
                messageR.append(light->color.r);
                mOscSender->send(messageR);
                osc::Message messageG(light->mOscAdress + "/g");
                messageG.append(light->color.g);
                mOscSender->send(messageG);
                osc::Message messageB(light->mOscAdress + "/b");
                messageB.append(light->color.b);
                mOscSender->send(messageB);
            }
        }
    }
}

void PhotonicDirectorApp::drawGui()
{
    static bool showLightEditor = true;
    static bool showChannelEditor = true;
    static bool showEffectEditor = true;
    static bool showDmxInspector = false;
    // Draw the general ui.
    ImGui::ScopedWindow window("Controls");
    
    ui::Separator();
    ui::Text("Osc settings");
    ui::Spacing();
    if  (ui::InputInt("Osc Receive Port", &mOscReceivePort)) {
        setupOsc(mOscReceivePort, mOscSendPort);
    }
    if  (ui::InputInt("Osc Send Port", &mOscSendPort)) {
        setupOsc(mOscReceivePort, mOscSendPort);
    }

    ui::Separator();
    ui::Text("Dmx settings");
    if (! ui::IsWindowCollapsed()) {
        if (! mDmxOut.isConnected()) {
            auto devices = mDmxOut.getDevicesList();
            ui::ListBoxHeader("Choose device", devices.size());
            for (auto device : devices) {
                if (ui::Selectable(device.c_str())) {
                    mDmxOut.connect(device);
                }
            }
            ui::ListBoxFooter();
        }
        else {
            ui::Text("Connected to: ");
            const std::string deviceInfo = mDmxOut.getConnectedDevice();
            ui::Text("%s", deviceInfo.c_str());
            ui::SameLine();
            if (ui::Button("Disconnect")) {
                mDmxOut.disConnect();
            }
        }
    }
    ui::Separator();
    ui::Text("Calibrator");
    if (ui::Button("Start calibration")) {
        mLightCalibrator.start();
    }
    if (mLightCalibrator.isCalibrating()) {
        ui::Text("Now calibrating: %s", mLightCalibrator.getCurrentLight()->mName.c_str());
        ui::InputFloat3("Position: ", &mLightCalibrator.currentPosition[0]);
    }
    ui::Separator();
    ui::Checkbox("Visualize effects", &mGuiStatusData.visualizeEffects);
    ui::Separator();
    ui::Text("Widgets");
    ui::Checkbox("Show light editor", &showLightEditor);
    ui::Checkbox("Show channel editor", &showChannelEditor);
    ui::Checkbox("Show effect editor", &showEffectEditor);
    ui::Checkbox("Show DMX inspector", &showDmxInspector);
    ui::Separator();
    ui::Text("File");
    ui::Spacing();
    if (ui::Button("Load")) {
        load();
    }
    ui::SameLine();
    if (ui::Button("Save")) {
        save();
    }
    // Draw the channel window if there are channels.
    if (showChannelEditor) {
        drawChannelControls();
    }
    if (showEffectEditor) {
        drawEffectControls();
    }
    if (showLightEditor) {
        drawLightControls();
    }
    if (showDmxInspector) {
        drawDmxInspector();
    }
}

void PhotonicDirectorApp::drawLightControls()
{
    ImGui::ScopedWindow window("Lights");
    static bool editingMode = false;
    ui::Checkbox("Edit mode", &editingMode);
    if (editingMode) {
        mVisualizer.enableEditingMode();
    }
    else {
        mVisualizer.disableEditingMode();
    }
    if (ui::Button("Add Light")) {
        ui::OpenPopup("Create Light");
    }
    if (ui::BeginPopupModal("Create Light")) {
        static std::string lightName;
        static int lightType = 0;
        static vec3 lightPosition;
        ui::InputText("Name", &lightName);
        ui::Combo("Type", &lightType, mLightFactory.getAvailableTypeNames());
        ui::InputFloat3("Position", &lightPosition[0]);

        if (ui::Button("Done")) {
            std::vector<LightType*> lightTypes = mLightFactory.getAvailableTypes();
            LightRef newLight = mLightFactory.create(lightPosition, lightTypes[lightType], std::string());
            newLight->mName = lightName;
            mLights.push_back(newLight);
            ui::CloseCurrentPopup();
        }
        ui::SameLine();
        if (ui::Button("Cancel")) {
            ui::CloseCurrentPopup();
        }
        ui::EndPopup();
    }

//    if (ui::Button("Add")) {
//        LightRef newLight = mLightFactory.create(vec3(1.0f));
//        mLights.push_back(newLight);
//        mGuiStatusData.lightToEdit = newLight;
//        mGuiStatusData.status = EDITING_LIGHT;
//    }
    if (mGuiStatusData.lightToEdit) {
        ui::SameLine();
        if (ui::Button("Remove")) {
            auto it = std::find(mLights.begin(), mLights.end(), mGuiStatusData.lightToEdit);
            if (it != mLights.end()) {
                // First remove the light from the effects.
                for (auto effect : mEffects) {
                    effect->removeLight(*it);
                }
                mLights.erase(it);
                mGuiStatusData.lightToEdit = nullptr;
            }
        }
    }
    if (! ui::IsWindowCollapsed()) {
        ui::ListBoxHeader("Edit lights");
        for (LightRef light: mLights) {
            if (ui::Selectable(light->mName.c_str(), mGuiStatusData.lightToEdit == light)) {
                mGuiStatusData.lightToEdit = light;
            }
        }
        ui::ListBoxFooter();
    }
    if (mGuiStatusData.lightToEdit) {
        ImGui::ScopedWindow lightEditWindow("Edit light");
        ui::InputText("Name", &mGuiStatusData.lightToEdit->mName);
        ui::SliderFloat("Intensity", &mGuiStatusData.lightToEdit->intensity, 0.f, 1.f);
        ui::ColorEdit4("Color", &mGuiStatusData.lightToEdit->color[0]);
        ui::DragFloat3("Position", &mGuiStatusData.lightToEdit->position[0]);
        ui::Spacing();
        ui::Text("Light type");
        ui::Separator();
        LightType* lightType = mGuiStatusData.lightToEdit->getLightType();
        std::string lightTypeName = "Type: " + lightType->name;
        ui::Text("%s", lightTypeName.c_str());
        int numChannels = mGuiStatusData.lightToEdit->getNumChannels();
        if (numChannels > 1) {
            std::string numberOfChannels = "Number of channels: " + std::to_string(numChannels);
            ui::Text("%s", numberOfChannels.c_str());
            std::string colorChannelPostion =
                    "Color channel position: " + std::to_string(mGuiStatusData.lightToEdit->getColorChannelPosition());
            ui::Text("%s", colorChannelPostion.c_str());
            std::string intensityChannelPosition =
                    "Intensity channel position: " + std::to_string(mGuiStatusData.lightToEdit->getIntensityChannelPosition());
            ui::Text("%s", intensityChannelPosition.c_str());
        }
        ui::Spacing();
        ui::Separator();
        ui::Text("DMX Settings");
        ui::Spacing();
        static int dmxChannel;
        dmxChannel = mGuiStatusData.lightToEdit->getDmxChannel();
        if (ui::InputInt("DMX channel", &dmxChannel, 0, 256)) {
            if (!(mGuiStatusData.lightToEdit->setDmxChannel(dmxChannel))) {
                ui::TextColored(Color(1.f, 0.f, 0.f), "The channel is already taken");
            }
        }
        ui::InputInt("Dmx offset", &mGuiStatusData.lightToEdit->mDmxOffsetIntentsityValue, 0, 255);
        ui::Checkbox("Send osc", &mGuiStatusData.lightToEdit->mSendOsc);
        if (mGuiStatusData.lightToEdit->mSendOsc) {
            ui::InputText("Osc address", &mGuiStatusData.lightToEdit->mOscAdress);
        }
        if (ui::Button("Done")) {
            mGuiStatusData.lightToEdit = nullptr;
            mGuiStatusData.status = IDLE;
        }
    }
}

void PhotonicDirectorApp::drawChannelControls()
{
    static const InputChannelRef* channelSelection = nullptr;
    {
        
        ImGui::ScopedWindow window("Channels");
        // Add the buttons.
        if (ui::Button("Add Channel")) {
            ui::OpenPopup("Create channel");
        }
        if (ui::BeginPopupModal("Create channel")) {
            static char channelName[64];
            ui::InputText("Name", channelName, 64);
            static char channelAddress[64];
            ui::InputText("Address", channelAddress, 64);
            if (ui::Button("Done")) {
                mChannels.push_back(InputChannel::create(channelName, channelAddress));
                ui::CloseCurrentPopup();
            }
            ui::EndPopup();
        }
        if (channelSelection) {
            ui::SameLine();
            if (ui::Button("Remove")) {
                auto it = std::find_if(mChannels.begin(), mChannels.end(), [](const InputChannelRef& channel){ return &channel == channelSelection; });
                if (it != mChannels.end()) {
                    mChannels.erase(it);
                    channelSelection = nullptr;
                }
            }
        }
        if (! ui::IsWindowCollapsed()) {
            ui::ListBoxHeader("Edit channels");
            for (const InputChannelRef& channel : mChannels) {
                if (ui::Selectable(channel->getName().c_str(), channelSelection == &channel)) {
                    channelSelection = &channel;
                }
                ui::SameLine();
                float value = channel->getValue();
                ui::SliderFloat("", &value, 0.0f, 1.0f);
            }
            ui::ListBoxFooter();
        }
    }
    
    if (channelSelection != nullptr) {
        ImGui::ScopedWindow window("Channel inspector");
        
        InputChannelRef channel = *channelSelection;
        static std::string name;
        name = channel->getName();
        if (ui::InputText("Name", &name)) {
            channel->setName(name);
        }
        static std::string address;
        address = channel->getAddress();
        if (ui::InputText("Address", &address)) {
            channel->setAdrress(address);
        }
        if (ui::Button("Done")) {
            channelSelection = nullptr;
        }
    }
    
}

void PhotonicDirectorApp::drawEffectControls()
{
    
    // Effects ui.
    static EffectRef* effectSelection = nullptr;
    {
        ImGui::ScopedWindow window("Effects");
        // Add the button.
        if (ui::Button("Create Effect")) {
            ui::OpenPopup("Create effect");
        }
        if (ui::BeginPopupModal("Create effect")) {
            static std::string effectName;
            static int effectType = 0;
            ui::InputText("Name", &effectName);
            ui::Combo("Type", &effectType, Effect::getTypes());
            if (ui::Button("Done")) {
                std::string type = Effect::getTypes()[effectType];
                console() << effectName << ", " << type << endl;
                EffectRef newEffect = Effect::create(type, effectName);
                // TODO: Now add all lights. Later lights should be picked.
                for (auto light : mLights) {
                    newEffect->addLight(light);
                }
                mEffects.push_back(newEffect);
                ui::CloseCurrentPopup();
            }
            ui::EndPopup();
        }
        /////////////////////////////////////////////
        /////// Effect overview
        /////////////////////////////////////////////
        
        ui::Text("Effects");
        ui::Separator();
        int testId = 0;
        // Create colors for the texts.
        ColorA colorActive(0.0, 1.0, 0.0, 1.0);
        ColorA colorInActive(1.0, 0.0, 0.0, 1.0);
        ColorA colorFading(1.0, 1.0, 0.0, 1.0);
        for (auto it = mEffects.begin(); it != mEffects.end(); ) {
            ui::PushID(testId);
            EffectRef & effectRef = *it;
            ui::Checkbox("", &effectRef->isTurnedOn);
            ui::SameLine();
            if (ui::Button("Remove")) {
                it = mEffects.erase(it);
                effectSelection = nullptr;
                ui::PopID();
                continue;
            }
            ui::SameLine();
            if(ui::Button("Edit")) {
                effectSelection = &effectRef;
            }
            ui::SameLine();
            std::string effectName = effectRef->getName() + " (" + effectRef->getTypeName() + ")";
            ui::Text("%s", effectName.c_str());
            ui::SameLine();
            ColorA statusColor;
            switch (effectRef->getStatus()) {
                case photonic::Effect::kStatus_On:
                    statusColor = colorActive;
                    break;
                    
                case photonic::Effect::kStatus_Off:
                    statusColor = colorInActive;
                    break;
                    
                default:
                    statusColor = colorFading;
                    break;
            }
            ui::TextColored(statusColor, "%s", effectRef->getStatusName().c_str());
            ui::PopID();
            testId++;
            it++;
        }
    }
    if (effectSelection != nullptr) {
        ImGui::ScopedWindow window("Effect inspector");

        EffectRef effect = *effectSelection;
        static std::string name;
        name = effect->getName();
        if (ui::InputText("Name", &name)) {
            effect->setName(name);
        }
        ui::Separator();
        ui::InputFloat("FadeTime", &effect->fadeTime);
        ui::SliderFloat("Weight", &effect->weight, 0.0f, 100.0f);
        ui::Spacing();
        if (! ui::IsWindowCollapsed()) {
            ui::ListBoxHeader("Choose input channel", (int) mChannels.size());
            for (const auto channel : mChannels) {
                if (ui::Selectable(channel->getName().c_str(), channel == effect->getChannel())) {
                    effect->setChannel(channel);
                }
            }
            ui::ListBoxFooter();
        }
        std::string lightSelectText = mGuiStatusData.status == ADDING_LIGHT_TO_EFFECT ? "Done selecting lights" : "Select/Deselect lights";
        if (ui::Button(lightSelectText.c_str())) {
            // Toggle the state of the gui.
            mGuiStatusData.status = (mGuiStatusData.status == IDLE) ? ADDING_LIGHT_TO_EFFECT : IDLE;
            if (mGuiStatusData.status == ADDING_LIGHT_TO_EFFECT) {
                mGuiStatusData.pickLightEffect = *effectSelection;
                mVisualizer.enableEditingMode();
            }
            else {
                mGuiStatusData.pickLightEffect = nullptr;
                mVisualizer.disableEditingMode();
            }
        }
        
        // Create list of lights.
        if (! ui::IsWindowCollapsed()) {
            ui::ListBoxHeader("Lights");
            for (const auto light : effect->getLights()) {
                ui::BulletText("%s", light->mName.c_str());
            }
            ui::ListBoxFooter();
        }
        ui::Separator();
        // Draw the params.
        std::map<int, Parameter*> params = effect->getParams();
        int paramId = 100;
        for (auto &item : params) {
            ui::PushID(paramId);
            Parameter* param = item.second;
            switch (param->type) {
                case photonic::Parameter::kType_Int:
                    ui::InputInt(param->description.c_str(), &param->intValue);
                    break;
                    
                case photonic::Parameter::kType_Float:
                    ui::InputFloat(param->description.c_str(), &param->floatValue);
                    break;
                    
                case photonic::Parameter::kType_Color:
                    ui::ColorPicker4(param->description.c_str(), &param->colorValue[0]);
                    break;

                case photonic::Parameter::kType_Vector3:
                    ui::InputFloat3(param->description.c_str(), &param->vec3Value[0]);
                    break;

                case photonic::Parameter::kType_Channel_MinMax:
                    {
                        ui::Text("Settings for: %s, current value: %f", param->description.c_str(), param->getMappedChannelValue());
                        ImGui::Columns(6, NULL, false);
                        ui::InputFloat("Min in", &param->minIn);
                        ui::NextColumn();
                        ui::InputFloat("Max in", &param->maxIn);
                        ui::NextColumn();
                        ui::InputFloat("Min", &param->min);
                        ui::NextColumn();
                        ui::InputFloat("Max", &param->max);
                        ui::NextColumn();
                        ui::Columns(1);
                    }

                    // Intentional fallthrough.

                case photonic::Parameter::kType_Channel:
                    if (! ui::IsWindowCollapsed()) {
                        ui::ListBoxHeader(param->description.c_str(), (int) mChannels.size());
                        for (const auto channel : mChannels) {
                            if (ui::Selectable(channel->getName().c_str(), channel == param->channelRef)) {
                                param->channelRef = channel;
                            }
                        }
                        ui::ListBoxFooter();
                    }

                default:
                    break;
            }
            ui::PopID();
            paramId++;
        }
        
        effect->drawEditGui();
        if (ui::Button("Done")) {
            effectSelection = nullptr;
            mGuiStatusData.pickLightEffect = nullptr;
            mGuiStatusData.status = IDLE;
            mVisualizer.disableEditingMode();
        }
    }
    
}

void PhotonicDirectorApp::drawDmxInspector()
{
    ImGui::ScopedWindow window("Dmx inspector");
    if (! ui::IsWindowCollapsed()) {
        auto dmxVisuals = mDmxOut.getVisualizeTexture();
        ui::Image(dmxVisuals, dmxVisuals->getSize());
    }
}
void PhotonicDirectorApp::resize()
{
    mVisualizer.resize();
}

void PhotonicDirectorApp::pickLight()
{
    mGuiStatusData.pickedLight = mVisualizer.pickLight(mLights);
}

void PhotonicDirectorApp::draw()
{
    
    gl::clear( Color( 0, 0, 0 ) );
    mVisualizer.draw(mLights);
    
    // Delegate some gui drawing to the visualizer.
    if (mGuiStatusData.drawGui) {
        switch (mGuiStatusData.status) {
            case EDITING_LIGHT:
            case IDLE:
                if (mGuiStatusData.lightToEdit != nullptr) {
                    mVisualizer.highLightLight(mGuiStatusData.lightToEdit, Color(0.f, 1.f, 1.f));
                }
                break;

            case ADDING_LIGHT_TO_EFFECT:
                if (mGuiStatusData.pickLightEffect != nullptr) {
                    auto effectLights = mGuiStatusData.pickLightEffect->getLights();
                    for (auto light : effectLights) {
                        mVisualizer.highLightLight(light, Color(0.f, 1.f, 0.f));
                    }
                }

            default:
                break;
        }


        if (mGuiStatusData.pickedLight) {
            mVisualizer.highLightLight(mGuiStatusData.pickedLight);
        }
        // If we are calibrating, highlight the light as well.
        if (mLightCalibrator.isCalibrating()) {
            mVisualizer.highLightLight(mLightCalibrator.getCurrentLight(), Color(53, 252, 0));
        }
    }
    if (mGuiStatusData.visualizeEffects) {
        mVisualizer.drawEffects(mEffects);
    }
}

PhotonicDirectorApp::~PhotonicDirectorApp() {
    // The setting of the dmx channel is needed, because the director app
    // is destroyed first, and then the lights, that try to reach the dmxOuput
    // which is already destroyed.
    for (const auto light : mLights) {
        light->setDmxChannel(0);
    }
}

CINDER_APP( PhotonicDirectorApp, RendererGl( RendererGl::Options().msaa(8)), [](cinder::app::AppBase::Settings *settings){
    settings->setTitle("Photonic Director");
    settings->setWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
} );

