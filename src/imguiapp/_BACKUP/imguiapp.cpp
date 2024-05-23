#include "Header.h"


// Data


// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
RECT windowRect;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;

#define SPLASH_WINDOW_CLASS_NAME L"SplashWindow"
#define SPLASH_WINDOW_TITLE L"Splash Window"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

Cbuttonmask buttonmask;
std::string gamepath;
FILE* logFile;

bool ChangeDllPath{ false };
bool game = false;
std::vector<DllPath> sDll{};

bool customSort(const DllPath& a, const DllPath& b) {
	auto extractNumber = [](const std::string& s) {
		std::smatch match;
		std::regex_search(s, match, std::regex(R"(d3d([\d]+))"));
		if (!match.empty()) {
			return std::stoi(match[1]);
		}
		return -1;
		};

	// Получаем числовую часть из строк
	int number_a = extractNumber(a.dll);
	int number_b = extractNumber(b.dll);

	// Если числовая часть у строк разная, то сортируем по ней
	if (number_a != number_b) {
		return number_a < number_b;
	}
	// Иначе сортируем по алфавиту
	else {
		return a.dll < b.dll;
	}
}

std::vector<bool> checkboxes;
std::vector <std::thread> threads;


//void ConvertToBoolArray(const std::string& hexString, bool data[], size_t size) {
//	// Преобразуем шестнадцатеричную строку в std::bitset<64>
//	std::bitset<64> mask(std::stoull(hexString, nullptr, 16));
//
//	// Извлекаем булевские значения из std::bitset<64> и записываем в массив data
//	for (size_t i = 0; i < size; ++i) {
//		data[i] = mask.test(i);
//	}
//}
//
//std::string CalculateAffinity(bool data[], size_t size) {
//	// Создаем маску сходства, изначально устанавливая все биты в 0
//	std::bitset<64> mask(0);
//
//	// Устанавливаем соответствующий бит в маске для каждого включенного элемента
//	for (size_t i = 0; i < size; ++i) {
//		if (data[i]) {
//			mask.set(i);
//		}
//	}
//
//	// Преобразуем маску сходства в шестнадцатеричный формат
//	std::stringstream stream;
//	stream << std::hex << std::uppercase << mask.to_ullong();
//	return stream.str();
//}

std::string folderPath{};

struct ButtonData {
	std::string label;
	bool isBeingDragged;
};

//class Console {
//public:
//	Console() : isOpen_(true) {}
//
//	void Draw(const char* title) {
//		if (!isOpen_) return;
//		ImGui::SetNextWindowPos(ImVec2(300,300), ImGuiCond_Once);
//		ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
//		ImGui::Begin(title, &isOpen_, ImGuiWindowFlags_MenuBar);
//		if (ImGui::BeginMenuBar()) {
//			if (ImGui::MenuItem("Clear")) ClearLog();
//			ImGui::EndMenuBar();
//		}
//
//		ImGui::Separator();
//		ImGui::BeginChild("ScrollingRegion", ImVec2(0, 180), false, ImGuiWindowFlags_HorizontalScrollbar);
//
//		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
//		for (const auto& item : items_)
//			ImGui::TextUnformatted(item.c_str());
//		if (scrollToBottom_)
//			ImGui::SetScrollHereY(1.0f);
//		scrollToBottom_ = false;
//		ImGui::PopStyleVar();
//
//		ImGui::EndChild();
//		ImGui::End();
//	}
//
//	void AddLog(const char* fmt, ...) IM_FMTARGS(2) {
//		va_list args;
//		va_start(args, fmt);
//		char buffer[1024];
//		vsnprintf(buffer, IM_ARRAYSIZE(buffer), fmt, args);
//		buffer[IM_ARRAYSIZE(buffer) - 1] = 0;
//		va_end(args);
//		items_.push_back(buffer);
//		scrollToBottom_ = true;
//	}
//
//	void ClearLog() {
//		items_.clear();
//	}
//
//private:
//	std::vector<std::string> items_;
//	bool scrollToBottom_;
//	bool isOpen_;
//};
//
//Console console;

