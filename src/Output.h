//
//  Output.hpp
//  PhotonicDirector
//
//  Created by Jur de Vries on 06/11/2017.
//

#ifndef Output_hpp
#define Output_hpp

#include <stdio.h>
#include "DMXPro.h"

class DmxOutput {
public:
    DmxOutput();
    void setChannelValue(int channel, int value);
    void setChannelValue(int channel, float value);
    int getChannelValue(int channel);
    void reset();
    
    std::vector<std::string> getDevicesList();
    void connect(std::string deviceName);
    void disConnect();
    bool isConnected();
    std::string getConnectedDevice();
    
    // Should this be in a separate class?
    void visualize();
    cinder::gl::TextureRef getVisualizeTexture();
    
private:
    int mOut[512];
    cinder::gl::FboRef mFbo;
    int mWidth, mHeight;
    
    DMXProRef mDmxPro;
    bool mDmxProIsConnected;
};

#endif /* Output_hpp */
