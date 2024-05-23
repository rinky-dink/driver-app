#pragma once
#define _CRT_SECURE_NO_WARNINGS


#include <boost/process.hpp>
#include <thread>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <d3d9.h>
#include <tchar.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>
#include <zlib.h>
#include <future>
#include <string>
#include <algorithm>
#include <bitset>
#include "reg.h"
#include "imspinner.h"

#include <WS2tcpip.h>
#include <mutex>


#include <libipc/ipc.h>
#include <strsafe.h>

#include "ServiceManager.h"
#include <Shellapi.h>
#include <stdio.h>
#include <winioctl.h>

#define BLUESTREET_DEVICE 0x8000
#define IOCTL_BLUESTREET_SEND_DATA CTL_CODE(BLUESTREET_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_BLUESTREET_RECEIVE_DATA CTL_CODE(BLUESTREET_DEVICE, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_BLUESTREET_DEBUG_PRINT CTL_CODE(BLUESTREET_DEVICE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _sMSG {
    WCHAR Path[256];
    WCHAR DLLS[256];
        } sMSG;
typedef struct _sMSG* PsMSG;

#if DBG

#define DbgPrint(Format, ...) \
    printf(Format, __VA_ARGS__)
#else
#define DbgPrint(Format, ...) \
    logToFile(Format, __VA_ARGS__)
    //printf(Format, __VA_ARGS__)
#endif


static const char* AppClass = "APP CLASS";
static const char* AppName = "APP NAME";
static HWND hwnd = NULL;
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};


static ImFont* DefaultFont = nullptr;
static int WindowWidth = 1280;
static int WindowHeight = 800;


        

namespace fs = std::filesystem;
using json = nlohmann::json;

extern float progress;
extern size_t count_download;
extern std::string folderPath;
extern FILE* logFile;
extern bool ChangeDllPath;
extern std::string programm_folder;

enum type {
    DXVK,
    VKD3D,
    unknown
};

enum install_types
{
    error = -1,
    none,
    installed,
};

enum class modify_types
{
    none,
    reinstall,
    del,
    maxvalue
};

enum FlagsState {
    Flag_None = 0,
    Flag_ShowOnlyInstalled = 1 << 0,
    Flag_ModifyWindow = 1 << 1
};


modify_types operator++(modify_types& value);


struct DllPath {

    std::string dll{};
    bool include{ false };

};
extern std::vector<DllPath> sDll;

struct ReleaseInfo {
    modify_types modify { modify_types::none } ;
    type typebin;
    install_types flag { install_types::none };
    std::string version;
    std::string downloadUrl;
    bool pending { false };


    void clear()
    {
        modify = modify_types::none;
        flag = install_types::none;
        pending = false;
    }

    type gettypebinfromrepos(std::string repos)
    {
        if (repos == "doitsujin/dxvk")
            return DXVK;
        else if (repos == "HansKristian-Work/vkd3d-proton")
            return VKD3D;
        return unknown;
    }
    std::string getfoldername()
    {
        std::string bin;

        if (typebin == DXVK)
            bin = "dxvk-";
        else if (typebin == VKD3D)
            bin = "vkd3d-proton-";
        else
            return "error";

        std::regex versionRegex(".+ (.+)$");

        std::smatch match;

        if (!std::regex_search(version, match, versionRegex))
            return "error";

        return programm_folder + "installed\\" + bin + match[1].str();

    }

};

struct Cbuttonmask {
    size_t i[2]{};
};

struct ProgressData {
    double lastPercentage;
    size_t taskId;
};

extern Cbuttonmask buttonmask;

/*<---             functions.cpp             --->*/
void logToFile(const char* format, ...);

HRGN CreateRoundRectRgn(int x, int y, int width, int height, int radius);

void SetWindowRoundRect(HWND hwnd, int width, int height, int radius);

bool HasFlag(FlagsState flag, FlagsState& mask);

std::vector<std::string> getInstalledVersions(std::vector<ReleaseInfo>& ver);

std::string extractFileName(const std::string& url);

bool CompareReleases(const ReleaseInfo& release1, const ReleaseInfo& release2);

void Download(const size_t& taskId, ReleaseInfo& arr);

void DownloadInThreads(std::vector<ReleaseInfo>& ver, int numThreads);

int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t /*ultotal*/, curl_off_t /*ulnow*/);

std::vector<ReleaseInfo> GetReleases(const std::vector<std::string>& repositories, const std::string& token);

bool IsElevated();

bool gettestsigning();
bool customSort(const DllPath& a, const DllPath& b);

/*<---                                        --->*/



/*<---          imgui addition.cpp            --->*/

void ComboListVer(std::vector<ReleaseInfo>& ver, const bool& theard_status, std::string name, size_t from, size_t to, FlagsState flag = Flag_None);

void deinmodifyModal(std::vector<ReleaseInfo>& ver, const bool& theard_status);

void DarkTheme();

/*<---                                        --->*/