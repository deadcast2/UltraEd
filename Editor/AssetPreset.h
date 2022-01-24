#ifndef _ASSETPRESET_H_
#define _ASSETPRESET_H_

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>
#include "Flags.h"

namespace UltraEd
{
    class AssetPreset
    {
    public:
        static bool IsPresetID(const boost::uuids::uuid &id)
        {
            return id == PumpkinModelID() || id == PumpkinTextureID();
        }

        static std::filesystem::path GetPath(const boost::uuids::uuid &id)
        {
            if (id == PumpkinModelID())
                return std::filesystem::path("assets/pumpkin.dae");
            else if (id == PumpkinTextureID())
                return std::filesystem::path("assets/pumpkin.png");

            return std::filesystem::path();
        }

        static const boost::uuids::uuid PumpkinModelID()
        {
            boost::uuids::string_generator gen;

            return gen("c843c154-d32c-4a3c-80e3-228ff34a85a9");
        }

        static const boost::uuids::uuid PumpkinTextureID()
        {
            boost::uuids::string_generator gen;

            return gen("f9bbff56-bbc7-4513-9ff6-2b5211fccd78");
        }
    };
}

#endif
