#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>
#include <QSet>

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
        out << "Usage:\n";
        out << "program code.c vars.txt output.txt\n";

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
        out << "Error: invalid code file extension\n";

        return 1;
    }

    if (!hasAllowedExtension(
            varsFilePath,
            textExtensions))
    {
        out << "Error: invalid variables file extension\n";

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
