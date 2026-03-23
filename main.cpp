#include <iostream>
#include <windows.h> // 用于控制台乱码修复

#include "VulkanLearnApplication.h"

int main() {
    SetConsoleOutputCP(65001); // 修复控制台乱码

    VulkanLearnApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}