#include "Header.h"

std::mutex mtx;
std::vector<int> taskProgress{};
float progress = 0.0f;
size_t count_download = 0;

HRGN CreateRoundRectRgn(int x, int y, int width, int height, int radius) {
    return CreateRoundRectRgn(x + 1, y + 1, x + width - 1, y + height - 1, radius, radius);
}

void SetWindowRoundRect(HWND hwnd, int width, int height, int radius) {
    HRGN hRgn = CreateRoundRectRgn(0, 0, width, height, radius);
    SetWindowRgn(hwnd, hRgn, TRUE);
    DeleteObject(hRgn);
}

bool HasFlag(FlagsState flag, FlagsState& mask) {
    return (flag & mask) != 0;
}

modify_types operator++(modify_types& value) {
    value = static_cast<modify_types>(static_cast<int>(value) + 1);
    if (value == modify_types::maxvalue) {
        value = modify_types::none;
    }
    return value;
}

std::string extractFileName(const std::string& url) {
    std::regex pattern(R"(/([^/]+)$)");
    std::smatch match;

    if (std::regex_search(url, match, pattern)) {
        return match[1].str();
    }
    else {
        return url;
    }
}
std::mutex logMutex;

void logToFile(const char* format, ...) {
    std::lock_guard<std::mutex> lock(logMutex);

    fopen_s(&logFile, std::string{ programm_folder + "log.txt" }.c_str(), "a");
    if (logFile != nullptr)
    {
        va_list args;
        va_start(args, format);
        vfprintf(logFile, format, args);
        va_end(args);

        fclose(logFile);
    }

}

bool CompareReleases(const ReleaseInfo& release1, const ReleaseInfo& release2) {
    return release1.version == release2.version && release1.downloadUrl == release2.downloadUrl;
}

std::vector<std::string> getInstalledVersions(std::vector<ReleaseInfo>& ver) {
    std::vector<std::string> result;

    std::regex versionRegex(("^([^-]+)[A-z-]+(.+)$"));

    std::smatch match;
    DbgPrint("Found: ");
    for (const auto& entry : fs::directory_iterator(programm_folder + "installed")) {
        if (entry.is_directory()) {
            std::string folderName = entry.path().filename().string();

            if (std::regex_search(folderName, match, versionRegex)) {
                if (match.size() > 2) {
                    for (auto& str : ver) {
                        if ((match[1].str() == "dxvk" && str.typebin == DXVK) ||
                            (match[1].str() == "vkd3d" && str.typebin == VKD3D)) {
                            if (str.version == "Version " + match[2].str()) {
                                DbgPrint("\"%s\";\t", match[0].str().c_str());
                                str.flag = installed;
                                break;
                            }
                        }
                    }

                    result.push_back(match[1].str() + match[2].str());
                }
            }
        }
    }
    DbgPrint("\n");
    return result;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(reinterpret_cast<char*>(contents), totalSize);
    return totalSize;
}

size_t WriteCallbackd(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* file = static_cast<std::ofstream*>(userp);
    file->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

void Download(const size_t& taskId, ReleaseInfo& arr) {
    if (arr.flag == installed && arr.modify == modify_types::reinstall)
    {
        try {
            fs::remove_all(arr.getfoldername());
            arr.flag = install_types::none;
            DbgPrint("Файл(%s) успешно удален.\n", arr.getfoldername().c_str());
        }
        catch (const fs::filesystem_error& e) {
            arr.flag = install_types::error;
            DbgPrint("Не удалось удалить файл(%s): %s\n", arr.getfoldername().c_str(), e.what());
        }
    }

    CURL* curl;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

    if (curl) {
        fs::path tempPath = fs::temp_directory_path();
        fs::path filePath = tempPath / extractFileName(arr.downloadUrl.c_str());

        std::ofstream file(filePath, std::ios::binary);
        if (file.is_open()) {
            ProgressData progressData;
            progressData.lastPercentage = 0.0;
            progressData.taskId = taskId;

            curl_easy_setopt(curl, CURLOPT_URL, arr.downloadUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackd);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);


            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressData);

            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);

            CURLcode res = curl_easy_perform(curl);
            file.close();

            if (res != CURLE_OK) {
                DbgPrint("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            else {
                std::string command = "C:\\Windows\\System32\\tar.exe -xkzf " + filePath.string() + " -C ./installed";

                try {
                    boost::process::child process(command, boost::process::std_out > boost::process::null);
                    process.wait();

                    int exitCode = process.exit_code();

                    if (exitCode == 0) {
                        arr.flag = install_types::installed;
                        arr.modify = modify_types::none;
                        arr.pending = false;

                        DbgPrint("Successfully installed(%s)\n", filePath.string().c_str());
                    }
                    else {
                        arr.flag = install_types::error;
                        DbgPrint("Extraction failed with exit code: %d\n", exitCode);
                    }


                }
                catch (const boost::process::process_error& e) {
                    DbgPrint("Error during extraction: %s\n", e.what());
                }
            }
        }
        else {
            DbgPrint("The file(%s) could not be opened for writing.\n", filePath.string().c_str());
        }

        try {
            fs::remove(filePath);
            DbgPrint("The file(%s) was successfully deleted.\n", filePath.string().c_str());
        }
        catch (const fs::filesystem_error& e) {
            DbgPrint("The file(%s) could not be deleted: %s\n", filePath.string().c_str(), e.what());
        }
        curl_easy_cleanup(curl);
    }


    curl_global_cleanup();
}

