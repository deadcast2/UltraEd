#include <shlwapi.h>
#include <FastLZ/fastlz.h>
#include <fstream>
#include <istream>
#include <sstream>
#include "Common.h"
#include "FileIO.h"
#include "Util.h"

namespace UltraEd
{
    bool FileIO::Save(Scene *scene, const std::filesystem::path &path)
    {
        std::string file = path.string();

        // Add the extension if not supplied in the dialog.
        if (file.find(APP_SCENE_FILE_EXT) == std::string::npos)
        {
            file.append(APP_SCENE_FILE_EXT);
        }

        std::ofstream writer(file);
        if (writer)
        {
            writer << scene->Save().dump(1);
            return true;
        }

        return false;
    }

    bool FileIO::Load(std::shared_ptr<nlohmann::json> &data, const std::filesystem::path &path)
    {
        std::ifstream reader(path);
        if (reader)
        {
            std::stringstream stream;
            stream << reader.rdbuf();
            data = std::make_shared<nlohmann::json>(json::parse(stream.str()));

            return true;
        }

        return false;
    }

    bool FileIO::Compress(const std::string &path)
    {
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(path.c_str(), "rb"), fclose);
        if (file == NULL) return false;

        // Get the total size of the file.
        fseek(file.get(), 0, SEEK_END);
        size_t size = ftell(file.get());
        rewind(file.get());

        // Read in entire file.
        std::unique_ptr<char, decltype(free) *> data((char *)malloc(size), free);
        if (data == NULL) return false;
        size_t bytesRead = fread(data.get(), 1, size, file.get());
        if (bytesRead != size) return false;

        // Compressed buffer must be at least 5% larger.
        std::unique_ptr<char, decltype(free) *> compressed((char *)malloc(size + static_cast<size_t>(size * 0.05f)), free);
        if (compressed == NULL) return false;
        int bytesCompressed = fastlz_compress(data.get(), static_cast<int>(size), compressed.get());
        if (bytesCompressed == 0) return false;

        // Annotate compressed data with uncompressed size.
        int annotatedSize = bytesCompressed + sizeof(int);
        std::unique_ptr<char, decltype(free) *> buffer((char *)malloc(annotatedSize), free);
        memcpy(buffer.get(), &size, sizeof(int));
        memcpy(buffer.get() + sizeof(int), compressed.get(), bytesCompressed);

        // Write compressed file back out.
        file = std::unique_ptr<FILE, decltype(fclose) *>(fopen(path.c_str(), "wb"), fclose);
        if (file == NULL) return false;

