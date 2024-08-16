// This file is a part of  openglr
// Copyright (C) 2023  akshay bansod <akshayb@gmx.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.



#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "window.hpp"
#define glsl_version "#version 330 core"

// definition of static members
window::inputHandler window::defaultInputHandler;
window::windowConfig window::defaultWindowConfig;
bool isGladInited = false,  isGlfwInited = false, isImguiInited=false;

window::window(windowConfig& cfg){

    if(!isGlfwInited) isGlfwInited = glfwInit();

    // setup monitor
    int monitorCount;
    GLFWmonitor* targetMon = glfwGetMonitors(&monitorCount)
        [ (cfg.monitorIndex < monitorCount) ? cfg.monitorIndex : 0 ];

    // use display resoulution for height and width
    if(!cfg.height || !cfg.width)
        glfwGetMonitorWorkarea(targetMon, NULL, NULL, &cfg.width, &cfg.height);

    // window creation hints
    glfwWindowHint(GLFW_RESIZABLE, cfg.resizable);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    

    // create window
    m_win = glfwCreateWindow(
        m_width = cfg.width,
        m_height = cfg.height,
        cfg.title, 
        (cfg.setFullscreen) ? targetMon : NULL,
        NULL
    );

    LOGI << "created window with title " << cfg.title;
    
    // setup input callbacks
    switchHandler(cfg.handler);
    glfwSetKeyCallback(m_win, glfwKeyCallback);
    glfwSetDropCallback(m_win, glfwDropCallback);
    glfwSetWindowSizeCallback(m_win, glfwResizeCallback);
    glfwSetCursorPosCallback(m_win, glfwCursorPosCallback);
    glfwSetWindowCloseCallback(m_win, glfwWindowCloseCallback);
    glfwSetMouseButtonCallback(m_win, glfwMouseButtonCallback);
    glfwSetScrollCallback(m_win, glfwScrollCallback);

    // switch context
    GLFWwindow* prevCtx = glfwGetCurrentContext();
    // spawn renderThread
    renderThrd = std::thread(&window::renderLoop, this);
}


window::inputHandler* window::switchHandler(inputHandler* handler){

    if(!handler) return nullptr;
    handler->display = this;
    inputHandler* prev = m_handler;
    m_handler = handler;
    glfwSetWindowUserPointer(m_win, handler);
    return prev;
};

void window::setActive(){
    glfwMakeContextCurrent(m_win);
}

void window::resize(int width, int height){
    // check if width and height are not zero else use current dimension and update the dimension as well
    glfwSetWindowSize(m_win,  m_width = (width) ? width : m_width, m_height = (height) ? height : m_height);
}

void window::toggleCursor()
{
    glfwSetInputMode(m_win, GLFW_CURSOR, (cursorEnable) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    cursorEnable = !cursorEnable;
    LOGI << "cursor toggle called. Current cursor state " << cursorEnable;
}

window::~window()
{
    glfwDestroyWindow(m_win);
};

// window::inputHandler methods
void window::inputHandler::processEvents(){
    glfwWaitEvents();
};


// static window callbacks
void window::glfwCursorPosCallback(GLFWwindow* win, double x, double y){
    inputHandler& handler = *(inputHandler*) glfwGetWindowUserPointer(win);

    // update handler dataset
    handler.dposX = x - handler.posX;
    handler.dposY = y - handler.posY;
    handler.posX = x;
    handler.posY = y;

    handler.cursorUpdate();
};

void window::glfwKeyCallback(GLFWwindow* win, int key, int scancode, int action, int mods){
    if(action == GLFW_REPEAT) return;

    inputHandler& handler = *(inputHandler*) glfwGetWindowUserPointer(win);
    handler.keyStateUpdate(key, action);
};

void window::glfwWindowCloseCallback(GLFWwindow* win){
    inputHandler& handler = *(inputHandler*) glfwGetWindowUserPointer(win);
    handler.display->run = false;
    handler.close();
};

void window::glfwResizeCallback(GLFWwindow* win, int width, int height){
    inputHandler& handler = *(inputHandler*) glfwGetWindowUserPointer(win);
    handler.width = width;
    handler.height = height;

    handler.resized();
};

void window::glfwMouseButtonCallback(GLFWwindow* win, int button, int action, int mods){

    inputHandler& handler = *(inputHandler*) glfwGetWindowUserPointer(win);
    handler.mouseButtonUpdate(button, action);
};

void window::glfwDropCallback(GLFWwindow* win, int path_count, const char* paths[]){
    inputHandler& handler = *(inputHandler*) glfwGetWindowUserPointer(win);

    for(int i=0; i < path_count; i++){
        handler.dropInput(paths[i]);
    };
};

void window::glfwScrollCallback(GLFWwindow* win, double dx, double dy){
    inputHandler& handler = *(inputHandler*) glfwGetWindowUserPointer(win);

    handler.dScrollX = dx;
    handler.dScrollY = dy;
    handler.scrollX += dx;
    handler.scrollY += dy;

    handler.scrollUpdate();
};



using namespace std::chrono;

void window::renderLoop(){

    LOGD << "render thread started";
    setActive();

    // initialize imgui
    if(!isImguiInited){
        isImguiInited=true;
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    };
    
    LOGE_IF(!ImGui_ImplGlfw_InitForOpenGL(m_win, true) || !ImGui_ImplOpenGL3_Init(glsl_version)) << "imgui initialization failed";

    // initialize glad
    if(!isGladInited) isGladInited = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    LOGE_IF(!isGladInited) << "glad initialization failed"; 


    milliseconds lastTick =  duration_cast<milliseconds>(system_clock::now().time_since_epoch()), now;
    std::list<layer*>::reverse_iterator pointer;
    int  delay;

    while (run)
    {
        // render all layers in reverse order
        for(pointer = layers.rbegin(); pointer != layers.rend(); pointer++){
            (*pointer)->render();
        }

        glfwSwapBuffers(m_win);

        now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        delay = minFrameDelay - (now - lastTick).count();
        if(delay > 0.0f) std::this_thread::sleep_for(milliseconds(delay));
        lastTick = now;
    }
};
