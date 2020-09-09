#ifndef _CONVERTERS_H_
#define _CONVERTERS_H_

#include "Records.h"
#include "Util.h"

namespace UltraEd
{
    inline void to_json(json &j, const AssetRecord &a)
    {
        j = json {
            { "id", Util::UuidToString(a.id) },
            { "type", a.type },
            { "purgeId", Util::UuidToString(a.purgeId) },
            { "sourcePath", a.sourcePath },
            { "lastModified", a.lastModified }
        };
    }

    inline void from_json(const json &j, AssetRecord &a)
    {
        a.id = Util::StringToUuid(j.at("id").get<std::string>().c_str());
        a.purgeId = Util::StringToUuid(j.at("purgeId").get<std::string>().c_str());
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
