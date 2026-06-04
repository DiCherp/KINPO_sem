#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTextStream out(stdout);

    if (argc != 4)
    {
        out << "Usage:\n";
        out << "program code.c vars.txt output.txt\n";

        return 1;
    }

    QString codeFilePath = argv[1];
    QString varsFilePath = argv[2];
    QString outputFilePath = argv[3];

    out << "Program started successfully\n\n";

    out << "Code file: "
        << codeFilePath
        << "\n";

    out << "Variables file: "
        << varsFilePath
        << "\n";

    out << "Output file: "
        << outputFilePath
        << "\n";

    return 0;
}
