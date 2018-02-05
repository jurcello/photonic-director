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
static klass##Factory global_##klass##Factory; \

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
    
    struct Parameter {
        enum Type {
            kType_float,
            kType_int,
        };
        Parameter();
        Parameter(Type type, std::string description = "");
      
        Type type;
        float floatValue;
        int intValue;
        std::string description;
    };
    
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
        enum Status {
            kStatus_On,
            kStatus_Off,
            kStatus_FadingOut,
            kStatus_FadingIn,
        };
        
        static void registerType(const std::string type, EffectFactory* factory);
        static EffectRef create(std::string type, std::string name);
        static EffectRef create(std::string type, std::string name, std::string uuid);
        static std::vector<std::string> getTypes();
        
        Effect(std::string name, std::string uuid = "");
        ~Effect();
        
        std::string getUuid();
        std::string getName();
        std::string getStatusName();
        Status getStatus();
        double getFadeValue();
        
        void setName(std::string name);
        void addLight(Light* light);
        void removeLight(Light* light);
        void toggleLight(Light* light);
        bool hasLight(Light* light);
        std::vector<Light*> getLights();
        void setChannel(InputChannelRef channel);
        InputChannelRef getChannel();
        // Params section.
        Parameter* getParam(int index);
        std::map<int, Parameter*>& getParams();
        
        
        virtual std::string getTypeClassName() = 0;
        virtual std::string getTypeName() = 0;
        virtual void drawEditGui();
        virtual void execute(float dt);
        
        // Public accessable variables. Part of the interface!
        float fadeTime;
        bool isTurnedOn;
        
    protected:
        std::string mUuid;
        std::string mName;
        std::vector<Light*> mLights;
        InputChannelRef mChannel;
        Status mStatus;
        double mStatusChangeTime;
        double mFadeValue;
        std::map<int, Parameter*> mParams;
        
    private:
        static std::map<std::string, EffectFactory*> factories;
        static std::vector<std::string> types;
    };
    
    class SimpleVolumeEffect : public Effect {
    public:
        SimpleVolumeEffect(std::string name, std::string uuid = ""): Effect(name, uuid){};
        
        virtual void execute(float dt);
        virtual std::string getTypeName();
        virtual std::string getTypeClassName();
    };
    
    class StaticValueEffect : public Effect {
    public:
        enum Inputs {
            kInput_Volume = 1
        };
        
        // Todo: move implementation to the implementation file.
        StaticValueEffect(std::string name, std::string uuid = "");
        
        virtual void execute(float dt) override;
        virtual std::string getTypeName() override;
        virtual std::string getTypeClassName() override;

    };
}

namespace ph = photonic;

#endif /* Effects_h */
