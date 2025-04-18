
#include <iostream>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

bool ReplaceManifestEntryWithSpaces(const char* exePath) {
    const char* targetFile = "MapleStory.exe";
    const char* backupFile = "MapleStory.exe.backup";

    if (!CopyFileA(targetFile, backupFile, FALSE)) {
        printf("Error creating backup for %s [%d]\n", targetFile, GetLastError());
        return 1;
    }

    const char* target =
        "<requestedExecutionLevel level=\"requireAdministrator\" uiAccess=\"true\"></requestedExecutionLevel>";
    size_t targetLen = strlen(target);

    HANDLE hFile = CreateFileA(
        exePath,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open file [" << GetLastError() << "]" << std::endl;
        return false;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        std::cerr << "Invalid file size." << std::endl;
        CloseHandle(hFile);
        return false;
    }

    BYTE* buffer = new BYTE[fileSize];
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
        std::cerr << "Failed to read file." << std::endl;
        delete[] buffer;
        CloseHandle(hFile);
        return false;
    }

    bool found = false;
    for (DWORD i = 0; i <= fileSize - targetLen; ++i) {
        if (memcmp(buffer + i, target, targetLen) == 0) {
            SetFilePointer(hFile, i, NULL, FILE_BEGIN);
            std::string spaces(targetLen, ' ');
            DWORD bytesWritten = 0;
            if (!WriteFile(hFile, spaces.c_str(), targetLen, &bytesWritten, NULL)) {
                std::cerr << "Failed to write to file." << std::endl;
                delete[] buffer;
                CloseHandle(hFile);
                return false;
            }
            std::cout << "Manifest entry removed at offset 0x" << std::hex << i << std::endl;
            found = true;
            break;
        }
    }

    if (!found) {
        std::cerr << "Target manifest entry not found." << std::endl;
    }

    delete[] buffer;
    CloseHandle(hFile);
    return found;
}


int main()
{
    const char* exePath = "MapleStory.exe"; // 当前目录的目标文件
    if (!ReplaceManifestEntryWithSpaces(exePath)) {
        std::cerr << "Failed to patch executable." << std::endl;
        return 1;
    }
    std::cout << "Patch complete." << std::endl;
    std::cin.get();
}
