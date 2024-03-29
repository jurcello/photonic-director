#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/Log.h"
#include "CinderImGui.h"
#include "cinder/Json.h"
#include "Light.h"
#include "LightCalibrator.h"
#include "Effects.h"
#include "ConfigManager.h"
#include "UnityConnector.h"
#include "Visualizer.h"
#include "Osc.h"
#include <boost/algorithm/string.hpp>
#include "Poco/DNSSD/DNSSDResponder.h"
#include "Poco/DNSSD/Bonjour/Bonjour.h"
#include "MidiMessage.h"
#include "Scene.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ph;
using protocol = asio::ip::udp;

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

const char *LIGHT_SELECT = "LIGHT_SELECT";
const char *CHANNEL_SELECT = "CHANN_SELECT";
const char *EFFECT_SELECT_DRAG_DROP = "EFF_SELECT";

enum gui_status
{
    IDLE,
    EDITING_LIGHT,
    ADDING_LIGHT_TO_EFFECT,
    CALIBRATING_LIGHT,
};

struct GuiStatusData
{
    EffectRef pickLightEffect;
    LightRef lightToEdit;
    LightRef pickedLight;
    LightRef hoveredLight;
    bool drawGui;
    bool visualizeEffects;

    bool isDraggingLight;
    bool isDraggingChannel;

    gui_status status;
};

class PhotonicDirectorApp : public App
{
  public:
    PhotonicDirectorApp();

    virtual ~PhotonicDirectorApp();

    void setTheme(ImGui::Options &options);

    void setup() override;
    void mouseDown(MouseEvent event) override;
    void mouseUp(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void mouseMove(MouseEvent event) override;
    void update() override;
    void draw() override;
    void keyDown(KeyEvent event) override;
    void resize() override;
    void fileDrop( FileDropEvent event );

    void save();
    void quickSave();
    void load();
    void loadFile(fs::path filePath);

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
    osc::ReceiverUdp *mOscReceiver;
    osc::SenderUdp *mOscSender;
    osc::UdpSocketRef mOscSocket;
    bool mOscUnicast;
    std::string mOscSendAddress;
    int mOscReceivePort;
    int mOscSendPort;
    std::string mLastOscAddress;
    bool mSendLightOsc;

    // Unity related stuff.
    std::string mUnityAddress;
    int mUnityPort;
    UnityConnector mUnityConnector;

    // Channel stuff.
    vector<InputChannelRef> mChannels;
    vector<std::string> mIncomingOscAdresses;
    double mLastChannelGeneration;

    // Effects.
    list<EffectRef> mEffects;
    double mLastUpdate;

    SceneListRef mSceneList;
    SceneListUIRef mScenesUI;

    void oscReceive(const osc::Message &message);
    void updateChannelsFromOsc();
    void handleLightOscSending();
    void handleCalibrationSending();
    smf::MidiMessage mMidiMessage;

    // Gui stuff.
    void drawGui();
    void drawChannelControls();
    void drawLightControls();
    void drawEffectControls();
    void drawDmxInspector();
    void drawObjectsHierarchy();
    GuiStatusData mGuiStatusData;
    bool mShowVisualizer;
    bool mShowObjects;

    void changeEffectOrder(EffectRef effect, int newPosition);

    // Dmx output.
    DmxOutput mDmxOut;

    // Zeroconf
    Poco::DNSSD::DNSSDResponder *mDnssdResponder;
    Poco::DNSSD::ServiceHandle mServiceHandle;

    fs::path mCurrentPath;
    void saveToFile(fs::path savePath);
    float mLastSave = 0.f;
};

PhotonicDirectorApp::PhotonicDirectorApp()
    : mOscReceiver(nullptr),
      mOscSender(nullptr),
      mOscSocket(nullptr),
      mOscReceivePort(10000),
      mOscSendPort(10001),
      mOscUnicast(false),
      mOscSendAddress("192.168.1.11"),
      mLightFactory(&mDmxOut),
      mShowVisualizer(true),
      mShowObjects(false),
      mLastOscAddress(""),
      mLastChannelGeneration(0.0),
      mUnityAddress("192.168.1.8"),
      mUnityPort(8089)
{
    mGuiStatusData.pickLightEffect = nullptr;
    mGuiStatusData.lightToEdit = nullptr;
    mGuiStatusData.pickedLight = nullptr;
    mGuiStatusData.drawGui = true;
    mGuiStatusData.visualizeEffects = false;
    mGuiStatusData.isDraggingLight = false;
    mGuiStatusData.isDraggingChannel = false;

    mGuiStatusData.status = IDLE;
    Poco::DNSSD::initializeDNSSD();

    // Setup scenelist.
    mSceneList = SceneListRef(new SceneList);
    mScenesUI = SceneListUIRef(new SceneListUI(mSceneList));
}

void PhotonicDirectorApp::setTheme(ImGui::Options &options)
{
    options = options.darkTheme();
    options.color(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.93f, 0.0f, 0.63f));
}

void PhotonicDirectorApp::setup()
{
    mDnssdResponder = new Poco::DNSSD::DNSSDResponder();
    mDnssdResponder->start();

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
    // register with DNSSDResponder
    mDnssdResponder->unregisterService(mServiceHandle);
    std::string name = "Photonic Director - " + std::to_string(receivePort);
    Poco::DNSSD::Service service(0, name, "", "_osc._udp", "", "", receivePort);
    mServiceHandle = mDnssdResponder->registerService(service);

    if (mOscReceiver)
    {
        mOscReceiver->close();
        delete mOscReceiver;
    }
    if (mOscSender)
    {
        try
        {
            mOscSender->close();
            delete mOscSender;
        }
        catch (Exception exception)
        {
            console() << "An exception occured closing the connection";
        }
    }
    if (mOscSocket)
    {
        mOscSocket->close();
        mOscSocket = nullptr;
    }
    mOscReceiver = new osc::ReceiverUdp(receivePort);
    // Setup osc to listen to all addresses.
    mOscReceiver->setListener("/*", [&](const osc::Message &message) {
        oscReceive(message);
    });
    try
    {
        mOscReceiver->bind();
    }
    catch (const osc::Exception &ex)
    {
        CI_LOG_E("Error binding: " << ex.what() << ", val:" << ex.value());
    }
    mOscReceiver->listen([&](asio::error_code error, protocol::endpoint endpoint) -> bool {
        if (error)
        {
            if (error.value() != 89)
                CI_LOG_E("Error listening: " << error.message() << ", val: " << error.value() << ", endpoint: " << endpoint);
            return false;
        }
        else
        {
            return true;
        }
    });

    //////////////////////////
    // Setup sender.
    //////////////////////////
    // Us a local port of 31,000 because that one most probably is not used.
    const int localPort = 31000;
    mOscSocket = osc::UdpSocketRef(new protocol::socket(App::get()->io_service(), protocol::endpoint(protocol::v4(), localPort)));
    asio::ip::address_v4 address = asio::ip::address_v4::broadcast();
    if (mOscUnicast)
    {
        address = asio::ip::address_v4::from_string(mOscSendAddress);
    }
    else
    {
        mOscSocket->set_option(asio::socket_base::broadcast(true));
    }
    mOscSender = new osc::SenderUdp(mOscSocket, protocol::endpoint(address, sendPort));
    mLightCalibrator.setOscSender(mOscSender);
    for (auto effect : mEffects)
    {
        effect->setOscSender(mOscSender);
    }
}

