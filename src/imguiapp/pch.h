// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H
#define _CRT_SECURE_NO_WARNINGS
// add headers that you want to pre-compile here

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
#include <comdef.h>
#include <taskschd.h>
#include "TaskSchedulerLib.h"

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")

#if DBG
#define DbgPrint(Format, ...) printf(Format, __VA_ARGS__)
#else
#define DbgPrint(Format, ...) void logToFile(const char* format, ...); logToFile(Format, __VA_ARGS__)
#endif



#define BLUESTREET_DEVICE 0x8000
#define IOCTL_BLUESTREET_SEND_DATA CTL_CODE(BLUESTREET_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_BLUESTREET_RECEIVE_DATA CTL_CODE(BLUESTREET_DEVICE, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_BLUESTREET_DEBUG_PRINT CTL_CODE(BLUESTREET_DEVICE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _sMSG {
	WCHAR Path[256];
	WCHAR DLLS[256];
} sMSG;
typedef struct _sMSG* PsMSG;

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
extern bool ver_manager;
extern size_t count_download;

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
class HUD {
public:
	float size_hud = 1.f;
	float opacity_hud = 1.f;

	struct ParamInfo {
		std::string name;
		bool active;
	};

	HUD() {
		InitParams();
	}

	void AddParam(const std::string& paramName) {
		params[paramName] = ParamInfo{ paramName, false };
	}

	// Устанавливает активность параметра
	void SetParamActive(const std::string& paramName, bool isActive) {
		if (params.find(paramName) != params.end()) {
			params[paramName].active = isActive;
		}
	}

	// Получает статус активности параметра
	bool IsParamActive(const std::string& paramName) const {
		auto it = params.find(paramName);
		return it != params.end() ? it->second.active : false;
	}

	// Implementing begin() and end() methods for range-based for loop
	auto begin() { return params.begin(); }
	auto end() { return params.end(); }
	auto begin() const { return params.cbegin(); }
	auto end() const { return params.cend(); }

	std::size_t Count() const {
		return params.size();
	}
	std::map<std::string, ParamInfo> params;

	void update(const std::string& str) {
		std::istringstream iss(str);
		std::string token;
		while (std::getline(iss, token, ',')) {
			if (token.find("scale=") != std::string::npos) {
				std::string scale_str = token.substr(6); // Получаем подстроку после "scale="
				std::replace(scale_str.begin(), scale_str.end(), '.', ','); // Заменяем точки на запятые
				try {
					size_hud = std::stof(scale_str);
				}
				catch (...) {
					DbgPrint("Failed to parse scale value from token: %s\n", scale_str.c_str());
				}
			}
			else if (token.find("opacity=") != std::string::npos) {
				std::string opacity_str = token.substr(8); // Получаем подстроку после "opacity="
				std::replace(opacity_str.begin(), opacity_str.end(), '.', ','); // Заменяем точки на запятые
				try {
					opacity_hud = std::stof(opacity_str);
				}
				catch (...) {
					DbgPrint("Failed to parse opacity value from token: %s\n", opacity_str.c_str());

				}
			}
			else {

				SetParamActive(token, true);
			}
		}
	}

	std::string get() {
		std::string hud_params{};
		for (auto path : params) {
			if (path.second.active) {
				hud_params += path.second.name + ",";
			}
		}

		std::ostringstream ss;
		ss << std::fixed << std::setprecision(2) << size_hud;
		std::string size_hud_str = ss.str();

		// Заменяем запятую на точку
		for (auto& c : size_hud_str) {
			if (c == ',') {
				c = '.';
			}
		}

		if (hud_params.empty())
			return "0";

		// Добавляем строку к hud_params
		hud_params += "scale=" + size_hud_str + ",";

		// Преобразуем число в строку с двумя знаками после точки
		ss.str(""); // Очищаем stringstream
		ss << std::fixed << std::setprecision(2) << opacity_hud;
		std::string opacity_hud_str = ss.str();

		// Заменяем запятую на точку
		for (auto& c : opacity_hud_str) {
			if (c == ',') {
				c = '.';
			}
		}

		// Добавляем строку к hud_params
		hud_params += "opacity=" + opacity_hud_str;

		return hud_params;
	}

private:


