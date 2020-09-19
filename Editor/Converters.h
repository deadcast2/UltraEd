#ifndef _CONVERTERS_H_
#define _CONVERTERS_H_

#include "Records.h"
#include "Util.h"

namespace nlohmann
{
    template <>
    struct adl_serializer<D3DXVECTOR3>
    {
        static void to_json(json &j, const D3DXVECTOR3 &a)
        {
            j = json {
                { "x", a.x },
                { "y", a.y },
                { "z", a.z }
            };
        }

        static void from_json(const json &j, D3DXVECTOR3 &a)
        {
            j.at("x").get_to(a.x);
            j.at("y").get_to(a.y);
            j.at("z").get_to(a.z);
        }
    };

    template <>
    struct adl_serializer<D3DXMATRIX>
    {
        static void to_json(json &j, const D3DXMATRIX &a)
        {
            D3DXQUATERNION quat;
            D3DXQuaternionRotationMatrix(&quat, &a);

            j = json {
                { "x", quat.x },
                { "y", quat.y },
                { "z", quat.z },
                { "w", quat.w }
            };
        }

        static void from_json(const json &j, D3DXMATRIX &a)
        {
            D3DXQUATERNION quat;

            j.at("x").get_to(quat.x);
            j.at("y").get_to(quat.y);
            j.at("z").get_to(quat.z);
            j.at("w").get_to(quat.w);

            D3DXMatrixRotationQuaternion(&a, &quat);
        }
    };

    template <>
    struct adl_serializer<boost::uuids::uuid>
    {
        static void to_json(json &j, const  boost::uuids::uuid &a)
        {
            j = json {
                { "id", UltraEd::Util::UuidToString(a) }
            };
        }

        static void from_json(const json &j, boost::uuids::uuid &a)
        {
            a = UltraEd::Util::StringToUuid(j.at("id").get<std::string>().c_str());
        }
    };
}

namespace UltraEd
{
    inline void to_json(json &j, const AssetRecord &a)
    {
        j = json {
            { "id", a.id },
            { "type", a.type },
            { "purgeId", a.purgeId },
            { "sourcePath", a.sourcePath },
            { "lastModified", a.lastModified }
        };
    }

    inline void from_json(const json &j, AssetRecord &a)
    {
        j.at("id").get_to(a.id);
        j.at("purgeId").get_to(a.purgeId);
        j.at("type").get_to(a.type);
        j.at("sourcePath").get_to(a.sourcePath);
        j.at("lastModified").get_to(a.lastModified);
    }

    inline void to_json(json &j, const ProjectRecord &p)
    {
        j = json {
            { "name", p.name },
            { "version", p.version },
            { "assets", p.assets }
        };
    }

    inline void from_json(const json &j, ProjectRecord &p)
    {
        j.at("name").get_to(p.name);
        j.at("version").get_to(p.version);
        j.at("assets").get_to(p.assets);
    }
}

#endif
