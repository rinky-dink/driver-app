#include "pch.h"

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//RECT windowRect;

inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
{
	return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

static bool isButtonPressed = false;

Cbuttonmask buttonmask;

void DrawCloseButton(ImDrawList* draw_list, const ImVec2& center, float radius, ImU32 color, bool isPressed)
{
	// Отрисовка круглой кнопки
	draw_list->AddCircleFilled(center, radius, color);

	// Определение размера и толщины крестика
	float cross_size = radius * 0.7f;
	float cross_thickness = radius * 0.1f;

	// Определение позиции для рисования крестика
	ImVec2 cross_pos = ImVec2(center.x - cross_size * 0.5f, center.y - cross_size * 0.5f);

	// Если кнопка нажата, измените цвет крестика
	ImU32 cross_color = isPressed ? IM_COL32(200, 200, 200, 255) : IM_COL32_WHITE;

	// Отрисовка крестика
	draw_list->AddLine(ImVec2(cross_pos.x, cross_pos.y), ImVec2(cross_pos.x + cross_size, cross_pos.y + cross_size), cross_color, cross_thickness);
	draw_list->AddLine(ImVec2(cross_pos.x + cross_size, cross_pos.y), ImVec2(cross_pos.x, cross_pos.y + cross_size), cross_color, cross_thickness);
}


dataGame dGame;

NOTIFYICONDATA notifyIconData;
HMENU hPopupMenu;

#define ID_TRAY_APP_ICON 5000
#define ID_TRAY_EXIT 3000
#define ID_TRAY_OPEN 3001
#define ID_TRAY_DISABLE_TEST_MODE 3002
#define WM_SYSICON (WM_USER + 1)
#define IDI_ICON1                       101
void InitNotifyIconData(HWND hWnd) {
	memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));
	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = hWnd;
	notifyIconData.uID = ID_TRAY_APP_ICON;
	notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIconData.uCallbackMessage = WM_SYSICON;
	notifyIconData.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1)); // Используем вашу иконку
	strcpy_s(notifyIconData.szTip, "Tray Application");
}


void InitTrayMenu() {
	hPopupMenu = CreatePopupMenu();
	AppendMenuW(hPopupMenu, MF_STRING, ID_TRAY_OPEN, L"Открыть");
	AppendMenuW(hPopupMenu, MF_STRING, ID_TRAY_DISABLE_TEST_MODE, L"Отключить тестовый режим");
	AppendMenuW(hPopupMenu, MF_STRING, ID_TRAY_EXIT, L"Выйти");
}


FILE* logFile;
bool ChangeDllPath{ false };

std::string programm_folder;

bool ver_manager = true;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	std::setlocale(LC_NUMERIC, "C");
#pragma region Command Line Processing Arguments
	LPWSTR lpWideCmdLine = GetCommandLineW();
	int argc = 0;
	LPWSTR* wideArgv = CommandLineToArgvW(lpWideCmdLine, &argc);

	std::vector<char*> argv(argc);

	std::vector<std::string> narrowStrings(argc);
	std::locale::global(std::locale(""));
	for (int i = 0; i < argc; ++i)
	{
		int len = WideCharToMultiByte(CP_ACP, 0, wideArgv[i], -1, NULL, 0, NULL, NULL);
		narrowStrings[i].resize(len);
		WideCharToMultiByte(CP_ACP, 0, wideArgv[i], -1, &narrowStrings[i][0], len, NULL, NULL);
		argv[i] = &narrowStrings[i][0];
	}
#pragma endregion