	void InitParams() {
		static const std::vector<std::string> paramNames = {
			"full", "fps", "frametimes", "submissions", "drawcalls", "pipelines",
			"descriptors", "memory", "gpuload", "version", "api", "cs", "compiler", "samplers"
		};

		for (const auto& name : paramNames) {
			AddParam(name);
		}
	}
};



struct DllPath {

	std::string dll{};
	bool include{ false };

};


bool customSort(const DllPath& a, const DllPath& b);

struct ReleaseInfo {
	modify_types modify{ modify_types::none };
	type typebin;
	install_types flag{ install_types::none };
	std::string version;
	std::string downloadUrl;
	bool pending{ false };


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
	std::string getversion()
	{
		std::regex versionRegex(".+ (.+)$");
		std::smatch match;
		if (!std::regex_search(version, match, versionRegex))
			return "";

		return match[1].str();
	}
};

struct Cbuttonmask {
	size_t i[2]{};
};

struct ProgressData {
	double lastPercentage;
	size_t taskId;
};
struct dataGame
{
	std::string path{};
	//std::string ddls{};
	std::string dlls_FolderPath{};
	HUD hud{};
	std::vector<DllPath> dll;
	std::vector<ReleaseInfo> ReleaseInfo;

	bool menu{ false };
	dataGame() {};

	void parseVersion() {

		std::regex versionRegex("(.+)-(.+)");
		std::smatch match;

		std::string path = fs::path(dlls_FolderPath).filename().string();
		if (std::regex_search(path, match, versionRegex)) {
			std::string name = match[1].str().c_str();
			std::string version = match[2].str();
			DbgPrint("%s | %s\n", name, version);
			DbgPrint("dxvk %s", (name == "dxvk") ? "true" : "false");

			for (auto& i : ReleaseInfo)
			{
				if ((i.getversion() == version) && (i.typebin == ((name == "dxvk") ? DXVK : unknown)))
				{


					for (const auto& entry : fs::directory_iterator(dlls_FolderPath + "\\x64")) {
						if (entry.is_regular_file()) {
							std::string filename = entry.path().filename().string();
							auto it = std::find(included_dlls.begin(), included_dlls.end(), filename);

							dll.push_back(DllPath(filename, it != included_dlls.end()));


							//dll.push_back(DllPath(entry.path().filename().string(), false));
						}
					}

					std::sort(dll.begin(), dll.end(), customSort);
					i.pending = true;

				}
			}
		}
	}

	void update()
	{
		if (!path.empty()) {
			std::string dxvk_conf = fs::path(path).parent_path().string() + "\\dxvk.conf";
			std::ifstream inFile(dxvk_conf);

			std::string line;

			while (std::getline(inFile, line)) {
				if (line.find("dxvk.dlls.FolderPath") != std::string::npos) {
					size_t pos = line.find("\"");
					if (pos != std::string::npos) {
						dlls_FolderPath = line.substr(pos + 1, line.find("\"", pos + 1) - pos - 1);

					}
				}
				else if (line.find("dxvk.dlls") != std::string::npos) {
					size_t pos = line.find("= ");
					if (pos != std::string::npos) {
						std::istringstream iss(line.substr(pos + 2));
						std::string dll;
						while (std::getline(iss, dll, ',')) {
							included_dlls.push_back(dll);
						}

					}
				}
				else if (line.find("dxvk.hud") != std::string::npos) {
					size_t pos = line.find("= ");
					if (pos != std::string::npos) {
						hud.update(line.substr(pos + 2));
					}
				}
			}
			DbgPrint("dlls_FolderPath = %s\n", dlls_FolderPath.c_str());
			//DbgPrint("dlls = %s\n", ddls.c_str());
			DbgPrint("hud.get() = %s\n", hud.get().c_str());
		}
	}
private:
	std::vector<std::string> included_dlls{};
};
extern dataGame dGame;

extern Cbuttonmask buttonmask;

/*<---             functions.cpp             --->*/


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

void settestsinging(bool status);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);


/*<---                                        --->*/



/*<---          imgui addition.cpp            --->*/

void ComboListVer(std::vector<ReleaseInfo>& ver, const bool& theard_status, std::string name, size_t from, size_t to, FlagsState flag = Flag_None);

void deinmodifyModal(std::vector<ReleaseInfo>& ver, const bool& theard_status);

void DarkTheme();

/*<---                                        --->*/
#endif //PCH_H