void DownloadInThreads(std::vector<ReleaseInfo>& ver, int numThreads) {

    progress = 0;

    count_download = std::count_if(ver.begin(), ver.end(), [](const auto& x) {
        return (x.pending && x.flag != install_types::installed) || (x.pending && x.modify == modify_types::reinstall);
        });

    taskProgress.resize(count_download);

    std::vector<std::thread> threads;

    for (size_t i = 0, j = 0; i < ver.size(); i++) {
        if (ver[i].pending && ver[i].flag != install_types::installed || (ver[i].pending && ver[i].modify == modify_types::reinstall))
        {
            threads.emplace_back([arr = std::ref(ver[i]), taskId = j++, numThreads]() {
                Download(taskId, arr);
                });
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }
    count_download = 0;
    progress = 1;
    taskProgress.clear();
    taskProgress.resize(0);

}

int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t /*ultotal*/, curl_off_t /*ulnow*/) {
    ProgressData* progressData = static_cast<ProgressData*>(clientp);

    if (dltotal > 0) {
        float percentage = (dlnow / (float)dltotal) * 100.0;

        if (percentage - progressData->lastPercentage > 1.0) {

            std::lock_guard<std::mutex> lock(mtx);

            taskProgress[progressData->taskId] = percentage;
            float totalProgress = 0;

            for (auto& progress : taskProgress)
                totalProgress += progress;

            progress = totalProgress / 100 / count_download;

            progressData->lastPercentage = percentage;
        }
    }

    return 0;
}