//void AddLogToConsole(const char* fmt, ...) {
//	va_list args;
//	va_start(args, fmt);
//	char buffer[1024];
//	vsnprintf(buffer, IM_ARRAYSIZE(buffer), fmt, args);
//	buffer[IM_ARRAYSIZE(buffer) - 1] = 0;
//	va_end(args);
//	console.AddLog("%s", buffer);
//}


std::string programm_folder;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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

		HINSTANCE result = ShellExecute(NULL, "runas", argv[0], arg.c_str(), NULL, SW_SHOWNORMAL);
		if ((int)result <= 32) {
			DWORD error = GetLastError();
			std::cerr << "Failed to execute the program with elevated privileges. Error code: " << error << std::endl;
			return 0;
		}
		return 0;
	}

	bool testsigning = gettestsigning();

	if (!testsigning)
		return 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp("game", argv[i]) == 0)
		{
			if (i + 1 < argc)
				gamepath = argv[i + 1];
			game = true;

		}
		else if (strcmp("main", argv[i]) == 0)
		{

			main = true;
		}
	}

	std::cout << gamepath << std::endl;
	std::cout << argc << std::endl;
	std::cout << "Server" << std::endl;

	for (size_t i = 0; i < argc; i++)
	{
		std::cout << argv[i] << "\t";
		if (argv[i] == "game")
			game = true;

	}
	std::thread r{ [&] {
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
					if (i + 1 < substrings.size())
					{
						size_t pos = substrings[i + 1].find_last_of('.');

						if (pos != std::string::npos)
							if (substrings[i + 1].substr(pos) == ".exe" || substrings[i + 1].substr(pos) == ".exe\"")
							{
								gamepath = substrings[i + 1];
								gamepath.erase(std::remove(gamepath.begin(), gamepath.end(), '\"'), gamepath.end());
							}
							else
								continue;
						else
							continue;
						std::cout << "gamepath:: " << gamepath << std::endl;
					}
					updategamewindowpos = true;
					game = true;
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
	} };

		
	programm_folder = argv[0];
	size_t pos = programm_folder.find("imguiapp.exe");
	if (pos != std::string::npos) {
		programm_folder.erase(pos, std::string::npos);
	}

	ServiceManager driver{ programm_folder };
	{
		if (!driver.Install())
		{
			MessageBoxA(NULL, "Ошибка установки драйвера", "ERROR", MB_ICONERROR);
			return 1;
		}
	}

#pragma region InitializationImgui
	RECT desktop;
	GetWindowRect(GetDesktopWindow(), &desktop);
	WNDCLASSEX wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, AppClass, nullptr };
	RegisterClassEx(&wc);
	hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, AppClass, AppName, WS_POPUP | WS_OVERLAPPED, 0/*(desktop.right / 2) - (WindowWidth / 2)*/, 0, 200, WindowHeight, 0, 0, wc.hInstance, 0);

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

	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	ImVec2 CenteredMain{ ((float)screenWidth - WindowWidth) / 2.f ,  ((float)screenHeight - WindowHeight) / 2.f };

	std::cout << "(float)screenWidth / 2.f  + WindowWidth)" << (desktop.right / 2) - (WindowWidth / 2) << std::endl;

	DWORD dwFlag = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse; //| ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoScrollWithMouse
