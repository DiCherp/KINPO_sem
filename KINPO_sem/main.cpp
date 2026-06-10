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

bool readCodeFile(
    const QString& codePath,
    QStringList& lines)
{
    QFile file(codePath);

    if (!file.open(
            QIODevice::ReadOnly |
            QIODevice::Text))
    {
        printError(
            "Cannot open code file"
        );

        return false;
    }

    QTextStream stream(&file);

    int lineCount = 0;

    while (!stream.atEnd())
    {
        QString line =
            stream.readLine();

        if (line.length() > 256)
        {
            printError(
                "Line length exceeds 256 symbols"
            );

            return false;
        }

        if (line.contains("#define"))
        {
            printError(
                "Source code contains #define"
            );

            return false;
        }

        lines.append(line);

        lineCount++;

        if (lineCount > 1000)
        {
            printError(
                "File contains more than 1000 lines"
            );

            return false;
        }
    }

    if (lines.isEmpty())
    {
        printError(
            "Code file is empty"
        );

        return false;
    }

    return true;
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

bool readVariablesFile(
    const QString& varsPath,
    QStringList& varsList)
{
    QFile file(varsPath);

    if (!file.open(
            QIODevice::ReadOnly |
            QIODevice::Text))
    {
        printError(
            "Cannot open variables file"
        );

        return false;
    }

    QTextStream stream(&file);

    QSet<QString> uniqueVars;

    while (!stream.atEnd())
    {
        QString var =
            stream.readLine().trimmed();

        if (var.isEmpty())
        {
            printError(
                "Empty line found in variables file"
            );

            return false;
        }

        if (!isValidVariableName(var))
        {
            printError(
                "Invalid variable name: " + var
            );

            return false;
        }

        if (isType(var))
        {
            printError(
                "Variable name matches type: " + var
            );

            return false;
        }

        if (isKeyword(var))
        {
            printError(
                "Variable name matches keyword: " + var
            );

            return false;
        }

        if (uniqueVars.contains(var))
        {
            printError(
                "Duplicate variable: " + var
            );

            return false;
        }

        uniqueVars.insert(var);

        varsList.append(var);

        if (varsList.size() > 100)
        {
            printError(
                "Too many variables"
            );

            return false;
        }
    }

    if (varsList.isEmpty())
    {
        printError(
            "Variables file is empty"
        );

        return false;
    }

    return true;
}

QString removeComments(const QString& text)
{
    QString result;

    bool inSingleComment = false;
    bool inMultiComment = false;

    for (int i = 0; i < text.length(); i++)
    {
        if (!inSingleComment &&
            !inMultiComment &&
            i + 1 < text.length() &&
            text[i] == '/' &&
            text[i + 1] == '/')
        {
            inSingleComment = true;
            i++;

            continue;
        }

        if (!inSingleComment &&
            !inMultiComment &&
            i + 1 < text.length() &&
            text[i] == '/' &&
            text[i + 1] == '*')
        {
            inMultiComment = true;
            i++;

            continue;
        }

        if (inSingleComment &&
            text[i] == '\n')
        {
            inSingleComment = false;

            result += '\n';

            continue;
        }

        if (inMultiComment &&
            i + 1 < text.length() &&
            text[i] == '*' &&
            text[i + 1] == '/')
        {
            inMultiComment = false;
            i++;

            continue;
        }

        if (!inSingleComment &&
            !inMultiComment)
        {
            result += text[i];
        }
    }

    return result;
}

QStringList tokenizeCode(const QString& text)
{
    QString prepared = text;

    QString separators =
        "();,[]{}=+-*/<>!&|";

    for (QChar ch : separators)
    {
        prepared.replace(
            QString(ch),
            " " + QString(ch) + " "
        );
    }

    return prepared.split(
        QRegularExpression("\\s+"),
        QString::SkipEmptyParts
    );
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

    QFile outFile(outputFilePath);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            printError("Неверно указан файл для выходных данных. Возможно, указанного расположения не существует или нет прав на запись.");
            return 1;
        }

        // Закрываем файл, так как запись будет реализована в следующих коммитах
        outFile.close();

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

    QStringList codeLines;

    if (!readCodeFile(
            codeFilePath,
            codeLines))
    {
        return 1;
    }

    QStringList varsList;

    if (!readVariablesFile(
            varsFilePath,
            varsList))
    {
        return 1;
    }

    QString codeText =
        codeLines.join("\n");

    codeText =
        removeComments(codeText);

    QStringList tokens =
        tokenizeCode(codeText);

    QList<QPair<QString, bool>> results;

    for (const QString& var : varsList) {
        // Согласно спецификации, проверяем, есть ли переменная в коде
        bool found = tokens.contains(var);
        results.append(qMakePair(var, found));
    }

    out << "Code file loaded\n";
    out << "Lines count: "
        << codeLines.size()
        << "\n";

    out << "Variables loaded: "
        << varsList.size()
        << "\n\n";

    out << "\nTokens count: "
        << tokens.size()
        << "\n";

    return 0;
}