#pragma region Initialization_and_IPC_Processing
	bool updategamewindowpos = true;
	bool main = false;

	bool done = false;

	std::mutex mtx;
	HANDLE semaphore = CreateSemaphore(NULL, 0, 1, "sImguiAPP");

	constexpr char const name__[] = "ipc-chat";
		
	bool server = false;

	if (semaphore == NULL) {
		std::cerr << "Failed to create or open semaphore\n";
		return 1;
	}

	else if (GetLastError() == ERROR_ALREADY_EXISTS) {
		if (argc >= 2)
		{
			if (!server)
			{
				ipc::channel sender__{ name__, ipc::sender };
				std::cout << " Client" << std::endl;

				std::string args;
				for (int i = 1; i < argc; i++) {
					args += argv[i];
					args += "|";
				}

				sender__.send(args.c_str());
				return 0;
			}
		}
		else
			return 0;
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp("server", argv[i]) == 0)
			server = true;
	}
	
	if (!IsElevated())
	{
		std::string arg = server ? "" : "server ";
		for (int i = 1; i < argc; i++) {
			arg += argv[i];
			if (i < argc - 1) {
				arg += " ";
			}
		}
		char fullPath[MAX_PATH];
		if (GetFullPathName(argv[0], MAX_PATH, fullPath, NULL) == 0) {
			DWORD error = GetLastError();
			std::cerr << "Failed to get the full path. Error code: " << error << std::endl;
			return 0;
		}


		HINSTANCE result = ShellExecute(NULL, "runas", argv[0], arg.c_str(), fs::path(argv[0]).parent_path().string().c_str(), SW_SHOWNORMAL);
		if ((int)result <= 32) {
			DWORD error = GetLastError();
			std::cerr << "Failed to execute the program with elevated privileges. Error code: " << error << std::endl;
			return 0;
		}


		return 0;
	}

	bool testsigning = gettestsigning();
	std::thread r;
	std::thread future;
	bool async_theard{ false };
	size_t indexVKD3D;

	if (testsigning) {
		for (int i = 1; i < argc; i++) {
			if (strcmp("game", argv[i]) == 0)
			{
				if (i + 1 < argc)
					dGame.path = argv[i + 1];
				dGame.menu = true;

			}
			else if (strcmp("main", argv[i]) == 0)
			{
				main = true;
			}
		}

		for (size_t i = 0; i < argc; i++)
		{
			std::cout << argv[i] << "\t";
			if (argv[i] == "game")
				dGame.menu = true;

		}

		r = std::thread([&] {
			ipc::channel receiver__{ name__, ipc::receiver };
			while (!done) {
				ipc::buff_t buf = receiver__.recv();
				if (buf.empty()) break;

				std::string dat{ buf.get<char const*>(), buf.size() - 1 };

				std::cout << dat << std::endl;

				std::vector<std::string> substrings;

				std::stringstream ss(dat);

				std::string substring;
				while (std::getline(ss, substring, '|')) {
					substrings.push_back(substring);
				}

				for (size_t i = 0; i < substrings.size(); i++)
				{
					if (substrings[i] == "game")
					{
						for (auto& i : dGame.ReleaseInfo)
						{
							i.pending = false;
						}
						dGame.dll.clear();

						for (auto& i : dGame.hud)
						{
							i.second.active = false;
						}

						dGame.hud.opacity_hud = 1.f;
						dGame.hud.size_hud = 1.f;
						dGame.dlls_FolderPath = "";
						SetCurrentDirectory("C:\\Users\\Roman\\Downloads\\Жуки S03 2022 WEB-DLRip");
						DbgPrint("programm_folder %s", programm_folder.c_str());
						DbgPrint("fs::path(argv[0]).parent_path().c_str() %s", fs::path(argv[0]).parent_path());


						if (i + 1 < substrings.size())
						{
							size_t pos = substrings[i + 1].find_last_of('.');

							if (pos != std::string::npos)
								if (substrings[i + 1].substr(pos) == ".exe" || substrings[i + 1].substr(pos) == ".exe\"")
								{
									dGame.path = substrings[i + 1];
									dGame.path.erase(std::remove(dGame.path.begin(), dGame.path.end(), '\"'), dGame.path.end());
								}
								else
									continue;
							else
								continue;

							DbgPrint("gamepath:: %s", dGame.path.c_str());
							dGame.update();
							if (async_theard)
								dGame.parseVersion();

						}
						updategamewindowpos = true;
						dGame.menu = true;
						std::cout << "game = true;" << std::endl;

					}
					else if (substrings[i] == "main")
					{
						main = true;
						std::cout << "main = true;" << std::endl;
					}

				}
			}
			std::lock_guard<std::mutex> lock(mtx);
			receiver__.disconnect();
			}
		);
		dGame.update();

		std::string token = "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";

		future = std::thread([&] {
			dGame.ReleaseInfo = GetReleases({ "doitsujin/dxvk", "HansKristian-Work/vkd3d-proton" }, token);

			for (auto c : dGame.ReleaseInfo)
			{
				DbgPrint("%s!\n", c.version.c_str());
			}
			auto it = std::find_if(dGame.ReleaseInfo.begin(), dGame.ReleaseInfo.end(), [](const ReleaseInfo& release) {
				return release.typebin == VKD3D;
				});

			if (it != dGame.ReleaseInfo.end())
				indexVKD3D = std::distance(dGame.ReleaseInfo.begin(), it);

			getInstalledVersions(dGame.ReleaseInfo);
			dGame.parseVersion();
			async_theard = true;
			});

		programm_folder = argv[0];
		size_t pos = programm_folder.find("imguiapp.exe");
		if (pos != std::string::npos) {
			programm_folder.erase(pos, std::string::npos);
		}

		ServiceManager driver{ programm_folder };
		{
			if (!driver.Install())
			{
				//MessageBoxA(NULL, "Ошибка установки драйвера", "ERROR", MB_ICONERROR);
				//			return 1;
			}
		}
	}

	



	
#pragma endregion

