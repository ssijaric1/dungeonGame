#include "Application.h"
#include <gui/WinMain.h>

int main(int argc, const char * argv[])
{
    Application app(argc, argv);
    app.init("EN"); // Using English for now
    return app.run();
}