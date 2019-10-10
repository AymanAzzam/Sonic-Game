#ifndef APPLICATION_HPP
#define APPLICATION_HPP

//Always include GLAD before GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_CXX11
#include <glm/glm.hpp>

#include <input/keyboard.hpp>
#include <Input/mouse.hpp>
#include <game/game_scene.hpp>

class Application {
private:
    GLFWwindow* window;
    GameScene* currentScene = nullptr;
    Keyboard* keyboard;
	Mouse* mouse;
  
	long long timestamp;
	bool pause;

    //Delete copy constructor and assignment operation
    Application(Application const &) = delete;
    Application & operator =(Application const &) = delete;

public:
    Application(const char* title, int width, int height, bool isFullscreen = false);
    ~Application();
    void setScene(GameScene* scene);
    Scene* getScene();
    void run();

    GLFWwindow* getWindow(){ return window; }
    glm::ivec2 getWindowSize() {
        glm::ivec2 size;
        glfwGetWindowSize(window, &size.x, &size.y);
        return size;
    }

    void createEventHandlers();

    Keyboard* getKeyBoard(){return keyboard;}
	Mouse* getMouse() { return mouse; }
};

#endif