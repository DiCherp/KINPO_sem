#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>
#include <QSet>

void printError(const QString& message)
{
    QTextStream out(stdout);

    out << "Error: "
        << message
        << "\n";
}

bool hasAllowedExtension(
    const QString& path,
    const QSet<QString>& allowedExtensions)
{
    QString extension =
        QFileInfo(path)
            .suffix()
            .toLower();

    return allowedExtensions.contains(extension);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QTextStream out(stdout);

    if (argc != 4)
    {
        printError(
            "Usage: program code.c vars.txt output.txt"
        );

        return 1;
    }

    QString codeFilePath = argv[1];
    QString varsFilePath = argv[2];
    QString outputFilePath = argv[3];

    QSet<QString> codeExtensions =
    {
        "c",
        "cpp"
    };

    QSet<QString> textExtensions =
    {
        "txt"
    };

    if (!hasAllowedExtension(
            codeFilePath,
            codeExtensions))
    {
        printError(
            "Invalid code file extension"
        );

        return 1;
    }

    if (!hasAllowedExtension(
            varsFilePath,
            textExtensions))
    {
        printError(
            "Invalid variables file extension"
        );

        return 1;
    }

    out << "Files accepted\n\n";

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
