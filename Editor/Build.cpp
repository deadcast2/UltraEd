#include <regex>
#include "Build.h"
#include "Util.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "Settings.h"
#include "Debug.h"
#include "shlwapi.h"

namespace UltraEd
{
    bool Build::WriteSpecFile(const std::vector<Actor *> &actors)
    {
        std::string specSegments, specIncludes;
        const char *specHeader = "#include <nusys.h>\n\n"
            "beginseg"
            "\n\tname \"code\""
            "\n\tflags BOOT OBJECT"
            "\n\tentry nuBoot"
            "\n\taddress NU_SPEC_BOOT_ADDR"
            "\n\tstack NU_SPEC_BOOT_STACK"
            "\n\tinclude \"codesegment.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\rspboot.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\aspMain.o\""
            "\n\tinclude \"$(ROOT)\\usr\\lib\\PR\\gspF3DEX2.fifo.o\""
            "\nendseg\n";
        const char *specIncludeStart = "\nbeginwave"
            "\n\tname \"main\""
            "\n\tinclude \"code\"";
        const char *specIncludeEnd = "\nendwave";

        std::vector<std::filesystem::path> resourceCache;
        int loopCount = 0;
        for (const auto &actor : actors)
        {
            auto newResName = Util::NewResourceName(loopCount++);

            if (actor->GetType() != ActorType::Model) continue;

            auto model = reinterpret_cast<Model *>(actor);

            if (model == nullptr) continue;

            auto modelPath = Project::GetAssetPath(model->GetModelId());

            if (find(resourceCache.begin(), resourceCache.end(), modelPath) == resourceCache.end())
            {
                std::string id = Util::UuidToString(actor->GetId());
                id.insert(0, Project::BuildPath().string().append("\\")).append(".sos");

                std::string modelName(newResName);
                modelName.append("_M");

                specSegments.append("\nbeginseg\n\tname \"");
                specSegments.append(modelName);
                specSegments.append("\"\n\tflags RAW\n\tinclude \"");
                specSegments.append(id);
                specSegments.append("\"\nendseg\n");

                specIncludes.append("\n\tinclude \"");
                specIncludes.append(modelName);
                specIncludes.append("\"");

                resourceCache.push_back(modelPath);
            }

            const auto texturePath = model->GetTexture()->GetPath();

            if (!texturePath.empty() && find(resourceCache.begin(), resourceCache.end(), texturePath) == resourceCache.end())
            {
                std::string reason;
                if (model->GetTexture()->IsValid(reason))
                {
                    auto path = Project::BuildPath() / texturePath.filename();
                    model->GetTexture()->WritePngData(path);

                    std::string textureName(newResName);
                    textureName.append("_T");

                    specSegments.append("\nbeginseg\n\tname \"");
                    specSegments.append(textureName);
                    specSegments.append("\"\n\tflags RAW\n\tinclude \"");
                    specSegments.append(path.string().c_str());
                    specSegments.append("\"\nendseg\n");

                    specIncludes.append("\n\tinclude \"");
                    specIncludes.append(textureName);
                    specIncludes.append("\"");

                    resourceCache.push_back(texturePath);
                }
                else
                {
                    Debug::Instance().Error(std::string("Invalid texture for model ")
                        .append(model->GetName()).append(": ").append(reason));
                }
            }
        }

        std::string specPath = Util::GetPathFor("Engine\\spec");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(specPath.c_str(), "w"), fclose);
        if (file == NULL) return false;

