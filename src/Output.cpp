//
//  Output.cpp
//  PhotonicDirector
//
//  Created by Jur de Vries on 06/11/2017.
//

#include "Output.h"
#include "cinder/app/App.h"
#include "DMXPro.h"

using namespace cinder;
using namespace cinder::app;

DmxOutput::DmxOutput()
:mOut{0}, mWidth(320), mHeight(320), mDmxPro(nullptr), mDmxProIsConnected(false)
{
    gl::Fbo::Format format;
    format.colorTexture();
    mFbo = gl::Fbo::create(mWidth, mHeight, format);
}

void DmxOutput::setChannelValue(int channel, int value)
{
    mOut[channel - 1] = value;
//    if (mDmxPro != nullptr && mDmxPro->isConnected()) {
//        mDmxPro->setValue(value, channel - 1);
//    }
}

void DmxOutput::setChannelValue(int channel, float value)
{
    int intValue = 0;
    if (value < 0.f) {
        intValue = 0;
    }
    else if (value > 1.0f) {
        intValue = 255;
    }
    else {
        intValue = 255 * value;
    }
    setChannelValue(channel, intValue);
}

int DmxOutput::getChannelValue(int channel)
{
    return mOut[channel - 1];
}

void DmxOutput::reset()
{
    std::fill_n(mOut, 512, 0);
}

void DmxOutput::update()
{
    if (mDmxPro != nullptr && mDmxPro->isConnected()) {
        for (int i = 0; i < 256; i++) {
            mDmxPro->setValue(mOut[i], i);
        }
    }
}

std::vector<std::string> DmxOutput::getDevicesList() {
    std::vector<std::string> devices = DMXPro::getDevicesList();
    std::vector<std::string> dmxDevices;
    for (auto deviceName : devices) {
        if (deviceName.find("tty.usbserial") != std::string::npos) {
            dmxDevices.push_back(deviceName);
        }
    }
    return dmxDevices;
}

void DmxOutput::connect(std::string deviceName)
{
    if (!mDmxProIsConnected) {
        console() << "Starting connection" << std::endl;
        mDmxPro = DMXPro::create(deviceName);
        mDmxProIsConnected = true;
    }
}

void DmxOutput::disConnect()
{
    mDmxPro = nullptr;
    mDmxProIsConnected = false;
}

bool DmxOutput::isConnected()
{
    return mDmxProIsConnected;
}

std::string DmxOutput::getConnectedDevice()
{
    if (mDmxProIsConnected) {
        return mDmxPro->getDeviceName();
    }
    return "";
}

bool DmxOutput::registerChannel(int channel, std::string uid)
{
    auto existingRecord = mChannelRegistry.find(channel);
    if (existingRecord == mChannelRegistry.end()) {
        mChannelRegistry[channel] = uid;
        return true;
    }
    else {
        if (existingRecord->second == uid) {
            return true;
        }
    }
    return false;
}

void DmxOutput::releaseChannels(std::string uid)
{
    if (! mChannelRegistry.empty()) {
        for (auto it = mChannelRegistry.begin(); it != mChannelRegistry.end(); ) {
            if (it->second == uid) {
                it = mChannelRegistry.erase(it);
                continue;
            }
            it++;
        }
    }
}

void DmxOutput::visualize()
{
    gl::draw(getVisualizeTexture(), vec2(10,10));
}

gl::TextureRef DmxOutput::getVisualizeTexture()
{
    gl::ScopedFramebuffer fb(mFbo);
    gl::ScopedBlendAlpha sAb;
    gl::ScopedViewport vp(ivec2(0), mFbo->getSize());
    gl::pushMatrices();
    gl::setMatricesWindow(mFbo->getSize());
    gl::clear(Color(0.3f, 0.3f, 0.3f));
    int gutter = 2;
    int width = (mWidth / 16) - gutter;
    int height = (mHeight / 16) - gutter;
    int textMargin = 3;
    for (int i = 0; i < 16; i++ ) {
        int yPos = i * (width + gutter) + gutter / 2;
        for (int j = 0; j < 16; j++) {
            int xPos = j * (width + gutter) + gutter / 2;
            // Draw a rectangle.
            int value = mOut[i * 16 + j];
            gl::ScopedColor color(0.f, value / 256.f, 0.f);
            gl::drawSolidRect(Rectf(vec2(xPos, yPos), vec2(xPos + width, yPos + height)));
            gl::drawStringCentered(std::to_string(value), vec2(xPos + gutter + width / 2, yPos + gutter + textMargin));
        }
    }
    gl::popMatrices();
    return mFbo->getColorTexture();
}