void PhotonicDirectorApp::oscReceive(const osc::Message &message)
{
    try
    {
        // Get the midi message if there is one.
        if (message.getArgType(0) == osc::ArgType::MIDI)
        {
            // parse the mide signal and return
            uint8_t port;
            uint8_t status;
            uint8_t data1;
            uint8_t data2;
            message.getArgMidi(0, &port, &status, &data1, &data2);
            // Create the midi message.
            smf::MidiMessage midiMessage = smf::MidiMessage(status, data1, data2);
            for (auto effect : mEffects)
            {
                effect->listenToMidi(&midiMessage);
            }
            return;
        }

        mLastOscAddress = message.getAddress();
        if (std::find(mIncomingOscAdresses.begin(), mIncomingOscAdresses.end(), mLastOscAddress) == mIncomingOscAdresses.end())
        {
            // Only add messages containing a float at the start to the addresses.
            if (message.getArgType(0) == osc::ArgType::FLOAT || message.getArgType(0) == osc::ArgType::INTEGER_32)
            {
                mIncomingOscAdresses.push_back(mLastOscAddress);
            }
        }
        if (mChannels.size() > 0)
        {
            for (InputChannelRef channel : mChannels)
            {
                if (message.getAddress() == channel->getAddress())
                {
                    // Gather the args. There can be up to 3 args.
                    int numArgs = message.getNumArgs();
                    float arg1, arg2, arg3;
                    if (message.getNumArgs() >= 1)
                    {

                        if (message.getArgType(0) == osc::ArgType::INTEGER_32)
                        {
                            int arg = message.getArgInt32(0);
                            arg1 = arg;
                            channel->setValue(arg);
                            channel->setValue(arg1);
                            channel->setType(InputChannel::Type::kType_Dim1);
                        }
                        else if (message.getArgType(0) == osc::ArgType::FLOAT)
                        {
                            arg1 = message.getArgFloat(0);
                            channel->setValue(arg1);
                            channel->setType(InputChannel::Type::kType_Dim1);
                        }
                        if (numArgs >= 2)
                        {
                            arg2 = message.getArgFloat(1);
                            channel->setValue(vec2(arg1, arg2));
                            channel->setType(InputChannel::Type::kType_Dim2);
                            if (numArgs >= 3)
                            {
                                arg3 = message.getArgFloat(2);
                                channel->setValue(vec3(arg1, arg2, arg3));
                                channel->setType(InputChannel::Type::kType_Dim3);
                            }
                        }
                    }
                }
            }
        }
        mSceneList->listenToOsc(message);
        mLightCalibrator.receiveOscMessage(message);
        for (auto effect : mEffects)
        {
            effect->listenToOsc(message);
        }
        updateChannelsFromOsc();
    }
    catch (std::exception &exc)
    {
        app::console() << "Channel receives string or other unknown type: " << exc.what() << std::endl;
    }
}

void PhotonicDirectorApp::updateChannelsFromOsc()
{
    if (mLastUpdate - mLastChannelGeneration > 2.0)
    {
        for (auto address : mIncomingOscAdresses)
        {
            bool found = false;
            for (auto channel : mChannels)
            {
                if (channel->getAddress() == address)
                {
                    found = true;
                }
            }
            if (!found)
            {
                mChannels.push_back(InputChannel::create(address, address));
            }
        }
        mLastChannelGeneration = mLastUpdate;
    }
}

