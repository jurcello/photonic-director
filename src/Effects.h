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
#include <ctime>
#include "Light.h"

namespace photonic {
    class InputChannel;
    typedef std::shared_ptr<InputChannel> InputChannelRef;
    
    class InputChannel {
    public:
        static InputChannelRef create(std::string name, std::string address, std::string uuid = "");
        void setAdrress(std::string address);
        void setName(const std::string name);
        void setValue(float value);
        float getValue();
        std::string getUuid();
        std::string getAddress();
        std::string getName() const;
        
    protected:
        InputChannel(std::string name, std::string address, std::string uuid = "");
        float mValue;
        std::string mUuid;
        std::string mAddress;
        std::string mName;
    };
    
    class Effect;
    typedef std::shared_ptr<Effect> EffectRef;
    
    class Effect {
    public:
        static EffectRef create(std::string name);
        static EffectRef create(std::string name, std::string uuid);
        Effect(std::string name);
        Effect(std::string name, std::string uuid);
        std::string getUuid();
        std::string getName();
        void setName(std::string name);
        void addLight(Light* light);
        void removeLight(Light* light);
        std::vector<Light*> getLights();
        void setChannel(InputChannelRef channel);
        InputChannelRef getChannel();
        
        void execute(float dt);
        
    protected:
        std::string mUuid;
        std::string mName;
        std::vector<Light*> mLights;
        InputChannelRef mChannel;
    };
}

namespace ph = photonic;

#endif /* Effects_h */