#pragma region InitializationImgui
	RECT desktop;
	GetWindowRect(GetDesktopWindow(), &desktop);
	WNDCLASSEX wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, AppClass, nullptr };
	RegisterClassEx(&wc);
	hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, AppClass, AppName, WS_POPUP | WS_OVERLAPPED, 0/*(desktop.right / 2) - (WindowWidth / 2)*/, 0, 200, WindowHeight, 0, 0, wc.hInstance, 0);

	InitNotifyIconData(hwnd);
	InitTrayMenu();
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, ULW_COLORKEY);

	if (CreateDeviceD3D(hwnd) < 0)
	{
		CleanupDeviceD3D();
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	::ShowWindow(hwnd, SW_HIDE);
	::UpdateWindow(hwnd);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::GetStyle().AntiAliasedLines = true;

	DarkTheme();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	ImFontConfig config;
	config.MergeMode = false;
	config.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 18.0f, &config);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	DWORD dwFlag = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse; //| ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoScrollWithMouse
#pragma endregion

	while (!done)
	{
#pragma region Message Handler
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				done = true;
				//MessageBoxA(NULL, "Ошибка установки драйвера", "ERROR", MB_ICONERROR);
			}
			if (msg.message == WM_ENDSESSION)

			{
				//MessageBoxA(NULL, "Ошибка установки драйвера", "ERROR", MB_ICONERROR);
				done = true;
			}
			if (msg.message == WM_CLOSE)
			{
				done = true;
				//MessageBoxA(NULL, "Ошибка установки драйвера", "ERROR", MB_ICONERROR);
			}
		}

		


		if (done)
			break;
