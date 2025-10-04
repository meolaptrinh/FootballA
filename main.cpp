//@authr Nguyen Duc Minh Quan
#include <GLFW/glfw3.h>
#include<iostream>
#include<string>
#include<unordered_map>
#include<vector>
#include<functional>
#include<fstream>
#include <iterator>
#include <curl/curl.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "externlib/json.hpp"
#include "externlib/implot/implot.h"
#include "sever/sever.h"
#define STB_IMAGE_IMPLEMENTATION
#include "externlib/stb_image.h"
const int window_width = 800;
const int window_height = 600;
std::vector<bool>hide_seek = {false};
class openGL_draw{
public:
    openGL_draw(){};
    void rectangle(int x,int y,int width,int height,int R,int G,int B){
    int x1 = x+width;
    int y1 = y+height;
    float ndc_y = 1.0f- ((2.0f*y)/window_height);
    float ndc_y1 = 1.0f- ((2.0f*y1)/window_height);
    float ndc_x = 2.0f*x/window_width-1.0f;
    float ndc_x1 = 2.0f*x1/window_width-1.0f;
    glColor3f(R/255.0f,G/255.0f,B/255.0f);
    glBegin(GL_QUADS);
        glVertex2f(ndc_x,ndc_y1);
        glVertex2f(ndc_x1,ndc_y1);
        glVertex2f(ndc_x1,ndc_y);
        glVertex2f(ndc_x,ndc_y);
    glEnd();
    }

};
class Button{
public:
    int width;
    int height;
    int x;
    int y;
    int R,G,B;
    int vien;
    int vienR;
    int vienG;
    int vienB;
    std::function<void()> onClick;

    Button(int x,int y,int width,int height,int R,int G,int B,std::function<void()>_call,int vien=0,int vienR = 0,int vienG = 0,int vienB = 0):x(x),y(y),width(width),height(height),onClick(_call),R(R),G(G),B(B),vien(vien),vienR(vienR),
    vienG(vienG),vienB(vienB){}
    bool mouse_inside (double xm,double ym){
        return xm >= x && xm <= x + width && ym >= y && ym <= y + height;
    }
    void Draw(openGL_draw drawer){
        if(vien>0){
            drawer.rectangle(
                    x-int(vien),
                    y-int(vien),
                    width+2*int(vien),
                    height+2*int(vien),
                    vienR,vienG,vienB
                             );
        }
        drawer.rectangle(x,y,width,height,R,G,B);
    }
};
std::vector<Button>button_list;

