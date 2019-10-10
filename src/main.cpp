#include <application.hpp>
#include "game/game_scene.hpp"


int main(int argc, char** argv)
{
    //Start an windowed OpenGL application with size 640x480 and title "Runner Game" 
    Application* app = new Application("Sonic", 840, 680, false);
    
    //Create a scene and set it as the main scene in the application
    GameScene* scene = new GameScene(app);
    app->setScene(scene);
    
    //Start the application loop
    app->run();

    //Remember to deallocate everything before closing
    delete scene;
    delete app;

    return 0;
}
