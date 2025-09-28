#include <GLFW/glfw3.h>
#include<iostream>
#include<string>
#include<vector>
#include<functional>
const int window_width = 800;
const int window_height = 600;
std::string scene = "Menu";
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
    std::function<void()> onClick;

    Button(int x,int y,int width,int height,int R,int G,int B,std::function<void()>_call):x(x),y(y),width(width),height(height),onClick(_call),R(R),G(G),B(B){}
    bool mouse_inside (double xm,double ym){
        return xm >= x && xm <= x + width && ym >= y && ym <= y + height;
    }
    void Draw(openGL_draw drawer,unsigned int dovien = 0,int vienR = 0, int vienG = 0,int vienB = 0){
        if(dovien>0){
            drawer.rectangle(
                    x-int(dovien),
                    y-int(dovien),
                    width+2*int(dovien),
                    height+2*int(dovien),
                    vienR,vienG,vienB
                             );
        }
        drawer.rectangle(x,y,width,height,R,G,B);
    }
};
std::vector<Button>button_list_menu = {
        Button(6,4,125,50,130, 127, 117,[](){
        std::cout<<"button SIU clicked. Happy birthay Messi"<<std::endl;
    })
};

void mouse_button_callback(GLFWwindow* window, int button,int action,int mods){
    if(button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)return;
    double x_mouse,y_mouse;
    glfwGetCursorPos(window,&x_mouse,&y_mouse);
    std::cout<<"Pressed x : "<<x_mouse<<" y : "<<y_mouse<<std::endl;
    if(scene == "Menu"){
        for(Button &b:button_list_menu){
            if(b.mouse_inside(x_mouse,y_mouse)){
                b.onClick();
            }
        }
    }
}
int main() {

    glfwInit();
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "FootballA", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window,mouse_button_callback);


    openGL_draw drawer;
    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.0f, 1.0f, 1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        for(Button &b:button_list_menu){
            b.Draw(drawer,4);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