#pragma endregion

		//// Handle window resize
		//if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
		//{
		//    g_d3dpp.BackBufferWidth = g_ResizeWidth;
		//    g_d3dpp.BackBufferHeight = g_ResizeHeight;
		//    g_ResizeWidth = g_ResizeHeight = 0;
		//    ResetDevice();
		//}


		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (true) // testsinging
		{
			static std::wstring taskName = L"DXVK UI Startup";
			static std::string titlePopup = "Перезагрузка";
			static bool popup{ false };
			static bool autostart{ DoesScheduledTaskExist(taskName)};
			static ImVec2 winSize(658, 226);

			static ImVec2 CenteredMain{ ((float)screenWidth - winSize.x) / 2.f ,  ((float)screenHeight - winSize.y) / 2.f };
			static auto displayStateText = [](const std::string& prefix, bool state) {
				std::string stateText = state ? "Включено!" : "Выключено!";
				ImVec4 textColor = state ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

				ImVec2 windowSize = ImGui::GetWindowSize();
				float prefixWidth = ImGui::CalcTextSize(prefix.c_str()).x;
				float stateWidth = ImGui::CalcTextSize(stateText.c_str()).x;
				float totalWidth = prefixWidth + stateWidth;

				ImGui::SetCursorPosX((windowSize.x - totalWidth) * 0.5f);

				ImGui::Text("%s", prefix.c_str());
				ImGui::SameLine();

				ImGui::TextColored(textColor, "%s", stateText.c_str());
			};
			
			
			ImGui::SetNextWindowPos(CenteredMain, ImGuiCond_Once);
			ImGui::SetNextWindowSize(winSize, ImGuiCond_Once);
			ImGui::Begin("Установка драйвера", nullptr, dwFlag);

			


			ImGui::BeginChild("ChildWindow", ImVec2(0, 0), true);

			ImGui::SeparatorText("Для установки драйвера, нужно включить тестовый режим и перезагрузить компьютер!\nТестовый режим сможете выключить -> ПКМ по иконке в трее -> Выключить тестовый режим");

			displayStateText("Состояние автозапуска: ", autostart);
			displayStateText("Тестовый режим: ", testsigning);

			ImGui::Separator();

			ImGui::Spacing();
			ImGui::Spacing();

			


			ImGui::SetCursorPosX((winSize.x - 320) / 2 );
			if (ImGui::Button(
				std::format(
					"{} автозапуск",
					autostart ? "Выключить" : "Включить"
				).c_str(), 
				ImVec2(310, 30)))
			{
				int size_needed = MultiByteToWideChar(CP_ACP, 0, argv[0], -1, NULL, 0);
				std::wstring executablePath(size_needed, 0);
				MultiByteToWideChar(CP_ACP, 0, argv[0], -1, &executablePath[0], size_needed);

				if (!executablePath.empty() && executablePath.back() == L'\0') {
					executablePath.pop_back();
				}

				if (!autostart)
				{
					if (CreateScheduledTask(taskName, executablePath, L"-silent")) {
						DbgPrint("Application startup added to Task Scheduler successfully.\n");
						autostart = true;
					}
					else {
						DbgPrint("Failed to add application to Task Scheduler.\n");
					}
				}
				else
				{
					if (DeleteScheduledTask(taskName)) {
						DbgPrint("Task deleted successfully.\n");
						autostart = false;
					}
					else {
						DbgPrint("Failed to delete the task.\n");
					}
				}
			}

		



			ImGui::Spacing();

			ImGui::SetCursorPosX((winSize.x - 440) / 2);

			// Кнопка для включения тестового режима
			if (ImGui::Button("Включить тестовый режим", ImVec2(200, 30)))
			{
				settestsinging(!testsigning);
				testsigning = gettestsigning();
				popup = true;
			}

			ImGui::SameLine();

			ImGui::Dummy(ImVec2(20.0f, 0.0f));

			ImGui::SameLine();

			// Кнопка "Выйти"
			if (ImGui::Button("Выход", ImVec2(200, 30)))
			{
				done = true;
			}

			// Конец дочернего окна
			ImGui::EndChild();

			if (popup)
			{
				ImGui::OpenPopup(titlePopup.c_str());
				popup = false;
			}

			ImGui::SetNextWindowSize(ImVec2(270, 135), ImGuiCond_Always);
			if (ImGui::BeginPopupModal(titlePopup.c_str(), NULL, dwFlag | ImGuiWindowFlags_NoMove))
			{

				ImGui::BeginChild("child", ImVec2(260, 100), true, ImGuiWindowFlags_AlwaysAutoResize);
				ImGui::TextWrapped("Чтобы изменения вступили в силу, необходимо перезагрузить компьютер. Выполнить перезагрузку?");
				if (ImGui::Button("Да", ImVec2(100, 30)))
					ImGui::CloseCurrentPopup();

				ImGui::SameLine();

				ImGui::Dummy(ImVec2(40.0f, 0.0f));

				ImGui::SameLine();
				if (ImGui::Button("Позже", ImVec2(100, 30)))
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
				ImGui::EndChild();
			}

			ImGui::End();
		}
		if (false) {
			static bool isFirstFrame = true;
			static float prevProgress = 0.0f;
			static float lastUpdateTime = 0.0f;
			static std::string modifybuttontext;
			static ImVec2 winSize(658, 226);
			
			static ImVec2 CenteredMain{ ((float)screenWidth - winSize.x) / 2.f ,  ((float)screenHeight - winSize.y) / 2.f };
			ImGui::SetNextWindowSize(ImVec2(WindowWidth, WindowHeight), ImGuiCond_Once);
			ImGui::Begin("fsad", nullptr, dwFlag);

			deinmodifyModal(dGame.ReleaseInfo, async_theard);

			ImGui::SetCursorPosX(190);
			ImGui::Text("DXVK");

			ComboListVer(dGame.ReleaseInfo, async_theard, "DXVK", 0, indexVKD3D);

			if (progress > 0)
			{
				float currentTime = ImGui::GetTime();
				float deltaTime = currentTime - lastUpdateTime;
				lastUpdateTime = currentTime;

				float smoothProgress;

				if (isFirstFrame) {

					smoothProgress = progress;
					isFirstFrame = false;
				}
				else {
					smoothProgress = prevProgress + (progress - prevProgress) * deltaTime * 5.0f;
					smoothProgress = std::max(smoothProgress, 0.0f);
				}

				prevProgress = smoothProgress;


				ImU32 progressBarColor = IM_COL32(255, 165, 0, 255);  // Стандартный цвет (оранжевый)
				ImU32 fullProgressBarColor = IM_COL32(50, 205, 50, 255);  // Салатовый цвет для полностью загруженной полоски

				if (progress == 1)
					ImGui::PushStyleColor(ImGuiCol_PlotHistogram, fullProgressBarColor);



				ImGui::ProgressBar(smoothProgress, ImVec2(-1.0f, 0.0f), progress == 1 ? "Complited!" : "Loading");

				if (progress == 1)
					ImGui::PopStyleColor();
			}


			buttonmask.i[1] = std::count_if(dGame.ReleaseInfo.begin(), dGame.ReleaseInfo.end(), [](const auto& elem) {
				return elem.flag == install_types::installed && elem.pending;
				});
			buttonmask.i[0] = std::count_if(dGame.ReleaseInfo.begin(), dGame.ReleaseInfo.end(), [](const auto& elem) {
				return elem.flag != install_types::installed && elem.pending;
				});


			if (buttonmask.i[0] == 0 && buttonmask.i[1] == 0)
				modifybuttontext = "Выберите";
			else
			{
				if (buttonmask.i[0] > 0)
				{
					modifybuttontext = "Скачать и установить";
					if (buttonmask.i[1] > 0)
					{
						if (!modifybuttontext.empty())
							modifybuttontext += " / ";
						modifybuttontext += "Переустановить или удалить";
					}

				}
				else if (buttonmask.i[1] > 0 && buttonmask.i[0] == 0)
					modifybuttontext = "Переустановить или удалить";
			}

			if (ImGui::Button(modifybuttontext.c_str(), ImVec2(500, 100)))
			{
				if (buttonmask.i[0] > 0)
				{
					std::thread downloadThread(DownloadInThreads, std::ref(dGame.ReleaseInfo), 5);
					downloadThread.detach();
				}

				if (buttonmask.i[1] > 0)
					ImGui::OpenPopup("1233");

			}


			if (ImGui::Button("close", ImVec2(100, 20)))
				done = true;
			ImGui::Text("%s", (dGame.menu ? "true" : "false"));

			ImGui::End();
		
		}

