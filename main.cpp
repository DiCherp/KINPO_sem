#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTextStream out(stdout);

    out << "Program for checking variable declarations in C code\n";
    out << "Project initialized successfully\n";

    return 0;
}