std::vector<ReleaseInfo> GetReleases(const std::vector<std::string>& repositories, const std::string& token) {
    std::vector<ReleaseInfo> releases;
    std::vector<std::string> jsonFilePath{ programm_folder + "json\\dxvk.json", programm_folder + "json\\vkd3d.json" };
    size_t i = 0;
    for (const auto& repository : repositories) {
        std::string apiUrl = "https://api.github.com/repos/" + repository + "/releases";

        /*if (std::filesystem::exists(jsonFilePath) && std::filesystem::file_size(jsonFilePath) > 0) {

        }*/

        CURL* curl;
        CURLcode res;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
            headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json");
            headers = curl_slist_append(headers, "User-Agent: Your-User-Agent-Name");  // �������� "Your-User-Agent-Name" �� ���� ��� ������������ ��� �������� ����������
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            std::string responseString;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

            res = curl_easy_perform(curl);


            bool badAnswer{ false };
            bool apiError{ false };
            bool needUpdate{ false };
            if (res != CURLE_OK) {
                DbgPrint("Error get:\n");
            }
            else {
                std::string content;
                std::filesystem::create_directories(std::filesystem::path(jsonFilePath[i]).parent_path());
                std::fstream inputFile(jsonFilePath[i], std::ios::in);

                if (inputFile.is_open()) {
                    std::stringstream buffer;
                    buffer << inputFile.rdbuf();
                    content = buffer.str();
                    inputFile.close();
                }

                json jsonResponse;
                json jsonFile;

                try {
                    jsonResponse = json::parse(responseString);
                }
                catch (const json::parse_error& e) {
                    DbgPrint("Error parsing response JSON: %s\n", e.what());
                    badAnswer = true;
                }

                try {
                    jsonFile = json::parse(content);
                }
                catch (const json::parse_error& e) {
                    DbgPrint("Error parsing file JSON: %s\n", e.what());
                    if (badAnswer)
                        goto EXIT;

                    needUpdate = true;
                }

            APIERROR:
                for (const auto& release : badAnswer || apiError ? jsonFile : jsonResponse) {
                    ReleaseInfo releaseInfo;
                    releaseInfo.typebin = releaseInfo.gettypebinfromrepos(repository);
                    if (release.find("name") != release.end() && release["name"].is_string()) {
                        releaseInfo.version = release["name"].get<std::string>();
                    }
                    else {
                        DbgPrint("%s\n", responseString.c_str());
                        apiError = true;
                        if (needUpdate)
                            goto EXIT;
                        goto APIERROR;
                    }

                    if (release.find("assets") != release.end() && release["assets"].is_array() &&
                        !release["assets"].empty() && release["assets"][0].find("browser_download_url") != release["assets"][0].end() &&
                        release["assets"][0]["browser_download_url"].is_string()) {
                        releaseInfo.downloadUrl = release["assets"][0]["browser_download_url"].get<std::string>();
                    }
                    else {
                        DbgPrint("%s\n", responseString.c_str());
                        apiError = true;
                        if (needUpdate)
                            goto EXIT;
                        goto APIERROR;
                    }

                    releaseInfo.downloadUrl = release["assets"][0]["browser_download_url"].get<std::string>();

                    if (!needUpdate && !badAnswer && !apiError)
                    {
                        auto it = std::find_if(jsonFile.begin(), jsonFile.end(), [&](const auto& fileRelease) {
                            ReleaseInfo fileReleaseInfo;
                            fileReleaseInfo.version = fileRelease["name"].get<std::string>();
                            fileReleaseInfo.downloadUrl = fileRelease["assets"][0]["browser_download_url"].get<std::string>();
                            return CompareReleases(releaseInfo, fileReleaseInfo);
                            });

                        if (it != jsonFile.end()) {
                            DbgPrint("%s \t %s\n", releaseInfo.version.c_str(), releaseInfo.downloadUrl.c_str());
                        }
                        else {
                            DbgPrint("WRITE JSON FILE ->:\n\t\tRelease not found in the second JSON file: %s \t %s\n", releaseInfo.version.c_str(), releaseInfo.downloadUrl.c_str());
                            needUpdate = true;
                        }
                    }

                    releases.push_back(releaseInfo);
                }
                if (needUpdate)
                {
                    DbgPrint("UPDATE FILE\n");
                    inputFile.open(jsonFilePath[i], std::ios::out | std::ios::trunc);
                    if (inputFile.is_open()) {
                        inputFile << responseString;
                        inputFile.close();
                    }
                }
            }

        EXIT:
            DbgPrint("badAnswer : %d\n", badAnswer);
            DbgPrint("apiError : %d\n", apiError);
            DbgPrint("needUpdate : %d\n", needUpdate);

            if (apiError && needUpdate)
                MessageBoxA(NULL, "NETWORK && FILE", "ERROR", 0);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
        }
        i++;
    }
    return releases;
}




//void LoadFontFromResource(ImFont*& font)
//{
//    HRSRC hResource = FindResource(nullptr, MAKEINTRESOURCE(101), RT_FONT);
//    if (hResource)
//    {
//        HGLOBAL hGlobal = LoadResource(nullptr, hResource);
//        if (hGlobal)
//        {
//            const void* fontData = LockResource(hGlobal);
//            int fontSize = 18;
//
//            ImGuiIO& io = ImGui::GetIO();
//            ImFontAtlas* atlas = io.Fonts;
//
//            font = atlas->AddFontFromMemoryTTF(const_cast<void*>(fontData), hGlobal ? SizeofResource(nullptr, hResource) : 0, fontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
//            io.Fonts->Build();
//            FreeResource(hGlobal);
//
//        }
//    }
//}

