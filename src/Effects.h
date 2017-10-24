//
//  Effects.hpp
//  PhotonicDirector
//
//  Created by Jur de Vries on 24/10/2017.
//

#ifndef Effects_h
#define Effects_h

#include <stdio.h>
#include "cinder/app/App.h"

class InputChannel;
typedef std::shared_ptr<InputChannel> InputChannelRef;

class InputChannel {
public:
    static InputChannelRef create(std::string address);
    void setAdrress(std::string address);
    void setValue(float value);
    float getValue();
    std::string getAddress();
    
protected:
    InputChannel(std::string address);
    float mValue;
    std::string mAddress;
};



#endif /* Effects_h */
