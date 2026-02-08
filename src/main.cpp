#include "Application.h"
#include <gui/WinMain.h>

int main(int argc, const char* argv[])
{
    Application app(argc, argv);
    auto appProperties = app.getProperties();
    td::String trLang = appProperties->getValue("translation", "EN");
    app.init(trLang);
    int toRet = app.run();
    return toRet;
}