#pragma region unuse
		//if(false){
		//	static ImVec2 winSize(658, 226);
		//	static ImVec2 CenteredMain{ ((float)screenWidth - winSize.x) / 2.f ,  ((float)screenHeight - winSize.y) / 2.f };
		//	ImGui::SetNextWindowPos(CenteredMain, ImGuiCond_Once);
		//	ImGui::SetNextWindowSize(ImVec2(WindowWidth, WindowHeight), ImGuiCond_Once);
		//	ImGui::Begin("Dear ImGui Standalone Window", nullptr, dwFlag);

		//	deinmodifyModal(dGame.ReleaseInfo, async_theard);

		//	ImGui::SetCursorPosX(190);
		//	ImGui::Text("DXVK");
		//	ImGui::SameLine();
		//	ImGui::SetCursorPosX(615);
		//	ImGui::Text("VKD3D");
		//	ImGui::Text(async_theard ? "VKD3D" : "4314");

		//	
		//	ComboListVer(dGame.ReleaseInfo, async_theard, "DXVK", 0, indexVKD3D);
		//	ImGui::SameLine();
		//	ComboListVer(dGame.ReleaseInfo, async_theard, "VKD3D", indexVKD3D, dGame.ReleaseInfo.size());


		//	if (progress > 0)
		//	{
		//		float currentTime = ImGui::GetTime();
		//		float deltaTime = currentTime - lastUpdateTime;
		//		lastUpdateTime = currentTime;

		//		float smoothProgress;

		//		if (isFirstFrame) {

		//			smoothProgress = progress;
		//			isFirstFrame = false;
		//		}
		//		else {
		//			smoothProgress = prevProgress + (progress - prevProgress) * deltaTime * 5.0f;
		//			smoothProgress = std::max(smoothProgress, 0.0f);
		//		}

		//		prevProgress = smoothProgress;


		//		ImU32 progressBarColor = IM_COL32(255, 165, 0, 255);  // Стандартный цвет (оранжевый)
		//		ImU32 fullProgressBarColor = IM_COL32(50, 205, 50, 255);  // Салатовый цвет для полностью загруженной полоски

		//		if (progress == 1)
		//			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, fullProgressBarColor);



		//		ImGui::ProgressBar(smoothProgress, ImVec2(-1.0f, 0.0f), progress == 1 ? "Complited!" : "Loading");

		//		if (progress == 1)
		//			ImGui::PopStyleColor();
		//	}


		//	buttonmask.i[1] = std::count_if(dGame.ReleaseInfo.begin(), dGame.ReleaseInfo.end(), [](const auto& elem) {
		//		return elem.flag == install_types::installed && elem.pending;
		//		});
		//	buttonmask.i[0] = std::count_if(dGame.ReleaseInfo.begin(), dGame.ReleaseInfo.end(), [](const auto& elem) {
		//		return elem.flag != install_types::installed && elem.pending;
		//		});


		//	if (buttonmask.i[0] == 0 && buttonmask.i[1] == 0)
		//		modifybuttontext = "Выберите";
		//	else
		//	{
		//		if (buttonmask.i[0] > 0)
		//		{
		//			modifybuttontext = "Скачать и установить";
		//			if (buttonmask.i[1] > 0)
		//			{
		//				if (!modifybuttontext.empty())
		//					modifybuttontext += " / ";
		//				modifybuttontext += "Переустановить или удалить";
		//			}

		//		}
		//		else if (buttonmask.i[1] > 0 && buttonmask.i[0] == 0)
		//			modifybuttontext = "Переустановить или удалить";
		//	}

		//	if (ImGui::Button(modifybuttontext.c_str(), ImVec2(500, 100)))
		//	{
		//		if (buttonmask.i[0] > 0)
		//		{
		//			std::thread downloadThread(DownloadInThreads, std::ref(dGame.ReleaseInfo), 5);
		//			downloadThread.detach();
		//		}

		//		if (buttonmask.i[1] > 0)
		//			ImGui::OpenPopup("1233");

		//	}

		//	if (ImGui::Button("Открыть", ImVec2(100, 20)))
		//		dGame.menu = !dGame.menu;
		//	
		//	
		//	//if (ImGui::Button("Открытьddd", ImVec2(100, 20)))
		//	//	AddLogToConsole("Hello, world!");


		//	if (ImGui::Button("close", ImVec2(100, 20)))
		//		done = true;
		//	ImGui::Text("%s", (dGame.menu ? "true" : "false"));
		//	//for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); ++i) {
		//	//    ImGui::Checkbox(("##Checkbox" + std::to_string(i)).c_str(), &data[i]);
		//	//    ImGui::SameLine();
		//	//    ImGui::Text("%s", (data[i] ? "true" : "false"));
		//	//}

		//	// Вычисление и вывод сходства (affinity)
		//	//std::string affinity = CalculateAffinity(data, sizeof(data) / sizeof(data[0]));
		//	//ImGui::Text("affinity: 0x%s", affinity.c_str());

		//	//ConvertToBoolArray(affinity, data2, sizeof(data2) / sizeof(data2[0]));


		//	//ImGui::Text("out:");
		//	//for (size_t i = 0; i < sizeof(data2) / sizeof(data2[0]); ++i) {
		//	//	ImGui::Checkbox(("##checkbox2" + std::to_string(i)).c_str(), &data2[i]);
		//	//	ImGui::SameLine();
		//	//	ImGui::Text("%s", (data2[i] ? "true" : "false"));
		//	//}


		//	




		//	ImGui::End();
		//}