//std::vector<ReleaseInfo> GetReleases(const std::string& repository, const std::string& token, const std::string& jsonFilePath) {
//    std::string apiUrl = "https://api.github.com/repos/" + repository + "/releases";
//    std::vector<ReleaseInfo> releases;
//
//    /*if (std::filesystem::exists(jsonFilePath) && std::filesystem::file_size(jsonFilePath) > 0) {
//
//    }*/
//
//    CURL* curl;
//    CURLcode res;
//
//    curl_global_init(CURL_GLOBAL_DEFAULT);
//    curl = curl_easy_init();
//
//    if (curl) {
//        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
//        struct curl_slist* headers = nullptr;
//        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
//        headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json");
//        headers = curl_slist_append(headers, "User-Agent: Your-User-Agent-Name");  // �������� "Your-User-Agent-Name" �� ���� ��� ������������ ��� �������� ����������
//        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//
//        std::string responseString;
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
//
//        res = curl_easy_perform(curl);
//
//        
//        bool badAnswer { false };
//        bool apiError { false };
//        bool needUpdate { false };
//        if (res != CURLE_OK) {
//            std::cout << "Error get : " << std::endl;
//        }
//        else {
//            std::string content;
//            std::filesystem::create_directories(std::filesystem::path(jsonFilePath).parent_path());
//            std::fstream inputFile(jsonFilePath, std::ios::in);
//
//            if (inputFile.is_open()) {
//                std::stringstream buffer;
//                buffer << inputFile.rdbuf();
//                content = buffer.str();
//                inputFile.close();
//            }
//
//            json jsonResponse;
//            json jsonFile;
//           
//            try {
//                jsonResponse = json::parse(responseString);
//            }
//            catch (const json::parse_error& e) {
//
//                std::cout << "Error parsing response JSON : " << e.what() << std::endl;
//                badAnswer = true;
//            }
//
//            try {
//                jsonFile = json::parse(content);
//            }
//            catch (const json::parse_error& e) {
//                std::cout << "Error parsing file JSON: " << e.what() << std::endl;
//                if (badAnswer)
//                    goto EXIT;
//
//                needUpdate = true;
//            }
//
//            APIERROR:
//            for (const auto& release : badAnswer || apiError ? jsonFile : jsonResponse) {
//                ReleaseInfo releaseInfo;
//
//                if (release.find("name") != release.end() && release["name"].is_string()) {
//                    releaseInfo.version = release["name"].get<std::string>();
//                }
//                else {
//                    std::cout << responseString << std::endl;
//                    apiError = true;
//                    if (needUpdate)
//                        goto EXIT;
//                    goto APIERROR;
//                }
//
//                if (release.find("assets") != release.end() && release["assets"].is_array() &&
//                    !release["assets"].empty() && release["assets"][0].find("browser_download_url") != release["assets"][0].end() &&
//                    release["assets"][0]["browser_download_url"].is_string()) {
//                    releaseInfo.downloadUrl = release["assets"][0]["browser_download_url"].get<std::string>();
//                }
//                else {
//                    std::cout << responseString << std::endl;
//                    apiError = true;
//                    if (needUpdate)
//                        goto EXIT;
//                    goto APIERROR;
//                }
//
//                releaseInfo.downloadUrl = release["assets"][0]["browser_download_url"].get<std::string>();
//
//                if (!needUpdate && !badAnswer && !apiError)
//                {
//                    auto it = std::find_if(jsonFile.begin(), jsonFile.end(), [&](const auto& fileRelease) {
//                        ReleaseInfo fileReleaseInfo;
//                        fileReleaseInfo.version = fileRelease["name"].get<std::string>();
//                        fileReleaseInfo.downloadUrl = fileRelease["assets"][0]["browser_download_url"].get<std::string>();
//                        return CompareReleases(releaseInfo, fileReleaseInfo);
//                        });
//
//                    if (it != jsonFile.end()) {
//                        std::cout << releaseInfo.version << "\t" << releaseInfo.downloadUrl << "\n";
//                    }
//                    else {
//                        std::cout << "WRITE JSON FILE ->:\n\t\tRelease not found in the second JSON file: " << releaseInfo.version << "\t" << releaseInfo.downloadUrl << "\n";
//                        needUpdate = true;
//                    }
//                }
//
//                releases.push_back(releaseInfo);
//            }
//            if (needUpdate)
//            {
//                std::cout << "UPDATE FILE\n";
//                inputFile.open(jsonFilePath, std::ios::out | std::ios::trunc);
//                if (inputFile.is_open()) {
//                    inputFile << responseString;
//                    inputFile.close();
//                }
//            }
//        }
//        
//    EXIT:
//    std::cout << "badAnswer : " << badAnswer << std::endl;
//    std::cout << "apiError : " << apiError << std::endl;
//    std::cout << "needUpdate : " << needUpdate << std::endl;
//    
//        if(apiError && needUpdate)
//            MessageBoxA(NULL, "NETWORK && FILE", "ERROR", 0);
//        curl_easy_cleanup(curl);
//        curl_global_cleanup();
//    }
//
//    return releases;
//}
//
