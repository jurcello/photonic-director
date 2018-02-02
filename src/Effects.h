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

#define REGISTER_TYPE(klass) \
class klass##Factory : public EffectFactory { \
public: \
    klass##Factory() { \
        Effect::registerType(#klass, this); \
    } \
    virtual EffectRef create(std::string name) { \
        return EffectRef(new klass(name)); \
    } \
    virtual EffectRef create(std::string name, std::string uuid) { \
        return EffectRef(new klass(name, uuid)); \
    } \
}; \
static klass##Factory global_##klass##Factory;

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
    
    // Todo: refactor this to the object factory as described at:
    // https://blog.noctua-software.com/object-factory-c++.html
    class Effect;
    typedef std::shared_ptr<Effect> EffectRef;
    
    // Virtual factory to create new effects.
    class EffectFactory {
    public:
        virtual EffectRef create(std::string name) = 0;
        virtual EffectRef create(std::string name, std::string uuid) = 0;
    };
    
    class Effect {
    public:
        static void registerType(const std::string type, EffectFactory* factory);
        static EffectRef create(std::string type, std::string name);
        static EffectRef create(std::string type, std::string name, std::string uuid);
        static std::vector<std::string> getTypes();
        
        Effect(std::string name);
        Effect(std::string name, std::string uuid);
        
        std::string getUuid();
        std::string getName();
        void setName(std::string name);
        void addLight(Light* light);
        void removeLight(Light* light);
        void toggleLight(Light* light);
        bool hasLight(Light* light);
        std::vector<Light*> getLights();
        void setChannel(InputChannelRef channel);
        InputChannelRef getChannel();
        
        virtual std::string getTypeClassName() = 0;
        virtual void execute(float dt) = 0;
        virtual std::string getTypeName() = 0;
        
    protected:
        std::string mUuid;
        std::string mName;
        std::vector<Light*> mLights;
        InputChannelRef mChannel;
        
    private:
        static std::map<std::string, EffectFactory*> factories;
        static std::vector<std::string> types;
    };
    
    class SimpleVolumeEffect : public Effect {
    public:
        SimpleVolumeEffect(std::string name): Effect(name){};
        SimpleVolumeEffect(std::string name, std::string uuid): Effect(name, uuid){};
        
        virtual void execute(float dt);
        virtual std::string getTypeName();
        virtual std::string getTypeClassName();
    };
    
    class StaticValueEffect : public Effect {
    public:
        StaticValueEffect(std::string name): Effect(name){};
        StaticValueEffect(std::string name, std::string uuid): Effect(name, uuid){};
        
        virtual void execute(float dt);
        virtual std::string getTypeName();
        virtual std::string getTypeClassName();
    };
}

namespace ph = photonic;

#endif /* Effects_h */