#pragma endregion

		//ImGui::ShowDemoWindow();
		if(dGame.menu){

			if (updategamewindowpos)
			{
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				ImGui::SetNextWindowPos(ImVec2(cursorPos.x, cursorPos.y));
				updategamewindowpos = false;
			}
			ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_Once);
			ImGui::Begin("2Window", nullptr, dwFlag | ImGuiWindowFlags_NoTitleBar);
			{

				if (ImGui::BeginPopupModal("settings_game", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiPopupFlags_NoOpenOverExistingPopup))
				{
					if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
					{
						ImGui::CloseCurrentPopup();
					}

					size_t j = 0;
					ImGui::BeginChild("child_id", ImVec2(515, 335), true, ImGuiWindowFlags_AlwaysAutoResize);
					ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("HUD:").x) * 0.5f);
					ImGui::Text("HUD:");


					const std::string& full_name = dGame.hud.params["full"].name;
					bool full_active = dGame.hud.params["full"].active;

					if (!full_active)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 0.2f));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
					}
					ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 80) * 0.5f);
					if (ImGui::Button(full_name.c_str(), ImVec2(80, 50)))
					{
						dGame.hud.SetParamActive(full_name, !full_active);
						for (auto i : dGame.hud)
						{
							if (i.second.name == full_name)
								continue;
							dGame.hud.SetParamActive(i.second.name, !full_active);
						}
					}

					ImGui::PopStyleColor(2);
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
					for (auto i : dGame.hud)
					{
						if (i.second.name == full_name)
							continue;

						if (!i.second.active)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 0.2f));
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
						}
						if (ImGui::Button(i.second.name.c_str(), ImVec2(80, 50)))
						{
							dGame.hud.SetParamActive(i.second.name, !i.second.active);
							if(full_active)
								dGame.hud.SetParamActive(full_name, !full_active);

						}

						ImGui::PopStyleColor(2);

						if (j++ == 0 || ( j % 6 != 0 && j != dGame.hud.Count() ) )
							ImGui::SameLine();
					}
					ImGui::Spacing();
					ImGui::Separator();

					ImGui::SliderFloat("Размер (0 -> 1)", &dGame.hud.size_hud, 0.0f, 5.0f, "%.2f");
					{

					}
					ImGui::SliderFloat("Прозрачность (0 -> 1)", &dGame.hud.opacity_hud, 0.0f, 1.0f, "%.2f");
					{

					}
					ImGui::EndChild();
					ImGui::Button("HUD", ImVec2(50, 50));

					/*if (!sDll[i].include)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 0.2f));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
					}

					if (ImGui::Button(sDll[i].dll.c_str(), ImVec2(100, 60)))
						sDll[i].include = !sDll[i].include;
					if (i < sDll.size() - 1)
						ImGui::SameLine();*/


					

					ImGui::EndPopup();
				}
				

			}
			ComboListVer(dGame.ReleaseInfo, async_theard, "DXVK1", 0, indexVKD3D, Flag_ShowOnlyInstalled);
			ImVec2 button_pos(ImGui::GetCursorScreenPos());
			float button_radius = 50.0f;

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			DrawCloseButton(draw_list, button_pos + ImVec2(button_radius, button_radius), button_radius, IM_COL32(255, 0, 0, 255), isButtonPressed);

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsMouseHoveringRect(button_pos, button_pos + ImVec2(button_radius * 2, button_radius * 2)))
			{
				isButtonPressed = !isButtonPressed; // Инвертируем состояние нажатия кнопки
			}

			if (!dGame.dlls_FolderPath.empty()) {
				if (ChangeDllPath)
				{
					dGame.dll.clear();
					for (const auto& entry : fs::directory_iterator(dGame.dlls_FolderPath + "/x64")) {
						if (entry.is_regular_file()) {
							dGame.dll.push_back(DllPath(entry.path().filename().string(), false));
						}
					}
					std::sort(dGame.dll.begin(), dGame.dll.end(), customSort);
					ChangeDllPath = false;
				}

				for (int i = 0; i < dGame.dll.size(); i++)
				{
					ImGui::PushID(i);

					if (!dGame.dll[i].include)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 0.2f));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
					}

					if (ImGui::Button(dGame.dll[i].dll.c_str(), ImVec2(100, 60)))
						dGame.dll[i].include = !dGame.dll[i].include;
					if (i < dGame.dll.size() - 1)
						ImGui::SameLine();

					ImGui::PopStyleColor(2);


					ImVec2 m = ImGui::GetIO().MousePos;
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(.0f, .0f)); // Устанавливаем прозрачный цвет фона
					ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Устанавливаем прозрачный цвет фона
					ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, .0f); // Устанавливаем прозрачный цвет обводки
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoPreviewTooltip)) //ImGuiDragDropFlags_AcceptNoDrawDefaultRect |

					{

						ImGui::PushID(123);
						ImGui::SetDragDropPayload("DND_DEMO_CELL", &i, sizeof(int));
						//ImGui::SetNextWindowPos(ImVec2(m.x - 50, m.y - 30));

						////ImGui::Begin("1", nullptr, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMouseInputs);


						ImGui::Button(dGame.dll[i].dll.c_str(), ImVec2(100, 60));

						////ImGui::End();


						// Восстанавливаем стиль обводки кнопки
						ImGui::EndDragDropSource();
						ImGui::PopID();
					}
					ImGui::PopStyleColor();
					ImGui::PopStyleVar(2);

					if (ImGui::BeginDragDropTarget())
					{
						/*
						ImGui::Begin("1", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
						ImGui::Button(sDll[i].dll.c_str(), ImVec2(100, 60));
						ImGui::End();*/
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
						{
							IM_ASSERT(payload->DataSize == sizeof(int));

							int payload_n = *(const int*)payload->Data;
							std::swap(dGame.dll[i].dll, dGame.dll[payload_n].dll);
							std::swap(dGame.dll[i].include, dGame.dll[payload_n].include);

						}
						ImGui::EndDragDropTarget();
					}

					ImGui::PopID();
				}
				if (ImGui::Button("Запустить", ImVec2(100, 60))) {

					

					std::string dlls{};
					for (auto path : dGame.dll)
						if (path.include)
							dlls += path.dll + ",";

					if (!dlls.empty()) {
						dlls.resize(dlls.size() - 1);
					}

					std::string hud_params = dGame.hud.get();

					HANDLE hDevice = CreateFile("\\\\.\\BlueStreetDriver",
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						nullptr,
						OPEN_EXISTING,
						0,
						nullptr);

					if (hDevice == INVALID_HANDLE_VALUE) {
						printf("[!] Failed to open device\n");
						goto sd;
					}

					sMSG sendData;


					int wpathLength = MultiByteToWideChar(CP_UTF8, 0, dGame.dlls_FolderPath.c_str(), -1, NULL, 0);

					WCHAR* wpath = new WCHAR[wpathLength];

					MultiByteToWideChar(CP_UTF8, 0, dGame.dlls_FolderPath.c_str(), -1, wpath, wpathLength);

					std::wcout << wpath << std::endl;


					int wdllsLength = MultiByteToWideChar(CP_UTF8, 0, dlls.c_str(), -1, NULL, 0);

					WCHAR* wdlls = new WCHAR[wdllsLength];

					MultiByteToWideChar(CP_UTF8, 0, dlls.c_str(), -1, wdlls, wdllsLength);

					std::wcout << wdlls << std::endl;


					wcscpy_s(sendData.Path, wpath);
					wcscpy_s(sendData.DLLS, wdlls);

					DWORD returned;
					/*BOOL success = DeviceIoControl(hDevice,
						IOCTL_BLUESTREET_SEND_DATA,
						&sendData,
						sizeof(sendData),
						nullptr,
						0,
						&returned,
						nullptr);*/


					CloseHandle(hDevice);

					delete[] wpath;
					delete[] wdlls;

					//SetCurrentDirectory(fs::path(gamepath).parent_path().string().c_str());

					std::string dxvk_conf = fs::path(dGame.path).parent_path().string() + "\\dxvk.conf";

					if (fs::exists(dxvk_conf)) {
						
						std::ifstream inFile(dxvk_conf);
						if (inFile.is_open()) {
							std::string line;
							std::string output_t;
							std::string output;

							bool dxvk_dlls_FolderPath{ false };
							bool dxvk_dlls{ false };
							bool dxvk_hud{ false };

							// Читаем файл построчно
							while (std::getline(inFile, line)) {
								// Заменяем строки, если они соответствуют
								if (line.find("dxvk.dlls.FolderPath") != std::string::npos) {
									dxvk_dlls_FolderPath = true;
									line = "dxvk.dlls.FolderPath = \"" + dGame.dlls_FolderPath + "\"";
								}
								else if (line.find("dxvk.dlls") != std::string::npos) {
									dxvk_dlls = true;
									line = "dxvk.dlls = " + dlls;
								}
								else if (line.find("dxvk.hud") != std::string::npos) {
									dxvk_hud = true;
									line = "dxvk.hud = " + hud_params;
								}

								output_t += line + "\n";
							}

							inFile.close();

							if (!dxvk_dlls_FolderPath)
								output += "dxvk.dlls.FolderPath = \"" + dGame.dlls_FolderPath + "\"\n\n";
							
							if (!dxvk_dlls)
								output += "dxvk.dlls = " + dlls + "\n\n";
							
							if (!dxvk_hud)
								output += "dxvk.hud = " + hud_params + "\n\n";

							output += output_t;

							std::ofstream outFile(dxvk_conf);
							if (outFile.is_open()) {
								outFile << output;
								outFile.close();
								std::cout << "Параметры в файле " << dxvk_conf << " успешно изменены." << std::endl;
							}
							else {
								std::cerr << "Ошибка открытия файла для записи." << std::endl;
							}
						}
						else {
							std::cerr << "Ошибка открытия файла для чтения." << std::endl;
						}


					}
					else {

						std::ofstream outFile(dxvk_conf);
						if (!outFile) {
							std::cerr << "Ошибка создания файла " << std::endl;
						}

						outFile << "[" << fs::path(dGame.path).filename().string() << "]\n"
								<< "dxvk.dlls.FolderPath = \"" << dGame.dlls_FolderPath << "\"\n\n"
								<< "dxvk.dlls = " << dlls << "\n";



						CURL* curl;
						CURLcode res;
						const char* url = "https://raw.githubusercontent.com/doitsujin/dxvk/master/dxvk.conf";
						std::string output;

						// Инициализация библиотеки libcurl
						curl = curl_easy_init();
						if (curl) {
							// Установка URL для запроса
							curl_easy_setopt(curl, CURLOPT_URL, url);

							// Установка функции обратного вызова для записи данных в строку
							curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

							// Установка указателя на строку для записи
							curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

							// Выполнение запроса
							res = curl_easy_perform(curl);
							if (res != CURLE_OK) {
								std::cerr << "Ошибка выполнения запроса: " << curl_easy_strerror(res) << std::endl;
								curl_easy_cleanup(curl);
								return 1;
							}

							// Закрытие curl
							curl_easy_cleanup(curl);
						}
						else {
							std::cerr << "Ошибка инициализации libcurl" << std::endl;
							return 1;
						}

						// Изменение строки в полученных данных
						std::string searchString = "# dxvk.hud = ";
						std::string replaceString = "dxvk.hud = " + hud_params;
						size_t pos = output.find(searchString);
						if (pos != std::string::npos) {
							output.replace(pos, searchString.length(), replaceString);
						}
						else {
							std::cerr << "Строка для замены не найдена" << std::endl;
							return 1;
						}
	
						if (!outFile) {
							std::cerr << "Ошибка открытия файла для записи" << std::endl;
							return 1;
						}
						outFile << output;
						outFile.close();

					}


					//_spawnl(_P_NOWAIT, gamepath.c_str(), gamepath.c_str(), "-dxvk", nullptr);

					//game = false;

				}
				if (ImGui::Button("Настройки", ImVec2(100, 60)))
				{
					ImGui::OpenPopup("settings_game");
				}
			sd:
				;
			}

			ImGui::End();

		}