#pragma endregion


	bool show_demo_window = { true };
	bool show_another_window = { false };

	bool async_theard{ false };
	std::string token = "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";
	size_t indexVKD3D;

	std::vector<ReleaseInfo> emb;

	auto future = std::async([&] {
		emb = GetReleases({ "doitsujin/dxvk", "HansKristian-Work/vkd3d-proton" }, token);

		auto it = std::find_if(emb.begin(), emb.end(), [](const ReleaseInfo& release) {
			return release.typebin == VKD3D;
			});

		if (it != emb.end())
			indexVKD3D = std::distance(emb.begin(), it);

		getInstalledVersions( emb);
		async_theard = true;
		});

	float prevProgress = 0.0f;
	float lastUpdateTime = 0.0f;
	bool isFirstFrame = true;
	std::string modifybuttontext;
	bool showWindow = true;
	int selectedOption = -1;

	//bool data[] = { true, false, true, true, false, true, false };

	//bool data2[7] = {};

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
				MessageBoxA(NULL, "Ошибка установки драйвера", "ERROR", MB_ICONERROR);
			}
			if (msg.message == WM_ENDSESSION)

			{
				MessageBoxA(NULL, "Ошибка установки драйвера", "ERROR", MB_ICONERROR);
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

		if(main){
			
			ImGui::SetNextWindowPos(CenteredMain, ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(WindowWidth, WindowHeight), ImGuiCond_Once);
			ImGui::Begin("Dear ImGui Standalone Window", nullptr, dwFlag);

			deinmodifyModal(emb, async_theard);

			ImGui::SetCursorPosX(190);
			ImGui::Text("DXVK");
			ImGui::SameLine();
			ImGui::SetCursorPosX(615);
			ImGui::Text("VKD3D");


			ComboListVer(emb, async_theard, "DXVK", 0, indexVKD3D);
			ImGui::SameLine();
			ComboListVer(emb, async_theard, "VKD3D", indexVKD3D, emb.size());


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


			buttonmask.i[1] = std::count_if(emb.begin(), emb.end(), [](const auto& elem) {
				return elem.flag == install_types::installed && elem.pending;
				});
			buttonmask.i[0] = std::count_if(emb.begin(), emb.end(), [](const auto& elem) {
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
					std::thread downloadThread(DownloadInThreads, std::ref(emb), 5);
					downloadThread.detach();
				}

				if (buttonmask.i[1] > 0)
					ImGui::OpenPopup("1233");

			}

			if (ImGui::Button("Открыть", ImVec2(100, 20)))
				game = !game;
			
			
			//if (ImGui::Button("Открытьddd", ImVec2(100, 20)))
			//	AddLogToConsole("Hello, world!");


			if (ImGui::Button("close", ImVec2(100, 20)))
				done = true;
			ImGui::Text("%s", (game ? "true" : "false"));
			//for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); ++i) {
			//    ImGui::Checkbox(("##Checkbox" + std::to_string(i)).c_str(), &data[i]);
			//    ImGui::SameLine();
			//    ImGui::Text("%s", (data[i] ? "true" : "false"));
			//}

			// Вычисление и вывод сходства (affinity)
			//std::string affinity = CalculateAffinity(data, sizeof(data) / sizeof(data[0]));
			//ImGui::Text("affinity: 0x%s", affinity.c_str());

			//ConvertToBoolArray(affinity, data2, sizeof(data2) / sizeof(data2[0]));


			//ImGui::Text("out:");
			//for (size_t i = 0; i < sizeof(data2) / sizeof(data2[0]); ++i) {
			//	ImGui::Checkbox(("##checkbox2" + std::to_string(i)).c_str(), &data2[i]);
			//	ImGui::SameLine();
			//	ImGui::Text("%s", (data2[i] ? "true" : "false"));
			//}


			




			ImGui::End();
		}

		ImGui::ShowDemoWindow();

		if(game){
			if (updategamewindowpos)
			{
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				ImGui::SetNextWindowPos(ImVec2(cursorPos.x, cursorPos.y));
				updategamewindowpos = false;
			}
			ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_Once);
			ImGui::Begin("2Window", nullptr, dwFlag);

			ComboListVer(emb, async_theard, "DXVK1", 0, indexVKD3D, Flag_ShowOnlyInstalled);

			if (!folderPath.empty()) {
				if (ChangeDllPath)
				{
					sDll.clear();
					for (const auto& entry : fs::directory_iterator(folderPath + "/x64")) {
						if (entry.is_regular_file()) {
							sDll.push_back(DllPath(entry.path().filename().string(), false));
						}
					}
					std::sort(sDll.begin(), sDll.end(), customSort);
					ChangeDllPath = false;
				}
				static bool st = false;
				ImGui::Text("%s", st ? "true" : "false");
				for (int i = 0; i < sDll.size(); i++)
				{

					ImGui::PushID(i);

					if (!sDll[i].include)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 0.2f));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
					}

					if (ImGui::Button(sDll[i].dll.c_str(), ImVec2(100, 60)))
						sDll[i].include = !sDll[i].include;
					if (i < sDll.size() - 1)
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


						ImGui::Button(sDll[i].dll.c_str(), ImVec2(100, 60));

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
							std::swap(sDll[i].dll, sDll[payload_n].dll);

						}
						ImGui::EndDragDropTarget();
					}

					ImGui::PopID();
				}
				if (ImGui::Button("Запустить", ImVec2(100, 60))) {

					std::string dlls{};
					for (auto path : sDll)
						if (path.include)
							dlls += path.dll + ";";

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


					int wpathLength = MultiByteToWideChar(CP_UTF8, 0, folderPath.c_str(), -1, NULL, 0);

					WCHAR* wpath = new WCHAR[wpathLength];

					MultiByteToWideChar(CP_UTF8, 0, folderPath.c_str(), -1, wpath, wpathLength);

					std::wcout << wpath << std::endl;


					int wdllsLength = MultiByteToWideChar(CP_UTF8, 0, dlls.c_str(), -1, NULL, 0);

					WCHAR* wdlls = new WCHAR[wdllsLength];

					MultiByteToWideChar(CP_UTF8, 0, dlls.c_str(), -1, wdlls, wdllsLength);

					std::wcout << wdlls << std::endl;


					wcscpy_s(sendData.Path, wpath);
					wcscpy_s(sendData.DLLS, wdlls);

					DWORD returned;
					BOOL success = DeviceIoControl(hDevice,
						IOCTL_BLUESTREET_SEND_DATA,
						&sendData,
						sizeof(sendData),
						nullptr,
						0,
						&returned,
						nullptr);


					CloseHandle(hDevice);

					delete[] wpath;
					delete[] wdlls;

					SetCurrentDirectory(fs::path(gamepath).parent_path().string().c_str());

					_spawnl(_P_NOWAIT, gamepath.c_str(), gamepath.c_str(), "-dxvk", nullptr);

					game = false;

				}
			sd:
				;
			}

			ImGui::End();

		}
		
		//{
		//	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Устанавливаем прозрачный цвет фона
		//	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Устанавливаем прозрачный цвет обводки
		//	ImVec2 m = ImGui::GetIO().MousePos;
		//	ImGui::SetNextWindowPos(ImVec2(m.x - 50, m.y - 30));
		//	ImGui::Begin("1", nullptr,  ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMouseInputs);
		//	ImGui::Button("sDll[i].dll.c_str()", ImVec2(100, 60));
		//	ImGui::End();
		//	ImGui::PopStyleColor(2);
		//}

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

	//r.detach();

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

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) return 0; // Disable ALT application menu
		break;

	case WM_CLOSE:
		MessageBoxA(NULL, "Ошибка установки драйвера", "ERROR", MB_ICONERROR);
		DestroyWindow(hWnd);
		return 0;

	case WM_ENDSESSION:
		if (wParam)
		{
			MessageBoxA(NULL, "Ошибка установки драйвера", "ERROR", MB_ICONERROR);
			PostQuitMessage(0);
		}
		return 0;

	case WM_NCHITTEST:
	{
		ImVec2 Shit = ImGui::GetMousePos();
		if (Shit.y < 25 && Shit.x < WindowWidth - 25)
		{
			LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam);
			if (hit == HTCLIENT) hit = HTCAPTION;
			return hit;
		}
		else break;
	}

	case WM_DESTROY:
		std::cout << "WM_DESTROY" << std::endl;
		PostQuitMessage(0);
		return 0;

	default:
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

