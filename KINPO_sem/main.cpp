#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>
#include <QSet>
#include <QRegularExpression>

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

bool isType(const QString& word)
{
    static QSet<QString> types =
    {
        "int",
        "float",
        "char",
        "double",
        "bool"
    };

    return types.contains(word);
}

bool isKeyword(const QString& word)
{
    static QSet<QString> keywords =
    {
        "if",
        "else",
        "while",
        "for",
        "return",
        "switch",
        "case",
        "break",
        "continue"
    };

    return keywords.contains(word);
}

bool isValidVariableName(const QString& name)
{
    QRegularExpression regex(
        "^[a-zA-Z_][a-zA-Z0-9_]*$"
    );

    return regex.match(name).hasMatch();
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

    out << "\nValidation module loaded\n";

    return 0;
}