        std::string slashesNormalized(specHeader);
        slashesNormalized = std::regex_replace(slashesNormalized, std::regex("\\\\"), "\\\\");
        fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file.get());

        slashesNormalized = std::string(specSegments);
        slashesNormalized = std::regex_replace(slashesNormalized, std::regex("\\\\"), "\\\\");
        fwrite(slashesNormalized.c_str(), 1, slashesNormalized.size(), file.get());

        fwrite(specIncludeStart, 1, strlen(specIncludeStart), file.get());
        fwrite(specIncludes.c_str(), 1, specIncludes.size(), file.get());
        fwrite(specIncludeEnd, 1, strlen(specIncludeEnd), file.get());
        return true;
    }

    bool Build::WriteDefinitionsFile()
    {
        char buffer[128];
        std::string mode = Settings::GetVideoMode() == VideoMode::NTSC ? "OS_VI_NTSC_LAN1" : "OS_VI_PAL_LAN1";
        sprintf(buffer, "#define _UER_VIDEO_MODE %s\n", mode.c_str());

        std::string path = Util::GetPathFor("Engine\\definitions.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(path.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(buffer, 1, strlen(buffer), file.get());
        return true;
    }

    bool Build::WriteSegmentsFile(const std::vector<Actor *> &actors,
        std::map<std::filesystem::path, std::string> *resourceCache)
    {
        std::string romSegments;
        int loopCount = 0;
        for (const auto &actor : actors)
        {
            auto newResName = Util::NewResourceName(loopCount++);

            if (actor->GetType() != ActorType::Model) continue;

            auto model = reinterpret_cast<Model *>(actor);

            if (model == nullptr) continue;

            auto modelPath = Project::GetAssetPath(model->GetModelId());

            if (resourceCache->find(modelPath) == resourceCache->end())
            {
                std::string modelName(newResName);
                modelName.append("_M");

                romSegments.append("extern u8 _");
                romSegments.append(modelName);
                romSegments.append("SegmentRomStart[];\n");
                romSegments.append("extern u8 _");
                romSegments.append(modelName);
                romSegments.append("SegmentRomEnd[];\n");

                (*resourceCache)[modelPath] = newResName;
            }

            auto texturePath = Project::GetAssetPath(model->GetTexture()->GetId());
            if (!texturePath.empty() && resourceCache->find(texturePath) == resourceCache->end())
            {
                std::string textureName(newResName);
                textureName.append("_T");

                romSegments.append("extern u8 _");
                romSegments.append(textureName);
                romSegments.append("SegmentRomStart[];\n");
                romSegments.append("extern u8 _");
                romSegments.append(textureName);
                romSegments.append("SegmentRomEnd[];\n");

                (*resourceCache)[texturePath] = newResName;
            }
        }

        std::string segmentsPath = Util::GetPathFor("Engine\\segments.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(segmentsPath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(romSegments.c_str(), 1, romSegments.size(), file.get());
        return true;
    }

    bool Build::WriteSceneFile(Scene *scene)
    {
        char buffer[128];
        COLORREF bgColor = scene->GetBackgroundColor();
        sprintf(buffer, "int _UER_SceneBackgroundColor[3] = { %i, %i, %i };\n", GetRValue(bgColor),
            GetGValue(bgColor), GetBValue(bgColor));

        std::string scenePath = Util::GetPathFor("Engine\\scene.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(scenePath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(buffer, 1, strlen(buffer), file.get());
        return true;
    }

    bool Build::WriteActorsFile(const std::vector<Actor *> &actors,
        const std::map<std::filesystem::path, std::string> &resourceCache)
    {
        int actorCount = -1;
        
        std::string actorsArrayDef("vector _UER_Actors = NULL;\nvector _UER_ActorsPendingRemoval = NULL;");
        actorsArrayDef.append("\nActor *_UER_ActiveCamera = NULL;\n");

        std::string actorInits("\n\t_UER_Actors = vector_create();\_UER_ActorsPendingRemoval = vector_create();\n");

        std::map<boost::uuids::uuid, int> reducedActorIds;
        
        for (const auto &actor : actors)
        {
            std::string resourceName = Util::NewResourceName(++actorCount);
            actorInits.append("\n\tvector_add(_UER_Actors, ");

            D3DXVECTOR3 colliderCenter = actor->HasCollider() ? actor->GetCollider()->GetCenter() : D3DXVECTOR3(0, 0, 0);
            FLOAT colliderRadius = actor->HasCollider() && actor->GetCollider()->GetType() == ColliderType::Sphere ?
                dynamic_cast<SphereCollider *>(actor->GetCollider())->GetRadius() : 0.0f;
            D3DXVECTOR3 colliderExtents = actor->HasCollider() && actor->GetCollider()->GetType() == ColliderType::Box ?
                dynamic_cast<BoxCollider *>(actor->GetCollider())->GetExtents() : D3DXVECTOR3(0, 0, 0);

            // Store assigned int id to use for child linking.
            reducedActorIds[actor->GetId()] = actorCount;

            if (actor->GetType() == ActorType::Model)
            {
                auto model = reinterpret_cast<Model *>(actor);
                auto modelPath = Project::GetAssetPath(model->GetModelId());
                auto texturePath = Project::GetAssetPath(model->GetTexture()->GetId());

                if (resourceCache.find(modelPath) != resourceCache.end())
                    resourceName = resourceCache.at(modelPath);

                std::string modelName(resourceName);
                modelName.append("_M");

                if (!texturePath.empty())
                    actorInits.append("CActor_LoadTexturedModel(").append(std::to_string(actorCount)).append(", \"").append(model->GetName()).append("\", _");
                else
                    actorInits.append("CActor_LoadModel(").append(std::to_string(actorCount)).append(", \"").append(model->GetName()).append("\", _");

                actorInits.append(modelName).append("SegmentRomStart, _").append(modelName).append("SegmentRomEnd");

                if (!texturePath.empty())
                {
                    if (resourceCache.find(texturePath) != resourceCache.end())
                        resourceName = resourceCache.at(texturePath);

                    std::string textureName(resourceName);
                    textureName.append("_T");

                    auto dimensions = model->GetTexture()->Dimensions();
                    actorInits.append(", _").append(textureName).append("SegmentRomStart, _")
                        .append(textureName).append("SegmentRomEnd, ").append(std::to_string(dimensions[0])).append(", ")
                        .append(std::to_string(dimensions[1]));
                }

                // Add transform data.
                char vectorBuffer[256];
                D3DXVECTOR3 position = actor->GetPosition(false), scale = actor->GetScale(), axis;
                float angle;
                actor->GetAxisAngle(&axis, &angle);
                sprintf(vectorBuffer, ", %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                    position.x, position.y, position.z,
                    axis.x, axis.y, axis.z, angle * (180.0 / D3DX_PI),
                    scale.x, scale.y, scale.z,
                    colliderCenter.x, colliderCenter.y, colliderCenter.z, colliderRadius,
                    colliderExtents.x, colliderExtents.y, colliderExtents.z,
                    actor->HasCollider() ? actor->GetCollider()->GetName() : "None");
                actorInits.append(vectorBuffer).append("));\n");

                // Write out mesh data.
                std::vector<Vertex> vertices = actor->GetVertices();
                std::string id = Util::UuidToString(actor->GetId());
                id.insert(0, Project::BuildPath().string().append("\\")).append(".sos");
                FILE *file = fopen(id.c_str(), "w");
                if (file == NULL) return false;
                fprintf(file, "%i\n", static_cast<int>(vertices.size()));
                for (size_t i = 0; i < vertices.size(); i++)
                {
                    Vertex vert = vertices[i];
                    D3DXCOLOR color(vert.color);
                    fprintf(file, "%f %f %f %f %f %f %f %f %f\n", vert.position.x, vert.position.y, vert.position.z,
                        color.r, color.g, color.b, color.a, vert.tu, vert.tv);
                }
                fclose(file);
            }
            else if (actor->GetType() == ActorType::Camera)
            {
                actorInits.append("CActor_CreateCamera(").append(std::to_string(actorCount)).append(", \"").append(actor->GetName()).append("\", ");
                char vectorBuffer[256];
                D3DXVECTOR3 position = actor->GetPosition(false);
                D3DXVECTOR3 axis;
                float angle;
                actor->GetAxisAngle(&axis, &angle);
                sprintf(vectorBuffer, "%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %s",
                    position.x, position.y, position.z,
                    axis.x, axis.y, axis.z, angle * (180.0 / D3DX_PI),
                    colliderCenter.x, colliderCenter.y, colliderCenter.z, colliderRadius,
                    colliderExtents.x, colliderExtents.y, colliderExtents.z,
                    actor->HasCollider() ? actor->GetCollider()->GetName() : "None");
                actorInits.append(vectorBuffer).append("));\n");
            }
        }

        for (const auto &actor : actors)
        {
            for (const auto &child : actor->GetChildren())
            {
                actorInits.append("\n\tCActor_LinkChildToParent(_UER_Actors, ")
                    .append("vector_get(_UER_Actors, ").append(std::to_string(reducedActorIds[child->GetId()])).append(")")
                    .append(", vector_get(_UER_Actors, ").append(std::to_string(reducedActorIds[actor->GetId()])).append("));\n");
            }
        }

        std::string actorInitsPath = Util::GetPathFor("Engine\\actors.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(actorInitsPath.c_str(), "w"), fclose);
        if (file == NULL) return false;

        fwrite(actorsArrayDef.c_str(), 1, actorsArrayDef.size(), file.get());

        const char *actorInitStart = "\nvoid _UER_Load() {";
        fwrite(actorInitStart, 1, strlen(actorInitStart), file.get());
        fwrite(actorInits.c_str(), 1, actorInits.size(), file.get());
        fwrite("}\n\n", 1, 3, file.get());

        auto snippet = Util::GetSnippet("ActorUpdate.c");
        fwrite(snippet.c_str(), 1, snippet.size(), file.get());

        return true;
    }

    bool Build::WriteCollisionFile(const std::vector<Actor *> &actors)
    {
        std::string collisionPath = Util::GetPathFor("Engine\\collisions.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(collisionPath.c_str(), "w"), fclose);
        if (file == NULL) return false;

        auto snippet = Util::GetSnippet("ActorCollision.c");
        fwrite(snippet.c_str(), 1, snippet.size(), file.get());

        return true;
    }

    bool Build::WriteScriptsFile(const std::vector<Actor *> &actors)
    {
        std::string scriptStartStart("void _UER_Start() {");
        std::string scripts;
        char countBuffer[10];
        int actorCount = -1;

        for (const auto &actor : actors)
        {
            std::string actorRef;
            actorRef.append("vector_get(_UER_Actors, ");

            std::string newResName = Util::NewResourceName(++actorCount);
            std::string script = actor->GetScript();
            auto result = Util::ReplaceString(script, "$", newResName);

            _itoa(actorCount, countBuffer, 10);
            actorRef.append(countBuffer).append(")");
            scripts.append(result).append("\n");

            scriptStartStart.append("\n\t");

            if (scripts.find(std::string(newResName).append("Start(")) != std::string::npos)
            {
                scriptStartStart.append(actorRef).append("->start = ").append(newResName).append("Start").append(";\n\t");
                scriptStartStart.append(actorRef).append("->start(").append(actorRef).append(");\n\t");
            }

            if (scripts.find(std::string(newResName).append("Update(")) != std::string::npos)
            {
                scriptStartStart.append(actorRef).append("->update = ").append(newResName).append("Update").append(";\n\t");
            }

            if (scripts.find(std::string(newResName).append("Input(")) != std::string::npos)
            {
                scriptStartStart.append(actorRef).append("->input = ").append(newResName).append("Input").append(";\n\t");
            }

            if (scripts.find(std::string(newResName).append("Collide(")) != std::string::npos)
            {
                scriptStartStart.append(actorRef).append("->collide = ").append(newResName).append("Collide").append(";\n");
            }
        }

        std::string scriptsPath = Util::GetPathFor("Engine\\scripts.h");
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(scriptsPath.c_str(), "w"), fclose);
        if (file == NULL) return false;
        fwrite(scripts.c_str(), 1, scripts.size(), file.get());
        fwrite(scriptStartStart.c_str(), 1, scriptStartStart.size(), file.get());
        fwrite("}", 1, 1, file.get());
        return true;
    }

    bool Build::Start(Scene *scene)
    {
        const auto actors = scene->GetActors();

        // Share texture and model data to reduce ROM size. Resource use is tracked during
        // segment generation and the actor script generator uses that info. 
        std::map<std::filesystem::path, std::string> resourceCache;
        WriteSegmentsFile(actors, &resourceCache);
        WriteActorsFile(actors, resourceCache);

        WriteSpecFile(actors);
        WriteDefinitionsFile();
        WriteCollisionFile(actors);
        WriteScriptsFile(actors);
        WriteSceneFile(scene);

        return Compile();
    }

    bool Build::Run()
    {
        // Get the path to where the program is running.
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            DWORD exitCode;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            // Format the path to execute the ROM build.
            std::string currDir = Util::GetPathFor("Player");

            // Start the build with no window.
            CreateProcess(NULL, const_cast<LPSTR>("cmd /c cen64.exe pifdata.bin ..\\Engine\\main.n64"), NULL, NULL, FALSE,
                CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return exitCode == 0;
        }

        return false;
    }

    bool Build::Load()
    {
        // Get the path to where the program is running.
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            HANDLE stdOutRead = NULL;
            HANDLE stdOutWrite = NULL;

            SECURITY_ATTRIBUTES securityAttrs;
            securityAttrs.nLength = sizeof(SECURITY_ATTRIBUTES);
            securityAttrs.bInheritHandle = TRUE;
            securityAttrs.lpSecurityDescriptor = NULL;

            if (!CreatePipe(&stdOutRead, &stdOutWrite, &securityAttrs, 0))
                Debug::Instance().Error("Could not create pipe.");

            if (!SetHandleInformation(stdOutRead, HANDLE_FLAG_INHERIT, 0))
                Debug::Instance().Error("Could set handle information for pipe");

            DWORD exitCode;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.hStdError = stdOutWrite;
            si.hStdOutput = stdOutWrite;
            si.dwFlags |= STARTF_USESTDHANDLES;
            ZeroMemory(&pi, sizeof(pi));

            // Format the path to execute the ROM build.
            std::string currDir = Util::GetPathFor("Player\\USB");

            // Start the USB loader with no window.
            std::string command = Settings::GetBuildCart() == BuildCart::_64drive ?
                "cmd /c 64drive_usb.exe -l ..\\..\\Engine\\main.n64 -c 6102" :
                "cmd /c usb64.exe -rom=..\\..\\Engine\\main.n64 -start";
            CreateProcess(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, TRUE,
                CREATE_NO_WINDOW, NULL, currDir.c_str(), &si, &pi);

            DWORD dwRead;
            CHAR chBuf[4096];
            CloseHandle(stdOutWrite);

            // Send the cart loading results to the output window.
            while (ReadFile(stdOutRead, chBuf, 4096, &dwRead, NULL))
            {
                if (dwRead == 0) break;
                auto buffer = std::make_unique<char[]>(dwRead + 1); // Add 1 to prevent garbage.

                if (buffer)
                {
                    memcpy(buffer.get(), chBuf, dwRead);
                    Debug::Instance().Info(std::string(buffer.get()));
                }
            }

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CloseHandle(stdOutRead);

            return exitCode == 0;
        }

        return false;
    }

    bool Build::Compile()
    {
        // Set the root env variable for the N64 build tools.
        SetEnvironmentVariable("ROOT", "..\\Engine\\n64sdk\\ultra");

        // Get the path to where the program is running.
        char buffer[MAX_PATH];
        if (GetModuleFileName(NULL, buffer, MAX_PATH) > 0 && PathRemoveFileSpec(buffer) > 0)
        {
            HANDLE stdOutRead = NULL;
            HANDLE stdOutWrite = NULL;

            SECURITY_ATTRIBUTES securityAttrs;
            securityAttrs.nLength = sizeof(SECURITY_ATTRIBUTES);
            securityAttrs.bInheritHandle = TRUE;
            securityAttrs.lpSecurityDescriptor = NULL;

            if (!CreatePipe(&stdOutRead, &stdOutWrite, &securityAttrs, 0))
                Debug::Instance().Error("Could not create pipe");

            if (!SetHandleInformation(stdOutRead, HANDLE_FLAG_INHERIT, 0))
                Debug::Instance().Error("Could set handle information for pipe");

            DWORD exitCode;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.hStdError = stdOutWrite;
            si.hStdOutput = stdOutWrite;
            si.dwFlags |= STARTF_USESTDHANDLES;
            ZeroMemory(&pi, sizeof(pi));

            // Format the path to execute the ROM build.
            std::string currDir = Util::GetPathFor("Engine");

            // Start the build with no window.
            CreateProcess(NULL, const_cast<LPSTR>("cmd /c build.bat"), NULL, NULL, TRUE, CREATE_NO_WINDOW,
                NULL, currDir.c_str(), &si, &pi);

            DWORD dwRead;
            CHAR chBuf[4096];
            CloseHandle(stdOutWrite);

            // Send the build results to the console window.
            while (ReadFile(stdOutRead, chBuf, 4096, &dwRead, NULL))
            {
                if (dwRead == 0) break;
                auto buffer = std::make_unique<char[]>(dwRead + 1); // Add 1 to prevent garbage.

                if (buffer)
                {
                    memcpy(buffer.get(), chBuf, dwRead);
                    Debug::Instance().Info(std::string(buffer.get()));
                }
            }

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            CloseHandle(stdOutRead);

            return exitCode == 0;
        }

        return false;
    }
}
