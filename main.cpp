//@authr Nguyen Duc Minh Quan
#include <GLFW/glfw3.h>
#include<iostream>
#include<string>
#include<vector>
#include<functional>
#include<fstream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "externlib/json.hpp"
#include "externlib/implot/implot.h"
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
int main() {

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
        ImGui::PopFont();
        ImGui::PopStyleColor(2);
        //ve bieu do
        ImGui::Begin("Biểu đồ",nullptr,ImGuiWindowFlags_NoTitleBar);
        std::vector<double>point;
        std::vector<double>years;
        for(auto& s:teams[team_current].seasons){
                point.push_back(s.point);
                years.push_back(s.year);
        }
        if(ImPlot::BeginPlot(u8"Điểm theo mùa",u8"Năm",u8"Điểm")){
            ImPlot::SetupAxisFormat(ImAxis_X1,"%d");
            ImPlot::PlotLine(teams[team_current].name.c_str(),years.data(),point.data(),static_cast<int>(point.size()));
            ImPlot::EndPlot();
        }
        ImGui::End();
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


