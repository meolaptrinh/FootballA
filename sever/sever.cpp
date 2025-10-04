#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include "../externlib/json.hpp"

size_t WriteCallBack(void* content, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)content, size * nmemb);
    return size * nmemb;
}
size_t WriteVectorCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::vector<unsigned char>* buffer = (std::vector<unsigned char>*)userp;
    size_t totalSize = size * nmemb;
    buffer->insert(buffer->end(), (unsigned char*)contents, (unsigned char*)contents + totalSize);
    return totalSize;
}
void get_infor_from_api(const char* link,const char* name_file){
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL!" << std::endl;
        return;
    }

    std::string response;
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "X-Auth-Token: ea17100d5df145c594080fc09630571b");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, link);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return;
    }

    if (response.empty()) {
        std::cerr << "Empty response from server!" << std::endl;
        return;
    }

    try {
        nlohmann::json j = nlohmann::json::parse(response);
        std::ofstream file(name_file);
        if (!file.is_open()) {
            std::cerr << "Failed to open file!" << std::endl;
            return;
        }

        file << j.dump(4); // Ghi JSON đẹp
        file.close();

    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }

}
void SeverStart() {
    get_infor_from_api("https://api.football-data.org/v4/teams/66/matches?status=FINISHED&competitions=PL&season=2025&limit=1","assets/data/MU_Match.json");
    get_infor_from_api("https://api.football-data.org/v4/teams/65/matches?status=FINISHED&competitions=PL&season=2025&limit=1","assets/data/MC_Match.json");
    get_infor_from_api("https://api.football-data.org/v4/teams/57/matches?status=FINISHED&competitions=PL&season=2025&limit=1","assets/data/ARS_Match.json");
}
