#include "SceneManager.h"

int main() {
    std::string name{"Graphics Game Engine"};
    auto *gsm = new SceneManager(RendererType::VULKAN);
    if (gsm->Initialize(name, 1280, 720) == true) {
        gsm->Run();
    }
    delete gsm;
    exit(0);
}