#pragma region ImGui DirectX Rendering
	#pragma region ImGui Render Setup
		ImGui::Render();

		g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	#pragma endregion
		
	#pragma region Scene Clear Begin
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(0, 0, 0, 255);
		g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
	#pragma endregion

	#pragma region ImGui PlatformWindows Render
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	#pragma endregion

	#pragma region Scene Present

		HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	#pragma endregion
#pragma endregion
	}
	//MessageBoxA(NULL, "CLOSING", "INFO", MB_HELP);
	/* delete[] selected;*/
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	UnregisterClass(wc.lpszClassName, wc.hInstance);

	if (r.joinable()) {
		r.detach();
	}
	if (future.joinable()) {
		future.detach();
	}


	CloseHandle(semaphore);
	std::cout << "sdada" << std::endl;
	return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) return E_FAIL;

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0) return E_FAIL;

	return S_OK;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}
static bool startCursorPosSet = false;
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		InitNotifyIconData(hWnd);
		Shell_NotifyIcon(NIM_ADD, &notifyIconData);
		InitTrayMenu();
		break;
	}
	case WM_SYSICON: {
		if (lParam == WM_RBUTTONDOWN) {
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, curPoint.x, curPoint.y, 0, hWnd, NULL);
		}
		break;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case ID_TRAY_EXIT:
			Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
			PostQuitMessage(0);
			break;
		case ID_TRAY_OPEN:
			MessageBox(NULL, TEXT("Открыть выбрано"), TEXT("Сообщение"), MB_OK);
			break;
		case ID_TRAY_DISABLE_TEST_MODE:
			MessageBox(NULL, TEXT("Тестовый режим отключен"), TEXT("Сообщение"), MB_OK);
			break;
		}
		break;
	}
	case WM_DESTROY: {
		Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

