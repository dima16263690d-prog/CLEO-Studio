#include "main_window.hpp"
#include <QApplication>
#include <QFont>

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    QCoreApplication::setApplicationName("CLEO Studio");
    QCoreApplication::setApplicationVersion("0.1.0-dev");
    QCoreApplication::setOrganizationName("CLEO Studio");
    QFont applicationFont("Segoe UI", 10);
    application.setFont(applicationFont);
    MainWindow window;
    window.show();
    return application.exec();
}
