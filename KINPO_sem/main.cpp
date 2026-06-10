#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>
#include <QSet>
#include <QRegularExpression>
#include <QFile>

//вывод ошибки
void printError(const QString& message)
{
    QTextStream out(stdout);
    out << message << "\n";
}

bool readCodeFile(const QString& codePath, QStringList& lines)
{
    QFile file(codePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printError("Неверно указан файл с входными данными. Возможно, файл не существует");
        return false;
    }

    QTextStream stream(&file);
    int lineCount = 0;

    while (!stream.atEnd())
    {
        QString line = stream.readLine();

        if (line.length() > 256)
        {
            printError("Строка во входном файле .c не должна превышать 256 символов");
            return false;
        }

        if (line.contains("#define"))
        {
            printError("Код программы не должен содержать в себе define");
            return false;
        }

        lines.append(line);
        lineCount++;

        if (lineCount > 1000)
        {
            printError("Входной файл с кодом не должен содержать более 1000 строк");
            return false;
        }
    }

    if (lines.isEmpty())
    {
        printError("Один или несколько файлов пустые");
        return false;
    }

    return true;
}

bool hasAllowedExtension(const QString& path, const QSet<QString>& allowedExtensions)
{
    QString extension = QFileInfo(path).suffix().toLower();
    return allowedExtensions.contains(extension);
}

bool isType(const QString& word)
{
    static QSet<QString> types = {
        "int", "float", "char", "double", "bool", "long", "short", "signed", "unsigned", "const", "struct", "enum", "union"
    };
    return types.contains(word);
}

bool isKeyword(const QString& word)
{
    static QSet<QString> keywords = {
        "if", "else", "while", "for", "return", "switch", "case", "break", "continue"
    };
    return keywords.contains(word);
}

bool isValidVariableName(const QString& name)
{
    // Имя переменной должно начинаться с буквы или _, далее буквы, цифры или _
    QRegularExpression regex("^[a-zA-Z_][a-zA-Z0-9_]*$");
    return regex.match(name).hasMatch();
}

bool readVariablesFile(const QString& varsPath, QStringList& varsList)
{
    QFile file(varsPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        printError("Неверно указан файл с входными данными. Возможно, файл не существует");
        return false;
    }

    QTextStream stream(&file);
    QSet<QString> uniqueVars;

    while (!stream.atEnd())
    {
        QString varLine = stream.readLine().trimmed();

        if (varLine.isEmpty())
        {
            continue;
        }

        QStringList parts = varLine.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
        if (parts.size() > 1) {
             printError("Во входной строке слишком много элементов. Введите только имя переменной.");
             return false;
        }

        QString var = parts[0];

        //проверка первого символа на цифру
        if (var.length() > 0 && var[0].isDigit()) {
            printError(QString("Имя переменной не должно начинаться с цифры, а начинается с «%1»").arg(var[0]));
            return false;
        }

        if (!isValidVariableName(var))
        {
            //находим первый недопустимый символ для сообщения об ошибке
            QRegularExpression invalidCharRegex("[^a-zA-Z0-9_]");
            QRegularExpressionMatch match = invalidCharRegex.match(var);
            QString badChar = match.hasMatch() ? match.captured(0) : "?";

            printError(QString("Имя переменной не должно содержать недопустимые символы, а содержится «%1»").arg(badChar));
            return false;
        }

        if (isType(var) || isKeyword(var))
        {
            printError("Имя переменной не должно совпадать с названием оператора");
            return false;
        }

        if (uniqueVars.contains(var))
        {
            printError(QString("Имена переменных не должны дублироваться, а дублируется переменная «%1»").arg(var));
            return false;
        }

        uniqueVars.insert(var);
        varsList.append(var);

        if (varsList.size() > 100)
        {
            printError("Текстовый файл не должен содержать более 100 переменных для проверки");
            return false;
        }
    }

    if (varsList.isEmpty())
    {
        printError("Один или несколько файлов пустые");
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
        if (!inSingleComment && !inMultiComment && i + 1 < text.length() && text[i] == '/' && text[i + 1] == '/')
        {
            inSingleComment = true;
            i++;
            continue;
        }

        if (!inSingleComment && !inMultiComment && i + 1 < text.length() && text[i] == '/' && text[i + 1] == '*')
        {
            inMultiComment = true;
            i++;
            continue;
        }

        if (inSingleComment && text[i] == '\n')
        {
            inSingleComment = false;
            result += '\n';
            continue;
        }

        if (inMultiComment && i + 1 < text.length() && text[i] == '*' && text[i + 1] == '/')
        {
            inMultiComment = false;
            i++;
            continue;
        }

        if (!inSingleComment && !inMultiComment)
        {
            result += text[i];
        }
    }

    return result;
}

QStringList tokenizeCode(const QString& text)
{
    QString prepared = text;
    QString separators = "();,[]{}=+-*/<>!&|";

    for (QChar ch : separators)
    {
        prepared.replace(QString(ch), " " + QString(ch) + " ");
    }

    //заменяем переносы строк на пробелы для надежной токенизации
    prepared.replace("\n", " ");
    prepared.replace("\r", " ");

    return prepared.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    //Проверка аргументов
    if (argc != 4)
    {
        printError("Форматы файлов не соответствуют требуемым");
        return 1;
    }

    QString codeFilePath = argv[1];
    QString varsFilePath = argv[2];
    QString outputFilePath = argv[3];

    QSet<QString> codeExtensions = {"c", "cpp"};
    QSet<QString> textExtensions = {"txt"};

    if (!hasAllowedExtension(codeFilePath, codeExtensions) || !hasAllowedExtension(varsFilePath, textExtensions))
    {
        printError("Форматы файлов не соответствуют требуемым");
        return 1;
    }

    //Чтение и валидация исходного кода
    QStringList codeLines;
    if (!readCodeFile(codeFilePath, codeLines))
    {
        return 1;
    }

    //Чтение и валидация файла с переменными
    QStringList varsList;
    if (!readVariablesFile(varsFilePath, varsList))
    {
        return 1;
    }

    //Обработка кода (удаление комментариев и токенизация)
    QString codeText = codeLines.join("\n");
    codeText = removeComments(codeText);

    //Проверка на анонимные структуры (согласно спецификации)
    if (codeText.contains(QRegularExpression("struct\\s*\\{"))) {
        printError("Анонимные структуры не поддерживаются");
        return 1;
    }

    QStringList tokens = tokenizeCode(codeText);

    //Алгоритм проверки наличия переменных
    QList<QPair<QString, bool>> results;
    for (const QString& var : varsList) {
        bool found = tokens.contains(var);
        results.append(qMakePair(var, found));
    }

    //Запись результатов в выходной файл
    QFile outFile(outputFilePath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        printError("Неверно указан файл для выходных данных. Возможно, указанного расположения не существует или нет прав на запись.");
        return 1;
    }

    QTextStream outStream(&outFile);
    for (const auto& pair : results) {
        outStream << pair.first << (pair.second ? " true" : " false") << "\n";
    }

    outFile.close();

    return 0;
}
