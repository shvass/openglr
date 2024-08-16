

#include <glad/glad.h>
#include <openglr.hpp>
#include <window/window.hpp>
#include <pch.hpp>



// log all input events to stdout
class inputLogger : public window::inputHandler, public window::layer{

public:

    void render() override{
        glClearColor(0.7f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    };

    void inputLoop(){
        while (display->run)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            processEvents();
        }
        
    };

    void cursorUpdate() override{
        // LOGV << "cursor pos: %lf %lf  delta: %lf %lf", posX, posY, dposX, dposY);
    };

    void keyStateUpdate(int key, bool down) override{
        // LOGV << "key %c was %s", key, (down) ? "pressed" : "released");
    };

    void mouseButtonUpdate(int key, bool down) override{
        // LOGV << "mouse key %s was %s", (key) ? "right" : "left", (down) ? "pressed" : "released");
    };

    void scrollUpdate() override{
        LOGV << "scroll: %lf  %lf  delta: %lf  %lf", scrollX, scrollY, dScrollX, dScrollY;
    };


    void resized() override{
        LOGV << "window size" << width  << " x " <<  height;
    };

    void dropInput(const char* path){
        LOGV << "dropped " << path;
    };

    void close() override {
        LOGV << "window closed";
    };
};  




int main(){

    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::debug, &consoleAppender);

    PLOGI << "starting openglr window creation test";

    inputLogger logger;
    window::windowConfig cfg{
        .width = 800,
        .height = 600,
        .title = "openglr",
        .setFullscreen = false,
        .handler = &logger,
    };


    window* win = new window(cfg);
    win->layers.push_back(&logger);
    logger.inputLoop();
    
    return 0;
}