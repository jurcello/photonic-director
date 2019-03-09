//
// Created by  on 2019-03-09.
//

#ifndef PHOTONICDIRECTOR_VUMeter
#define PHOTONICDIRECTOR_VUMeter

#include "../Effects.h"
#include "CloseInLocation.h"

// TODO: I don't know if it is a good idea to use closeinlocation.
// Might be just better to use an own class for this.

namespace photonic {

    class VUMeter : public CloseInLocation {
    public:

        explicit VUMeter(std::string name, std::string uuid = "");

        void execute(double dt) override;

        std::string getTypeName() override;

        std::string getTypeClassName() override;

    };

}

#endif //PHOTONICDIRECTOR_VUMeter