        size_t bytesWritten = fwrite(buffer.get(), 1, annotatedSize, file.get());
        return bytesWritten == annotatedSize;
    }

    bool FileIO::Decompress(std::string &path)
    {
        std::unique_ptr<FILE, decltype(fclose) *> file(fopen(path.c_str(), "rb"), fclose);
        if (file == NULL) return false;

        // Get the total size of the file.
        fseek(file.get(), 0, SEEK_END);
        size_t size = ftell(file.get());
        rewind(file.get());

        // Read in entire file.
        std::unique_ptr<char, decltype(free) *> data((char *)malloc(size), free);
        if (data == NULL) return false;
        size_t bytesRead = fread(data.get(), 1, size, file.get());
        if (bytesRead != size) return false;

        // Read the uncompressed file length.
        int uncompressedSize = 0;
        memmove(&uncompressedSize, data.get(), sizeof(int));
        memmove(data.get(), data.get() + sizeof(int), static_cast<int>(size) - sizeof(int));

        std::unique_ptr<char, decltype(free) *> decompressed((char *)malloc(uncompressedSize), free);
        if (decompressed == NULL) return false;
        int bytesDecompressed = fastlz_decompress(data.get(), static_cast<int>(size) - sizeof(int), decompressed.get(),
            uncompressedSize);
        if (bytesDecompressed == 0) return false;

        // Write decompressed file back out.
        file = std::unique_ptr<FILE, decltype(fclose) *>(fopen(path.append(".tmp").c_str(), "wb"), fclose);
        if (file == NULL) return false;

        size_t bytesWritten = fwrite(decompressed.get(), 1, bytesDecompressed, file.get());
        return bytesWritten == bytesDecompressed;
    }

    bool FileIO::Pack(const char *path)
    {
        mtar_t tar;
        std::string output(path);
        output.append(".bin");

        mtar_open(&tar, output.c_str(), "w");
        TarifyFile(&tar, path);

        mtar_finalize(&tar);
        mtar_close(&tar);
        return Compress(output);
    }

    bool FileIO::Unpack(const char *path)
    {
        std::string decompressPath(path);
        if (Decompress(decompressPath))
        {
            mtar_t tar;
            mtar_header_t header;
            mtar_open(&tar, decompressPath.c_str(), "r");
            while (mtar_read_header(&tar, &header) == MTAR_ESUCCESS)
            {
                char *buffer = (char *)calloc(1, header.size + 1);
                mtar_read_data(&tar, buffer, header.size);

                // Get archived file path without file name.
                std::string newFolder(header.name);
                std::string::size_type pos = newFolder.find_last_of("\\");
                if (pos != std::string::npos) newFolder.erase(pos, std::string::npos);
                pos = newFolder.find_first_of("\\");
                if (pos != std::string::npos) newFolder.erase(0, pos);

                // Rewrite path to relative form.
                char pathBuffer[128];
                GetFullPathName(".", 128, pathBuffer, NULL);
                std::string enginePath(pathBuffer);
                enginePath.append("\\..\\Engine").append(newFolder);
                CreateDirectoryRecursively(enginePath.c_str());

                if (PathFileExists(enginePath.c_str()))
                {
                    // Add updated relative path for archived file.
                    std::string updatedName(header.name);
                    std::string::size_type pos = updatedName.find_last_of("\\");
                    updatedName = updatedName.substr(pos, std::string::npos).insert(0, enginePath);

                    // Create the file to updated destination.
                    FILE *file = fopen(updatedName.c_str(), "wb");
                    if (file)
                    {
                        fwrite(buffer, 1, header.size, file);
                        fclose(file);
                    }
                }

                free(buffer);
                mtar_next(&tar);
            }
            mtar_close(&tar);
            remove(decompressPath.c_str());
            return true;
        }
        return false;
    }

    void FileIO::TarifyFile(mtar_t *tar, const char *file)
    {
        std::string wildPath(file);
        wildPath.append("\\*");
        WIN32_FIND_DATA findData;
        HANDLE hFind = FindFirstFile(wildPath.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) return;

        while (FindNextFile(hFind, &findData) != 0)
        {
            if (strcmp(findData.cFileName, "..") == 0) continue;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                std::string nextFolder(file);
                nextFolder.append("\\").append(findData.cFileName);
                TarifyFile(tar, nextFolder.c_str());
            }
            else
            {
                std::string completeFilePath(file);
                completeFilePath.append("\\").append(findData.cFileName);
                FILE *hFile = fopen(completeFilePath.c_str(), "rb");
                if (hFile == NULL) continue;

                // Calculate resource length.
                fseek(hFile, 0, SEEK_END);
                long fileLength = ftell(hFile);
                rewind(hFile);

                // Read all contents of resource into a buffer.
                char *fileContents = (char *)malloc(fileLength);
                fread(fileContents, fileLength, 1, hFile);

                // Write the buffer to the tar archive.
                mtar_write_file_header(tar, completeFilePath.c_str(), fileLength);
                mtar_write_data(tar, fileContents, fileLength);
                fclose(hFile);
                free(fileContents);
            }
        }

        FindClose(hFind);
    }

    void FileIO::CreateDirectoryRecursively(const char *path)
    {
        std::vector<std::string> folders = Util::SplitString(path, "\\");
        std::string currentPath;

        // Walk up path creating each folder as we go deeper.
        for (const auto &folder : folders)
        {
            currentPath.append(folder).append("\\");

            // Skip the root since will be a drive letter.
            if (folder != folders.front())
            {
                CreateDirectory(currentPath.c_str(), NULL);
            }
        }
    }
}