void PhotonicDirectorApp::mouseDown(MouseEvent event)
{
    mVisualizer.mouseDown(event);
    switch (mGuiStatusData.status)
    {
    case ADDING_LIGHT_TO_EFFECT:
        if (mGuiStatusData.pickedLight != nullptr && mGuiStatusData.pickLightEffect != nullptr)
        {
            mGuiStatusData.pickLightEffect->toggleLight(mGuiStatusData.pickedLight);
        }
        break;

    default:
        if (mGuiStatusData.pickedLight != nullptr)
        {
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

void PhotonicDirectorApp::keyDown(KeyEvent event)
{
    if (event.getChar() == 'q' && event.isAccelDown())
        exit(0);
    if (event.getChar() == 'g')
    {
        mGuiStatusData.drawGui = !mGuiStatusData.drawGui;
    }
    if (event.getChar() == 'v')
    {
        mShowVisualizer = !mShowVisualizer;
    }
    if (event.getChar() == 's' && event.isAccelDown())
    {
        quickSave();
    }
}

void PhotonicDirectorApp::save()
{
    std::vector<string> extensions{"xml"};
    fs::path savePath = getSaveFilePath(fs::path(), extensions);
    if (!savePath.empty())
    {
        saveToFile(savePath);
    }
}

void PhotonicDirectorApp::quickSave()
{
    if (mCurrentPath.empty())
    {
        save();
        return;
    }
    mLastSave = getElapsedSeconds();
    saveToFile(mCurrentPath);
}

void PhotonicDirectorApp::saveToFile(fs::path savePath)
{
    mCurrentPath = savePath;
    config.startNewDoc();
    config.writeLights(mLights);
    config.writeChannels(mChannels);
    config.writeEffects(mEffects);
    config.writeScenes(mSceneList);
    config.writeValue<int>("oscReceivePort", mOscReceivePort);
    config.writeValue<int>("oscSendPort", mOscSendPort);
    config.writeValue<int>("unityPort", mUnityPort);
    config.writeValue<bool>("oscUnicast", mOscUnicast);
    config.writeValue<bool>("sendLightOsc", mSendLightOsc);
    config.writeValue<std::string>("oscSendAddress", mOscSendAddress);
    config.writeValue<std::string>("unityAddress", mUnityAddress);
    config.writeToFile(savePath);
}

void PhotonicDirectorApp::load()
{
    mGuiStatusData.lightToEdit = nullptr;
    std::vector<string> extensions{"xml"};
    fs::path loadPath = getOpenFilePath(fs::path(), extensions);
    loadFile(loadPath);
}

void PhotonicDirectorApp::loadFile(fs::path filePath) {
    if (!filePath.empty())
    {
        mCurrentPath = filePath;
        config.readFromFile(filePath);
        mOscReceivePort = config.readValue<int>("oscReceivePort", mOscReceivePort);
        mOscSendPort = config.readValue<int>("oscSendPort", mOscSendPort);
        mUnityPort = config.readValue<int>("unityPort", mUnityPort);
        mOscUnicast = config.readValue<bool>("oscUnicast", mOscUnicast);
        mOscSendAddress = config.readValue<std::string>("oscSendAddress", mOscSendAddress);
        mUnityAddress = config.readValue<std::string>("unityAddress", mUnityAddress);
        mSendLightOsc = config.readValue<bool>("sendLightOsc", mSendLightOsc);
        setupOsc(mOscReceivePort, mOscSendPort);
        // Reset the channelRegistry of the dmx out.
        mDmxOut.clearRegistry();
        config.readLights(mLights, &mLightFactory);
        config.readChannels(mChannels);
        config.readEffects(mEffects, mLights, mChannels);
        // Todo: there is some code duplication here.
        // However, there needs to be some refactoring concerning osc,
        // because we might want more than one osc connection.
        for (auto effect : mEffects)
        {
            effect->setOscSender(mOscSender);
        }
        config.readScenes(mSceneList, mEffects);
    }
}

void PhotonicDirectorApp::update()
{
    pickLight();

    if (mGuiStatusData.drawGui)
    {
        drawGui();
    }

    for (const auto light : mLights)
    {
        auto components = light->getComponents();
        if (components.size() > 0)
        {
            for (const auto component : components)
            {
                component->controlledBy = "";
            }
        }
    }

    double now = getElapsedSeconds();
    double dt = now - mLastUpdate;
    /////////////////////////////////////////////
    // Effect handling.
    /////////////////////////////////////////////
    // Main stage.
    for (auto effect : mEffects)
    {
        effect->execute(dt);
    }
    // Prepare DMX output.
    mDmxOut.reset();

    /////////////////////////////////////////////
    // Light handling: Main stage.
    /////////////////////////////////////////////
    for (const auto light : mLights)
    {
        if (mLightCalibrator.isCalibrating())
        {
            const float calibratedLightIntensity = static_cast<const float>(((sin(getElapsedSeconds()) + 1.0) / 4) + 0.5f);
            light->intensity = (light == mLightCalibrator.getCurrentLight()) ? calibratedLightIntensity * mLightCalibrator.getCurrentIntensity() : 0.0f;
            light->color = (light == mLightCalibrator.getCurrentLight()) ? light->getLightType()->editColor * mLightCalibrator.getCurrentIntensity() : ColorA::black();
        }
        else
        {
            float endIntensity = 0.f;
            ColorA endColor(0.0f, 0.0f, 0.0f, 1.0f);
            float highestWeight = 1.0f;
            for (const auto effect : mEffects)
            {
                if (effect->hasOutput() && effect->hasLight(light) && effect->getStage() == Effect::Stage::kStage_Main)
                {
                    const float effetcIntensity = light->getEffetcIntensity(effect->getUuid());
                    ////////////////////////////////////////////////////////
                    // Calculate the weight of the effect. The weight should be more if the intensity is more.
                    ////////////////////////////////////////////////////////
                    float effectWeight = (1.0f + effetcIntensity * effect->weight) * effect->getFadeValue();
                    if (effectWeight > highestWeight)
                    {
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
            if (light->isColorEnabled())
            {
                light->color = endColor;
            }
        }
    }
    // After effects.
    for (auto effect : mEffects)
    {
        if (effect->getStage() == Effect::Stage::kStage_After && effect->isTurnedOn)
        {
            effect->executePost(dt);
        }
    }

    // Todo: cleanup later.
    if (mVisualizer.isInEditMode() && mGuiStatusData.hoveredLight != nullptr)
    {
        mGuiStatusData.hoveredLight->intensity = 0.5f;
        if (mGuiStatusData.hoveredLight->isColorEnabled())
        {
            mGuiStatusData.hoveredLight->color = ColorA::gray(0.5f);
        }
    }

    ///////////////////////////////////////////////////
    // Light handling: updating DMX data.
    ///////////////////////////////////////////////////
    for (const auto light : mLights)
    {
        light->update();
        light->updateDmx();
    }
    mDmxOut.update();
    if (mSendLightOsc) {
        handleLightOscSending();
    }
    handleCalibrationSending();
    mLastUpdate = now;
}

void PhotonicDirectorApp::handleLightOscSending()
{
    // Create an osc bundle ot send the messages for better speed.
    osc::Bundle bundle;
    int maxLightsToSend = 10;
    int counter = 0;
    for (auto light : mLights)
    {
        counter++;
        if (light->mSendOsc && light->mOscAdress[0] == '/')
        {
            osc::Message message(light->mOscAdress);
            message.append(light->intensity);
            if (light->isColorEnabled())
            {
                message.append(true);
                message.append(light->color.r);
                message.append(light->color.g);
                message.append(light->color.b);
            }
            else
            {
                message.append(false);
            }
            bundle.append(message);
            if (counter > maxLightsToSend)
            {
                mOscSender->send(bundle);
                bundle.clear();
                counter = 0;
            }
        }
    }
    mOscSender->send(bundle);
}

void PhotonicDirectorApp::handleCalibrationSending()
{
    if (mLightCalibrator.isCalibrating())
    {
        osc::Message message("/calibrating");
        message.append(mLightCalibrator.getCurrentLight()->getUuid());
        mOscSender->send(message);
    }
}

void PhotonicDirectorApp::drawGui()
{
    static bool showLightEditor = true;
    static bool showChannelEditor = true;
    static bool showEffectEditor = true;
    static bool showDmxInspector = false;
    static bool showSceneInspector = true;
    // Draw the general ui.
    ImGuiWindowFlags windowFlags = 0;
    windowFlags |= ImGuiWindowFlags_NoMove;
    ImGui::ScopedWindow window("Controls", windowFlags);

    //////////// Save message
    if (mLastSave > getElapsedSeconds() - 0.5f && getElapsedSeconds() > 1.f)
    {
        ui::OpenPopup("Saved");
        if (ui::BeginPopupModal("Saved"))
        {
            ui::Text("Your save was succesfull");
            ui::EndPopup();
        }
    }
    else
    {
        if (ui::IsPopupOpen("Saved"))
        {
            ui::CloseCurrentPopup();
        }
    }

    ui::Separator();
    ui::Text("Osc settings");
    ui::Spacing();
    if (ui::InputInt("Osc Receive Port", &mOscReceivePort))
    {
        setupOsc(mOscReceivePort, mOscSendPort);
    }
    if (ui::InputInt("Osc Send Port", &mOscSendPort))
    {
        setupOsc(mOscReceivePort, mOscSendPort);
    }
    if (ui::Checkbox("Use udp unicast", &mOscUnicast))
    {
        setupOsc(mOscReceivePort, mOscSendPort);
    }
    if (mOscUnicast)
    {
        if (ui::InputText("Osc send address", &mOscSendAddress))
        {
            try
            {
                asio::ip::address_v4::from_string(mOscSendAddress);
                setupOsc(mOscReceivePort, mOscSendPort);
            }
            catch (std::exception e)
            {
                // Catch nothing.
            }
        }
    }
    ui::Checkbox("Send light osc", &mSendLightOsc);

    ui::Separator();
    ui::Text("Dmx settings");
    if (!ui::IsWindowCollapsed())
    {
        if (!mDmxOut.isConnected())
        {
            auto devices = mDmxOut.getDevicesList();
            ui::ListBoxHeader("Choose device", devices.size());
            for (auto device : devices)
            {
                if (ui::Selectable(device.c_str()))
                {
                    mDmxOut.connect(device);
                }
            }
            ui::ListBoxFooter();
        }
        else
        {
            ui::Text("Connected to: ");
            const std::string deviceInfo = mDmxOut.getConnectedDevice();
            ui::Text("%s", deviceInfo.c_str());
            ui::SameLine();
            if (ui::Button("Disconnect"))
            {
                mDmxOut.disConnect();
            }
        }
    }
    ui::Separator();
    ui::Text("Calibrator");
    if (ui::Button("Start calibration"))
    {
        mLightCalibrator.start();
    }
    if (mLightCalibrator.isCalibrating())
    {
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
    ui::Checkbox("Show Scene inspector", &showSceneInspector);
    ui::Checkbox("Show objects hierarchy", &mShowObjects);
    ui::Checkbox("Show DMX inspector", &showDmxInspector);
    ui::Separator();
    ui::Text("Unity connection");
    ui::InputText("Address", &mUnityAddress);
    ui::SameLine();
    ui::InputInt("Port", &mUnityPort);
    if (ui::Button("Sync lights with unity"))
    {
        try
        {
            mUnityConnector.initialize(mUnityAddress, mUnityPort, &mLights, &mLightFactory);
            // TODO: Use a better way. Some sort of event listener system for light removal,
            auto &mEffectsAlias = mEffects;
            std::function<void(const LightRef)> cleanFunction = [mEffectsAlias](const LightRef light) {
                for (auto effect : mEffectsAlias)
                {
                    effect->removeLight(light);
                }
            };
            mUnityConnector.sync(cleanFunction);
        }
        catch (ci::Exception &exception)
        {
            CI_LOG_W("exception caught, what: " << exception.what());
            ui::OpenPopup("Error syncing to unity");
        }
    }
    if (ui::BeginPopupModal("Error syncing to unity"))
    {
        ui::Text("There was an error syncing the lights. Please check address and port.");
        if (ui::Button("Close"))
        {
            ui::CloseCurrentPopup();
        }
        ui::EndPopup();
    }
    ui::Separator();
    ui::Text("File");
    if (!mCurrentPath.empty())
    {
        ui::Spacing();
        ui::Text("Current file: %s", mCurrentPath.filename().c_str());
    }
    ui::Spacing();
    if (ui::Button("Load"))
    {
        load();
    }
    ui::SameLine();
    if (ui::Button("Save"))
    {
        save();
    }
    // Draw the channel window if there are channels.
    if (showChannelEditor)
    {
        drawChannelControls();
    }
    if (showEffectEditor)
    {
        drawEffectControls();
    }
    if (showLightEditor)
    {
        drawLightControls();
    }
    if (mShowObjects)
    {
        drawObjectsHierarchy();
    }
    if (showDmxInspector)
    {
        drawDmxInspector();
    }
    if (showSceneInspector)
    {
        mScenesUI->drawGui();
    }
}

void PhotonicDirectorApp::drawLightControls()
{
    ImGui::ScopedWindow window("Lights");
    static bool editingMode = false;
    ui::Checkbox("Edit mode", &editingMode);
    if (editingMode)
    {
        mVisualizer.enableEditingMode();
    }
    else
    {
        mVisualizer.disableEditingMode();
    }
    if (ui::Button("Add Light"))
    {
        ui::OpenPopup("Create Light");
    }
    if (ui::BeginPopupModal("Create Light"))
    {
        static std::string lightName;
        static int lightType = 0;
        static vec3 lightPosition;
        ui::InputText("Name", &lightName);
        auto availableTypeNames = mLightFactory.getAvailableTypeNames();
        if (ui::BeginCombo("Type", availableTypeNames[lightType].c_str()))
        {
            for (int i = 0; i < availableTypeNames.size(); i++)
            {
                bool isSelected = (lightType == i);
                if (ui::Selectable(availableTypeNames[i].c_str(), isSelected))
                {
                    lightType = i;
                }
                if (isSelected)
                {
                    ui::SetItemDefaultFocus();
                }
            }
            ui::EndCombo();
        }
        ui::InputFloat3("Position", &lightPosition[0]);

        if (ui::Button("Done"))
        {
            std::vector<LightType *> lightTypes = mLightFactory.getAvailableTypes();
            LightRef newLight = mLightFactory.create(lightPosition, lightTypes[lightType], std::string());
            newLight->mName = lightName;
            mLights.push_back(newLight);
            ui::CloseCurrentPopup();
        }
        ui::SameLine();
        if (ui::Button("Cancel"))
        {
            ui::CloseCurrentPopup();
        }
        ui::EndPopup();
    }

    if (mGuiStatusData.lightToEdit)
    {
        ui::SameLine();
        if (ui::Button("Remove"))
        {
            auto it = std::find(mLights.begin(), mLights.end(), mGuiStatusData.lightToEdit);
            if (it != mLights.end())
            {
                // First remove the light from the effects.
                for (auto effect : mEffects)
                {
                    effect->removeLight(*it);
                }
                mLights.erase(it);
                mGuiStatusData.lightToEdit = nullptr;
            }
        }
    }
    if (!ui::IsWindowCollapsed())
    {
        ui::ListBoxHeader("Edit lights", mLights.size(), 20);
        ImGuiDragDropFlags srcFlags = 0;
        std::vector<LightRef> lightsSorted = mLights;
        std::sort(lightsSorted.begin(), lightsSorted.end(), [](LightRef light1, LightRef light2) -> bool {
            return light1->mName < light2->mName;
        });
        mGuiStatusData.hoveredLight = nullptr;
        for (LightRef light : lightsSorted)
        {
            if (ui::Selectable(light->mName.c_str(), mGuiStatusData.lightToEdit == light))
            {
                mGuiStatusData.lightToEdit = light;
            }
            if (ui::BeginDragDropSource(srcFlags))
            {
                if (!(srcFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
                {
                    ui::Text("Move %s to effect", light->mName.c_str());
                }
                ui::SetDragDropPayload(LIGHT_SELECT, &light, sizeof(light));
                ui::EndDragDropSource();
                mGuiStatusData.isDraggingLight = true;
            }
            if (ui::IsItemHovered())
            {
                mGuiStatusData.hoveredLight = light;
            }
        }
        ui::ListBoxFooter();
    }
    if (mGuiStatusData.lightToEdit)
    {
        ImGui::ScopedWindow lightEditWindow("Edit light");
        ui::InputText("Name", &mGuiStatusData.lightToEdit->mName);
        ui::SliderFloat("Intensity", &mGuiStatusData.lightToEdit->intensity, 0.f, 1.f);
        ui::ColorEdit4("Color", &mGuiStatusData.lightToEdit->color[0]);
        ui::DragFloat3("Position", &mGuiStatusData.lightToEdit->position[0]);
        ui::Spacing();
        ui::Text("Light type");
        ui::Separator();
        LightType *lightType = mGuiStatusData.lightToEdit->getLightType();
        std::string lightTypeName = "Type: " + lightType->name;
        ui::Text("%s", lightTypeName.c_str());
        int numChannels = mGuiStatusData.lightToEdit->getNumChannels();
        if (numChannels > 1)
        {
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
        if (ui::InputInt("DMX channel", &dmxChannel, 0, 256))
        {
            if (!(mGuiStatusData.lightToEdit->setDmxChannel(dmxChannel)))
            {
                ui::TextColored(Color(1.f, 0.f, 0.f), "The channel is already taken");
            }
        }
        ui::InputInt("Dmx offset", &mGuiStatusData.lightToEdit->mDmxOffsetIntentsityValue, 0, 255);
        ui::Checkbox("Send osc", &mGuiStatusData.lightToEdit->mSendOsc);
        if (mGuiStatusData.lightToEdit->mSendOsc)
        {
            ui::InputText("Osc address", &mGuiStatusData.lightToEdit->mOscAdress);
        }
        ui::Spacing();
        // Get the components and draw their ui's.
        auto components = mGuiStatusData.lightToEdit->getComponents();
        if (components.size() > 0)
        {
            if (ui::CollapsingHeader("Components"))
            {
                int componentId = 0;
                for (auto component : components)
                {
                    component->getGui()->draw(componentId);
                    componentId++;
                }
            }
        }
        if (ui::Button("Done"))
        {
            mGuiStatusData.lightToEdit = nullptr;
            mGuiStatusData.status = IDLE;
        }
    }
}

void PhotonicDirectorApp::drawChannelControls()
{
    static const InputChannelRef *channelSelection = nullptr;
    {

        ImGui::ScopedWindow window("Channels");
        // Add the buttons.
        if (ui::Button("Add Channel"))
        {
            ui::OpenPopup("Create channel");
        }
        if (ui::BeginPopupModal("Create channel"))
        {
            static char channelName[64];
            ui::InputText("Name", channelName, 64);
            static char channelAddress[64];
            ui::InputText("Address", channelAddress, 64);
            if (ui::Button("Done"))
            {
                mChannels.push_back(InputChannel::create(channelName, channelAddress));
                ui::CloseCurrentPopup();
            }
            ui::EndPopup();
        }
        if (channelSelection)
        {
            ui::SameLine();
            if (ui::Button("Remove"))
            {
                auto it = std::find_if(mChannels.begin(), mChannels.end(), [](const InputChannelRef &channel) { return &channel == channelSelection; });
                if (it != mChannels.end())
                {
                    mChannels.erase(it);
                    channelSelection = nullptr;
                }
            }
        }
        if (!ui::IsWindowCollapsed())
        {
            ui::ListBoxHeader("Edit channels", mChannels.size(), 20);
            for (const InputChannelRef &channel : mChannels)
            {
                if (ui::Selectable(channel->getName().c_str(), channelSelection == &channel))
                {
                    channelSelection = &channel;
                }
                ImGuiDragDropFlags srcFlags = 0;
                if (ui::BeginDragDropSource(srcFlags))
                {
                    if (!(srcFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
                    {
                        ui::Text("Move %s to effect", channel->getName().c_str());
                    }
                    ui::SetDragDropPayload(CHANNEL_SELECT, &channel, sizeof(channel));
                    ui::EndDragDropSource();
                    mGuiStatusData.isDraggingChannel = true;
                }
                ui::SameLine();
                float value = channel->getValue();
                ui::SliderFloat("", &value, 0.0f, 1.0f);
            }
            ui::ListBoxFooter();
        }
    }

    if (channelSelection != nullptr)
    {
        ImGui::ScopedWindow window("Channel inspector");

        InputChannelRef channel = *channelSelection;
        static std::string name;
        name = channel->getName();
        if (ui::InputText("Name", &name))
        {
            channel->setName(name);
        }
        static std::string address;
        address = channel->getAddress();
        if (ui::InputText("Address", &address))
        {
            channel->setAdrress(address);
        }
        static int smoothing;
        smoothing = channel->getSmoothing();
        if (ui::InputInt("Smoothing (high is more)", &smoothing))
        {
            channel->setSmoothing(smoothing);
        }
        if (channel->getType() == photonic::InputChannel::Type::kType_Dim1) {
            static bool isCalibrating;
            isCalibrating = channel->isCalibrating();
            if (ui::Checkbox("Calibrate", &isCalibrating)) {
                channel->setCalibrationMode(isCalibrating);
            }
            static float calibrationMax;
            calibrationMax = channel->getCalibrationMax();
            ui::InputFloat("Calibration max", &calibrationMax);
            if (ui::Button("Reset calibration")) {
                channel->setCalibrationMax(1.0f);
                channel->setCalibrationMode(false);
            }
        }
        if (ui::Button("Done"))
        {
            channelSelection = nullptr;
        }
    }
}

void PhotonicDirectorApp::drawEffectControls()
{

    // Effects ui.
    static EffectRef *effectSelection = nullptr;
    {
        ImGui::ScopedWindow window("Effects");
        // Add the button.
        if (ui::Button("Create Effect"))
        {
            ui::OpenPopup("Create effect");
        }
        ui::SameLine();
        if (ui::Button("All off"))
        {
            for (const auto effect : mEffects)
            {
                effect->isTurnedOn = false;
            }
        }
        if (ui::BeginPopupModal("Create effect"))
        {
            static std::string effectName;
            static std::string filter;
            static std::string previousFilter;
            static int effectType = 0;
            ui::InputText("Name", &effectName);
            auto effectTypes = Effect::getTypes();
            if (ui::InputText("Filter", &filter))
            {
                for (int i = 0; i < effectTypes.size(); i++)
                {
                    if (boost::algorithm::istarts_with(effectTypes.at(i).c_str(), filter))
                    {
                        effectType = i;
                        break;
                    }
                }
            }
            if (ui::BeginCombo("Type", effectTypes[effectType].c_str()))
            {
                bool selectedInFilter = false;
                int firstFilteredItem = -1;
                for (int i = 0; i < effectTypes.size(); i++)
                {
                    if (boost::algorithm::istarts_with(effectTypes.at(i).c_str(), filter))
                    {
                        if (firstFilteredItem < 0)
                        {
                            firstFilteredItem = i;
                        }

                        bool isSelected = (effectType == i);
                        if (ui::Selectable(effectTypes.at(i).c_str(), isSelected))
                        {
                            effectType = i;
                        }
                        if (isSelected)
                        {
                            selectedInFilter = true;
                            ui::SetItemDefaultFocus();
                        }
                    }
                }
                if (!selectedInFilter)
                {
                    effectType = firstFilteredItem;
                }
                ui::EndCombo();
            }
            previousFilter = filter;
            if (ui::Button("Done"))
            {
                // In order to test if a strange exception occurs,
                // close the effect editor for now.
                effectSelection = nullptr;
                mGuiStatusData.pickLightEffect = nullptr;
                mGuiStatusData.status = IDLE;
                mVisualizer.disableEditingMode();

                std::string type = Effect::getTypes()[effectType];
                console() << effectName << ", " << type << endl;
                EffectRef newEffect = Effect::create(type, effectName);
                newEffect->setOscSender(mOscSender);
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
        int effectOrderId = 0;
        // Create colors for the texts.
        ColorA colorActive(0.0, 1.0, 0.0, 1.0);
        ColorA colorInActive(1.0, 0.0, 0.0, 1.0);
        ColorA colorFading(1.0, 1.0, 0.0, 1.0);

        // Drag and drop variables for ordering.
        int moveTo = 0;
        EffectRef effectToMove = nullptr;

        for (auto it = mEffects.begin(); it != mEffects.end();)
        {
            ui::PushID(effectOrderId);
            EffectRef &effectRef = *it;

            // Start drag and drop functionality.
            ui::Button(":::");
            ui::SameLine();
            ImGuiDragDropFlags srcFlags = 0;
            if (ui::BeginDragDropSource(srcFlags))
            {
                ImGui::SetDragDropPayload(EFFECT_SELECT_DRAG_DROP, &effectRef, sizeof(EffectRef));
                ui::Text("Drag to reorder;");
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                ImGuiDragDropFlags targetFlags = 0;
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(EFFECT_SELECT_DRAG_DROP, targetFlags))
                {
                    effectToMove = *(EffectRef *)payload->Data;
                    moveTo = effectOrderId;
                }
                ImGui::EndDragDropTarget();
            }

            ui::Checkbox("", &effectRef->isTurnedOn);
            ui::SameLine();

            float fadeValueContent = effectRef->getFadeValue();
            float fadeValue[] = {fadeValueContent};
            ui::PlotHistogram("", fadeValue, 1, 0, NULL, 0.0f, 1.0f, ImVec2(20, 18));
            ui::SameLine();
            std::string effectName = effectRef->getName() + " (" + effectRef->getTypeName() + ")";
            ui::Text("%s", effectName.c_str());
            if (ui::IsItemClicked())
            {
                effectSelection = &effectRef;
            }
            ui::SameLine();
            if (ui::Button("Remove"))
            {
                mSceneList->onEffectRemove(*it);
                it = mEffects.erase(it);
                effectSelection = nullptr;
                ui::PopID();
                continue;
            }
            ui::SameLine();
            if (ui::Button("Edit"))
            {
                effectSelection = &effectRef;
            }
            ui::SameLine();
            if (ui::Button("Turn on immediately"))
            {
                effectRef->turnOnImmediately();
            }
            ui::SameLine();
            if (ui::Button("Turn off immediately"))
            {
                effectRef->turnOffImmediately();
            }
            ui::SameLine();
            ColorA statusColor;
            switch (effectRef->getStatus())
            {
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
            effectOrderId++;
            it++;
        }
        if (effectToMove != nullptr)
        {
            changeEffectOrder(effectToMove, moveTo);
        }
    }
    if (effectSelection != nullptr)
    {
        EffectRef effect = *effectSelection;
        if (effect)
        {
            ImGui::ScopedWindow window("Effect inspector");

            static std::string name;
            name = effect->getName();
            if (ui::InputText("Name", &name))
            {
                effect->setName(name);
            }
            ui::Separator();
            ui::Checkbox("Enable", &effect->isTurnedOn);
            ui::Separator();
            ////////////////////////////////////////////
            // OSC input.
            ////////////////////////////////////////////
            static std::string oscAddress;
            static bool oscLearn;
            oscAddress = effect->oscAddressForOnOff;
            if (oscLearn)
            {
                effect->oscAddressForOnOff = mLastOscAddress;
            }
            if (ui::InputText("OSC address for on/off", &oscAddress))
            {
                effect->oscAddressForOnOff = oscAddress;
            }
            ui::SameLine();
            ui::Checkbox("OSC learn", &oscLearn);
            // End OSC input.
            ui::Separator();
            ui::InputFloat("Fadein Time", &effect->fadeInTime);
            ui::InputFloat("Fadeout Time", &effect->fadeOutTime);
            ui::SliderFloat("Weight", &effect->weight, 0.0f, 100.0f);
            ui::Spacing();
            if (!ui::IsWindowCollapsed())
            {
                ui::ListBoxHeader("Choose input channel", (int)mChannels.size());
                for (const auto channel : mChannels)
                {
                    if (ui::Selectable(channel->getName().c_str(), channel == effect->getChannel()))
                    {
                        effect->setChannel(channel);
                    }
                }
                ui::ListBoxFooter();
            }
            std::string lightSelectText = mGuiStatusData.status == ADDING_LIGHT_TO_EFFECT ? "Done selecting lights" : "Select/Deselect lights";
            if (ui::Button(lightSelectText.c_str()))
            {
                // Toggle the state of the gui.
                mGuiStatusData.status = (mGuiStatusData.status == IDLE) ? ADDING_LIGHT_TO_EFFECT : IDLE;
                if (mGuiStatusData.status == ADDING_LIGHT_TO_EFFECT)
                {
                    mGuiStatusData.pickLightEffect = *effectSelection;
                    mVisualizer.enableEditingMode();
                }
                else
                {
                    mGuiStatusData.pickLightEffect = nullptr;
                    mVisualizer.disableEditingMode();
                }
            }

            // Create list of lights.
            if (!ui::IsWindowCollapsed())
            {
                if (ui::Button("Add all lights"))
                {
                    for (auto light : mLights)
                    {
                        effect->addLight(light);
                    }
                }
                ui::ListBoxHeader("Lights");
                int lightEditId = 0;
                for (const auto light : effect->getLights())
                {
                    ui::PushID(lightEditId);
                    ui::BulletText("%s", light->mName.c_str());
                    ui::SameLine();
                    if (ui::Button("Remove"))
                    {
                        effect->removeLight(light);
                        ui::PopID();
                        continue;
                    }
                    lightEditId++;
                    ui::PopID();
                }
                ui::ListBoxFooter();
                if (mGuiStatusData.status == ADDING_LIGHT_TO_EFFECT)
                {
                    ui::ListBoxHeader("Lights available");
                    int addingEditId = 0;
                    for (const auto light : mLights)
                    {
                        std::string uuid = light->getUuid();
                        if (!effect->hasLight(light) && effect->supportsLight(light))
                        {
                            ui::PushID(addingEditId);
                            ui::BulletText("%s", light->mName.c_str());
                            ui::SameLine();
                            if (ui::Button("Add"))
                            {
                                effect->addLight(light);
                                ui::PopID();
                                continue;
                            }
                            addingEditId++;
                            ui::PopID();
                        }
                    }
                    ui::ListBoxFooter();
                }
                if (ui::BeginDragDropTarget() && mGuiStatusData.isDraggingLight)
                {
                    ImGuiDragDropFlags target_flags = 0;
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(LIGHT_SELECT, target_flags))
                    {
                        LightRef light = *(LightRef *)payload->Data;
                        effect->addLight(light);
                    }
                }
            }
            ui::Separator();
            // Draw the params.
            std::map<int, Parameter *> params = effect->getOrderedParamsForUI();
            int paramId = 100;
            for (auto &item : params)
            {
                ui::PushID(paramId);
                Parameter *param = item.second;
                switch (param->type)
                {
                case photonic::Parameter::kType_Int:
                    ui::InputInt(param->description.c_str(), &param->intValue);
                    break;

                case photonic::Parameter::kType_Float:
                    ui::InputFloat(param->description.c_str(), &param->floatValue);
                    break;

                case photonic::Parameter::kType_Color:
                    ui::ColorEdit4(param->description.c_str(), &param->colorValue[0]);
                    break;

                case photonic::Parameter::kType_OscTrigger:
                    ui::Checkbox(param->description.c_str(), &param->triggerValue);
                    ui::SameLine();
                    static std::string triggerChannel;
                    triggerChannel = param->oscAdress;
                    if (ui::InputText("Osc address", &triggerChannel))
                    {
                        param->oscAdress = triggerChannel;
                    }
                    break;

                case photonic::Parameter::kType_Vector3:
                    ui::ListBoxHeader("Drop Light", vec2(30, 10));
                    ui::ListBoxFooter();
                    if (ui::BeginDragDropTarget() && mGuiStatusData.isDraggingLight)
                    {
                        ImGuiDragDropFlags target_flags = 0;
                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(LIGHT_SELECT, target_flags))
                        {
                            LightRef light = *(LightRef *)payload->Data;
                            param->vec3Value = vec3(light->position.x, light->position.y, light->position.z);
                        }
                    }
                    ui::SameLine();
                    ui::InputFloat3(param->description.c_str(), &param->vec3Value[0]);
                    break;

                case photonic::Parameter::kType_Light:
                    if (!ui::IsWindowCollapsed())
                    {
                        ui::ListBoxHeader(param->description.c_str(), 1);
                        if (param->lightRef)
                        {
                            ui::Selectable(param->lightRef->mName.c_str());
                        }
                        ui::ListBoxFooter();
                        if (ui::BeginDragDropTarget() && mGuiStatusData.isDraggingLight)
                        {
                            ImGuiDragDropFlags target_flags = 0;
                            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(LIGHT_SELECT, target_flags))
                            {
                                LightRef light = *(LightRef *)payload->Data;
                                param->lightRef = light;
                            }
                        }
                        else
                        {
                            if (!param->lightRef)
                            {
                                ui::Text("Please use drag and drop to assign light");
                            }
                        }

                        if (ui::Button("Reset light"))
                        {
                            param->lightRef = nullptr;
                        }
                    }
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
                    if (!ui::IsWindowCollapsed())
                    {
                        ui::ListBoxHeader(param->description.c_str(), 1);
                        if (param->channelRef)
                        {
                            ui::Selectable(param->channelRef->getName().c_str());
                        }
                        ui::ListBoxFooter();
                        if (ui::BeginDragDropTarget() && mGuiStatusData.isDraggingChannel)
                        {
                            ImGuiDragDropFlags target_flags = 0;
                            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(CHANNEL_SELECT, target_flags))
                            {
                                InputChannelRef channel = *(InputChannelRef *)payload->Data;
                                param->channelRef = channel;
                            }
                        }
                        else
                        {
                            if (!param->channelRef)
                            {
                                ui::Text("Please use drag and drop to assign channel");
                            }
                        }

                        if (ui::Button("Reset channel"))
                        {
                            param->channelRef = nullptr;
                        }
                    }

                default:
                    break;
                }
                ui::PopID();
                paramId++;
            }

            effect->drawEditGui();
            if (ui::Button("Done"))
            {
                effectSelection = nullptr;
                mGuiStatusData.pickLightEffect = nullptr;
                mGuiStatusData.status = IDLE;
                mVisualizer.disableEditingMode();
            }
        }
    }
}

void PhotonicDirectorApp::changeEffectOrder(EffectRef effect, int newPosition)
{
    mEffects.remove(effect);
    auto iterator = mEffects.begin();
    std::advance(iterator, newPosition);
    mEffects.emplace(iterator, effect);
}

void PhotonicDirectorApp::drawDmxInspector()
{
    ImGui::ScopedWindow window("Dmx inspector");
    if (!ui::IsWindowCollapsed())
    {
        auto dmxVisuals = mDmxOut.getVisualizeTexture();
        ui::Image(dmxVisuals, dmxVisuals->getSize());
    }
}

void PhotonicDirectorApp::drawObjectsHierarchy()
{
    ImGui::ScopedWindow window("Objects");
    mGuiStatusData.isDraggingLight = false;
    mGuiStatusData.isDraggingChannel = false;
    if (!ui::IsWindowCollapsed())
    {
        ImGuiDragDropFlags srcFlags = 0;
        if (ui::TreeNode("Lights"))
        {
            for (const auto light : mLights)
            {
                ui::Selectable(light->mName.c_str());
                if (ui::BeginDragDropSource(srcFlags))
                {
                    if (!(srcFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
                    {
                        ui::Text("Move %s to effect", light->mName.c_str());
                    }
                    ui::SetDragDropPayload(LIGHT_SELECT, &light, sizeof(light));
                    ui::EndDragDropSource();
                    mGuiStatusData.isDraggingLight = true;
                }
            }
            ui::TreePop();
        }
        if (ui::TreeNode("Channels"))
        {
            for (const auto channel : mChannels)
            {
                ui::Selectable(channel->getName().c_str());
                if (ui::BeginDragDropSource(srcFlags))
                {
                    if (!(srcFlags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
                    {
                        ui::Text("Move %s to effect", channel->getName().c_str());
                    }
                    ui::SetDragDropPayload(CHANNEL_SELECT, &channel, sizeof(channel));
                    ui::EndDragDropSource();
                    mGuiStatusData.isDraggingChannel = true;
                }
            }
            ui::TreePop();
        }
    }
}

void PhotonicDirectorApp::resize()
{
    mVisualizer.resize();
}

void PhotonicDirectorApp::fileDrop(FileDropEvent event) {
    loadFile(event.getFile(0));
}

void PhotonicDirectorApp::pickLight()
{
    mGuiStatusData.pickedLight = mVisualizer.pickLight(mLights);
}

void PhotonicDirectorApp::draw()
{

    gl::clear(Color(0, 0, 0));
    if (mShowVisualizer)
    {
        mVisualizer.draw(mLights);
    }

    // Delegate some gui drawing to the visualizer.
    if (mGuiStatusData.drawGui)
    {
        switch (mGuiStatusData.status)
        {
        case EDITING_LIGHT:
        case IDLE:
            if (mGuiStatusData.lightToEdit != nullptr)
            {
                mVisualizer.highLightLight(mGuiStatusData.lightToEdit, Color(0.f, 1.f, 1.f));
            }
            break;

        case ADDING_LIGHT_TO_EFFECT:
            if (mGuiStatusData.pickLightEffect != nullptr)
            {
                auto effectLights = mGuiStatusData.pickLightEffect->getLights();
                for (auto light : effectLights)
                {
                    mVisualizer.highLightLight(light, Color(0.f, 1.f, 0.f));
                }
            }

        default:
            break;
        }

        if (mGuiStatusData.pickedLight)
        {
            mVisualizer.highLightLight(mGuiStatusData.pickedLight);
        }
        // If we are calibrating, highlight the light as well.
        if (mLightCalibrator.isCalibrating())
        {
            mVisualizer.highLightLight(mLightCalibrator.getCurrentLight(), Color(53, 252, 0));
        }
    }
    if (mGuiStatusData.visualizeEffects)
    {
        mVisualizer.drawEffects(mEffects);
    }
}

PhotonicDirectorApp::~PhotonicDirectorApp()
{
    // The setting of the dmx channel is needed, because the director app
    // is destroyed first, and then the lights, that try to reach the dmxOuput
    // which is already destroyed.
    for (const auto light : mLights)
    {
        light->setDmxChannel(0);
    }
    Poco::DNSSD::uninitializeDNSSD();
}

CINDER_APP(PhotonicDirectorApp, RendererGl(RendererGl::Options().msaa(8)), [](cinder::app::AppBase::Settings *settings) {
    settings->setTitle("Photonic Director");
    settings->setWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
});