void mouse_button_callback(GLFWwindow* window, int button,int action,int mods){
    if(button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)return;
    double x_mouse,y_mouse;
    glfwGetCursorPos(window,&x_mouse,&y_mouse);
    std::cout<<"Pressed x : "<<x_mouse<<" y : "<<y_mouse<<std::endl;

        for(Button &b:button_list){
            if(b.mouse_inside(x_mouse,y_mouse)){
                b.onClick();
            }
        }
}
struct Season{
    int year;
    int point;
};
struct Team{
    std::string name;
    std::vector<Season>seasons;
};
struct Match{
    std::string home_team;
    std::string away_team;
    int home_team_point;
    int away_team_point;
    std::string link_image_home_team;
    std::string link_image_away_team;
};
struct Logo{
    GLuint image;
    int w;
    int h;
};
bool DownloadImageToMemory(const std::string& url, std::vector<unsigned char>& outBuffer) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteVectorCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // cho phép redirect
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}
GLuint LoadTextureFromMemory(const std::vector<unsigned char>& buffer, int* out_width, int* out_height) {
    int channels;
    unsigned char* data = stbi_load_from_memory(buffer.data(), buffer.size(), out_width, out_height, &channels, 4);
    if (!data) {
        std::cerr << "Failed to load image from memory" << std::endl;
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *out_width, *out_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}
void lay_data_Match(Match& match,std::string file){
    std::ifstream file_match(file);
    nlohmann::json data_match;
    file_match>>data_match;

    match.away_team = data_match["matches"][0]["awayTeam"]["shortName"];
    match.link_image_away_team = data_match["matches"][0]["awayTeam"]["crest"];
    match.home_team = data_match["matches"][0]["homeTeam"]["shortName"];
    match.link_image_home_team = data_match["matches"][0]["homeTeam"]["crest"];
    match.away_team_point = data_match["matches"][0]["score"]["fullTime"]["away"];
    match.home_team_point = data_match["matches"][0]["score"]["fullTime"]["home"];
}
int main() {

    SeverStart();
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "FootballA", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window,mouse_button_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();(void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    //khoi tao font
    ImFont* font_id_01 = io.Fonts->AddFontFromFileTTF("assets/fonts/arial.ttf",16.0f);
    ImFont* font_id_02 = io.Fonts->AddFontFromFileTTF("assets/fonts/arial.ttf",20.0f);
    ImFont* font_id_03 = io.Fonts->AddFontFromFileTTF("assets/fonts/arial.ttf",120.0f);
    openGL_draw drawer;
    const char* team_list[] = {"Manchester United","Manchester City","Arsenal"};
    int team_current = 0;
    //du lieu vao

    std::ifstream file("assets/data/data.json");
    if(!file.is_open()){
        std::cerr<<"Load Data Error";
        return -1;
    }
    std::vector<Team>teams;
    nlohmann::json data;
    file>>data;
    for(auto& t:data["teams"]){
        Team team;
        team.name = t["name"];
        for(auto& s:t["seasons"]){
            Season season;
            season.year = s["year"];
            season.point = s["point"];
            team.seasons.push_back(season);
        }
        teams.push_back(team);
    }

    //lay du lieu tran dau
    std::vector<Match>lastest_PL_match;
    std::unordered_map<int,std::string>name_team_map;
    name_team_map[0] = "MU";
    name_team_map[1] = "MC";
    name_team_map[2] = "ARS";
    for(int i = 0;i<std::size(team_list);i++){
        Match match_PL;
        lay_data_Match(match_PL,"assets/data/"+name_team_map[i]+"_Match.json");
        lastest_PL_match.push_back(match_PL);
    }
    //anh xa ten
    std::unordered_map<int,std::string>short_name_team_map;
    short_name_team_map[0] = "Man United";
    short_name_team_map[1] = "Man City";
    short_name_team_map[2] = "Arsenal";

    //load Logo
    std::vector<Logo>logo_home_team;
    std::vector<Logo>logo_away_team;

    for(int i = 0;i<std::size(team_list);i++){
        std::vector<unsigned char>buffer;
        Logo logo;
               if(!DownloadImageToMemory(lastest_PL_match[i].link_image_home_team,buffer)){
            std::cerr<<"Khong load anh tu URL";
            return 0;
        }
        else if(buffer.empty()){
                std::cerr<<"Buffer rong";
                return 0;
        }
        else{
            logo.image = LoadTextureFromMemory(buffer,&logo.w,&logo.h);
            if(logo.image == 0){
            std::cerr<<"Load khong thanh cong anh";
            return 0;
            }
        }
        logo_home_team.push_back(logo);
        Logo logo_away;
        std::vector<unsigned char>buffer_away;
        if(DownloadImageToMemory(lastest_PL_match[i].link_image_away_team,buffer_away)){
                logo_away.image = LoadTextureFromMemory(buffer_away,&logo_away.w,&logo_away.h);
        }
        logo_away_team.push_back(logo_away);
        }


    //vong lap chinh
    while (!glfwWindowShouldClose(window)) {
        //khoi tao frame
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        glClearColor(1.0f, 1.0f, 1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        for(Button &b:button_list){
            b.Draw(drawer);
        }
        //ve nut chinh
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::PushFont(font_id_01);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,IM_COL32(38, 110, 224,255));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,IM_COL32(219, 53, 53,255));

        ImGui::Begin("MENU BUTTON",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize);
            if(ImGui::BeginCombo(u8"Chọn đội bóng",team_list[team_current])){
                for(int n = 0;n<IM_ARRAYSIZE(team_list);n++){
                    bool selected = (team_current==n);
                    if(ImGui::Selectable(team_list[n],selected))team_current=n;
                    if(selected)ImGui::SetItemDefaultFocus();
                    }
                ImGui::EndCombo();
                }
        ImGui::End();
        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(2);
        //ve bieu do
        ImGui::SetNextWindowSize(ImVec2(1024,300));
        ImGui::Begin("Biểu đồ",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoResize);
        std::vector<double>point;
        std::vector<double>years;
        for(auto& s:teams[team_current].seasons){
                point.push_back(s.point);
                years.push_back(s.year);
        }
        if(ImPlot::BeginPlot(u8"Điểm theo mùa",u8"Năm",u8"Điểm")){
            ImPlot::SetupAxisTicks(ImAxis_X1, years.data(), static_cast<int>(years.size()));
            ImPlot::PlotLine(teams[team_current].name.c_str(),years.data(),point.data(),static_cast<int>(point.size()));
            ImPlot::EndPlot();
        }
        ImGui::End();
        ImGui::PopFont();

        ImGui::PushFont(font_id_02);
        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0, 0, 0,255));
        ImGui::SetNextWindowPos(ImVec2(53,409));
        ImGui::Begin("Ketqua",nullptr,ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
        ImGui::Text(u8"Kết quả trận Premier League gần nhất của %s ", team_list[team_current]);
        ImGui::End();



        ImGui::SetNextWindowPos(ImVec2(53,459));
        ImGui::Begin("Match_home",nullptr,ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
            ImGui::Image((void*)(intptr_t)logo_home_team[team_current].image,ImVec2(logo_home_team[team_current].w*0.5f,logo_home_team[team_current].h*0.5f));
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(452,459));
        ImGui::Begin("Match_away",nullptr,ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
        ImGui::Image((void*)(intptr_t)logo_away_team[team_current].image,ImVec2(logo_away_team[team_current].w*0.5f,logo_away_team[team_current].h*0.5f));
        ImGui::End();

        ImGui::PopFont();

        ImGui::SetNextWindowPos(ImVec2(186,459));
        ImGui::PushFont(font_id_03);
        ImGui::Begin("Tiso",nullptr,ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
        ImGui::Text("%d%s%d",lastest_PL_match[team_current].home_team_point," - ",lastest_PL_match[team_current].away_team_point);
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopFont();


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
}
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        glfwTerminate();
        return 0;
}


