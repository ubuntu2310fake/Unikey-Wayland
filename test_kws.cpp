#include <QGuiApplication>
#include <KWindowSystem/KWindowSystem>
#include <KWindowSystem/KWindowInfo>
#include <iostream>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    WId activeId = KWindowSystem::activeWindow();
    KWindowInfo info(activeId, NET::WMVisibleName, NET::WM2WindowClass);
    std::cout << "Active WId: " << activeId << std::endl;
    std::cout << "Class: " << info.windowClassName().toStdString() << std::endl;
    return 0;
}
