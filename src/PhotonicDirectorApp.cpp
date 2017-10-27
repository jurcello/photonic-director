#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "cinder/Log.h"
#include "CinderImGui.h"
#include "Light.h"
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

class PhotonicDirectorApp : public App {
public:
    
    PhotonicDirectorApp();
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
    void addLight();
    
    void setupOsc(int port);
    
    ~PhotonicDirectorApp();
    
protected:
    ConfigManager config;
    
    vector<Light*> mLights;
    
    // Light picking.
    Light* pickedLight;
    
    // Visualizer.
    Visualizer mVisualizer;
    
    // Create parameters
    bool mDrawGui;
    
    // Test light inspector.
    Light* lightToEdit;
    
    // Osc related stuff.
    osc::ReceiverUdp* mOscReceiver;
    int mOscPort;
    
    // Channel stuff.
    vector<InputChannelRef> mChannels;
    
    // Effects.
    vector<EffectRef> mEffects;
    EffectRef mPickLightEffect;
    
    void oscReceive(const osc::Message &message);
    // Gui stuff.
    void drawGui();
    void drawChannelControls();
    void drawLightControls();
    void drawEffectControls();

    
};

PhotonicDirectorApp::PhotonicDirectorApp()
: mOscReceiver(nullptr), mOscPort(10000), mPickLightEffect(nullptr)
{
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
    lightToEdit = nullptr;
    ImGui::Options options;
    setTheme(options);
    ImGui::initialize(options);
    pickedLight = nullptr;
    
    // Setup some initial lights.
    mLights.push_back(new Light(vec3(2.0f, 2.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.6f));
    mLights.push_back(new Light(vec3(1.0f, 1.0, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.99f));
    mLights.push_back(new Light(vec3(3.0f, 2.0f, -1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.9f));
    
    // Setup visualizer.
    mVisualizer.setup(mLights);
    
    // Initialize params.
    mDrawGui = true;
    setupOsc(mOscPort);
    setupOsc(mOscPort);
}

void PhotonicDirectorApp::setupOsc(int port)
{
    if (mOscReceiver) {
        mOscReceiver->close();
        delete mOscReceiver;
    }
    mOscReceiver = new osc::ReceiverUdp(port);
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

}

void PhotonicDirectorApp::oscReceive(const osc::Message &message)
{
    if (mChannels.size() > 0) {
        for (InputChannelRef channel : mChannels) {
            if (message.getAddress() == channel->getAddress()) {
                channel->setValue(message.getArgFloat(0));
            }
        }
    }
}

void PhotonicDirectorApp::addLight() {
    Light* newLight = new Light(vec3(1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f);
    mLights.push_back(newLight);
    lightToEdit = newLight;
}

void PhotonicDirectorApp::mouseDown( MouseEvent event )
{
    mVisualizer.mouseDown(event);
    if (pickedLight) {
        // Only do 1 action with the picked light.
        if (mPickLightEffect != nullptr) {
            mPickLightEffect->toggleLight(pickedLight);
        }
        else {
            lightToEdit = pickedLight;
        }

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
        mDrawGui = !mDrawGui;
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
        config.writeInt("oscPort", mOscPort);
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
            mOscPort = oscPort;
            setupOsc(mOscPort);
        }
        config.readLights(mLights);
        config.readChannels(mChannels);
        config.readEffects(mEffects, mLights, mChannels);
    }
}

void PhotonicDirectorApp::update()
{
    pickLight();
    
    if (mDrawGui) {
        drawGui();
    }
    
    
    /////////////////////////////////////////////
    // Effect handling.
    /////////////////////////////////////////////
    for (auto effect : mEffects) {
        effect->execute(0.f);
    }
}

void PhotonicDirectorApp::drawGui()
{
    static bool showLightEditor = true;
    static bool showChannelEditor = true;
    static bool showEffectEditor = true;
    // Draw the general ui.
    ui::ScopedWindow window("Controls");
    
    ui::Separator();
    ui::Text("Osc settings");
    ui::Spacing();
    if  (ui::InputInt("Port", &mOscPort)) {
        setupOsc(mOscPort);
    }
    ui::Separator();
    ui::Text("Widgets");
    ui::Checkbox("Show light editor", &showLightEditor);
    ui::Checkbox("Show channel editor", &showChannelEditor);
    ui::Checkbox("Show effect editor", &showEffectEditor);
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
}

void PhotonicDirectorApp::drawLightControls()
{
    ui::ScopedWindow window("Lights");
    if (ui::Button("Add")) {
        Light* newLight = new Light(vec3(1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f);
        mLights.push_back(newLight);
        lightToEdit = newLight;
    }
    if (lightToEdit) {
        ui::SameLine();
        if (ui::Button("Remove")) {
            auto it = std::find(mLights.begin(), mLights.end(), lightToEdit);
            if (it != mLights.end()) {
                mLights.erase(it);
                delete lightToEdit;
                lightToEdit = nullptr;
            }
        }
    }
    if (! ui::IsWindowCollapsed()) {
        ui::ListBoxHeader("Edit lights");
        for (Light* light: mLights) {
            if (ui::Selectable(light->mName.c_str(), lightToEdit == light)) {
                lightToEdit = light;
            }
        }
        ui::ListBoxFooter();
    }
    if (lightToEdit) {
        ui::ScopedWindow lightEditWindow("Edit light");
        ui::InputText("Name", &lightToEdit->mName);
        ui::SliderFloat("Intensity", &lightToEdit->intensity, 0.f, 1.f);
        ui::ColorEdit4("Color", &lightToEdit->color[0]);
        ui::DragFloat3("Position", &lightToEdit->position[0]);
        if (ui::Button("Done")) {
            lightToEdit = nullptr;
        }
    }
}

void PhotonicDirectorApp::drawChannelControls()
{
    static const InputChannelRef* channelSelection = nullptr;
    {
        
        ui::ScopedWindow window("Channels");
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
        ui::ScopedWindow window("Channel inspector");
        
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
        ui::ScopedWindow window("Effects");
        // Add the button.
        if (ui::Button("Create Effect")) {
            ui::OpenPopup("Create effect");
        }
        if (ui::BeginPopupModal("Create effect")) {
            static std::string effectName;
            ui::InputText("Name", &effectName);
            if (ui::Button("Done")) {
                EffectRef newEffect = Effect::create(effectName);
                // TODO: Now add all lights. Later lights should be picked.
                for (auto light : mLights) {
                    newEffect->addLight(light);
                }
                mEffects.push_back(newEffect);
                ui::CloseCurrentPopup();
            }
            ui::EndPopup();
        }
        if (effectSelection) {
            ui::SameLine();
            if (ui::Button("Remove")) {
                auto it = std::find(mEffects.begin(), mEffects.end(), *effectSelection);
                if (it != mEffects.end()) {
                    mEffects.erase(it);
                    effectSelection = nullptr;
                }
            }
        }
        if (! ui::IsWindowCollapsed()) {
            ui::ListBoxHeader("Edit effects");
            for (EffectRef& effect : mEffects) {
                if (ui::Selectable(effect->getName().c_str(), effectSelection == &effect)) {
                    effectSelection = &effect;
                }
            }
            ui::ListBoxFooter();
        }
    }
    if (effectSelection != nullptr) {
        ui::ScopedWindow window("Effect inspector");
        
        EffectRef effect = *effectSelection;
        static std::string name;
        name = effect->getName();
        if (ui::InputText("Name", &name)) {
            effect->setName(name);
        }
        ui::Separator();
        ui::Spacing();
        if (! ui::IsWindowCollapsed()) {
            ui::ListBoxHeader("Choose input channel",mChannels.size());
            for (auto channel : mChannels) {
                if (ui::Selectable(channel->getName().c_str(), channel == effect->getChannel())) {
                    effect->setChannel(channel);
                }
            }
            ui::ListBoxFooter();
        }
        std::string lightSelectText = mPickLightEffect == nullptr ? "Select/Deselect lights" : "Done selecting lights";
        if (ui::Button(lightSelectText.c_str())) {
            mPickLightEffect = mPickLightEffect == nullptr ? *effectSelection : nullptr;
            mVisualizer.enableEditingMode();
        }
        
        if (ui::Button("Done")) {
            effectSelection = nullptr;
            mPickLightEffect = nullptr;
            mVisualizer.disableEditingMode();
        }
    }
    
}

void PhotonicDirectorApp::resize()
{
    mVisualizer.resize();
}

void PhotonicDirectorApp::pickLight()
{
    pickedLight = mVisualizer.pickLight(mLights);
}

void PhotonicDirectorApp::draw()
{
    
    gl::clear( Color( 0, 0, 0 ) );
    
    mVisualizer.draw(mLights);
    
    if (pickedLight) {
        mVisualizer.highLightLight(pickedLight);
    }
    if (lightToEdit) {
        mVisualizer.highLightLight(lightToEdit, Color(0.f, 1.f, 1.f));
    }
    if (mPickLightEffect != nullptr) {
        auto effectLights = mPickLightEffect->getLights();
        for (auto light : effectLights) {
            mVisualizer.highLightLight(light, Color(0.f, 1.f, 0.f));
        }
    }
}

PhotonicDirectorApp::~PhotonicDirectorApp()
{
    for (Light* light : mLights) {
        delete light;
    }
    // Delete the osc receiver.
}

CINDER_APP( PhotonicDirectorApp, RendererGl( RendererGl::Options().msaa(8)), [](cinder::app::AppBase::Settings *settings){
    settings->setTitle("Photonic Director");
    settings->setWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
} );

