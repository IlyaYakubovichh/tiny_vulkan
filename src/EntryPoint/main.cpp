#include "Application.h"

int main()
{
    tiny_vulkan::ApplicationSpec appSpec;
    appSpec.windowWidth = 1280;
    appSpec.windowHeight = 720;
    appSpec.windowName = "TinyVulkan";

    tiny_vulkan::Application application(appSpec);
    application.Run();

    return 0;
}