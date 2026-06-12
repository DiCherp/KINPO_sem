#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>
#include <QSet>
#include <QRegularExpression>
#include <QFile>

/**
 * @brief Выводит сообщение об ошибке в стандартный поток вывода (stdout).
 * * В соответствии со спецификацией, сообщение выводится без префиксов
 * (например, без "Error: "), строго в том виде, в котором оно передано.
 * * @param message Текст сообщения об ошибке из Таблицы 1.
 */
void printError(const QString& message)
{
    QTextStream out(stdout);
    out << message << "\n";
}

/**
 * @brief Читает файл с исходным кодом и проводит его проверку на соответствие ограничениям.
 * * Накладывает следующие ограничения:
 * - Длина строки не превышает 256 символов.
 * - Отсутствуют директивы препроцессора #define.
 * - Общее количество строк не превышает 1000.
 * * @param codePath Путь к проверяемому файлу (.c или .cpp).
 * @param lines Ссылка на список строк (QStringList), куда будет сохранен код.
 * @return true Если файл успешно прочитан и прошел все проверки спецификации.
 * @return false Если файл не удалось открыть или нарушено одно из ограничений.
 */
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

/**
 * @brief Проверяет наличие разрешенного расширения у файла.
 * * @param path Путь к проверяемому файлу.
 * @param allowedExtensions Набор (QSet) допустимых расширений (например, "c", "cpp", "txt").
 * @return true Если расширение файла присутствует в списке допустимых.
 * @return false Если файл имеет недопустимое расширение.
 */
bool hasAllowedExtension(const QString& path, const QSet<QString>& allowedExtensions)
{
    QString extension = QFileInfo(path).suffix().toLower();
    return allowedExtensions.contains(extension);
}

/**
 * @brief Проверяет, является ли переданное слово базовым или пользовательским типом данных C/C++.
 * * @param word Проверяемое слово (токен).
 * @return true Если слово является типом (int, float, struct, const и т.д.).
 * @return false Если слово не является типом.
 */
bool isType(const QString& word)
{
    static QSet<QString> types = {
        "int", "float", "char", "double", "bool", "long", "short", "signed", "unsigned", "const", "struct", "enum", "union"
    };
    return types.contains(word);
}

/**
 * @brief Проверяет, является ли переданное слово ключевым словом (оператором) языка C/C++.
 * * @param word Проверяемое слово (токен).
 * @return true Если слово является ключевым словом (if, while, break и т.д.).
 * @return false Если слово не является ключевым словом.
 */
bool isKeyword(const QString& word)
{
    static QSet<QString> keywords = {
        "if", "else", "while", "for", "return", "switch", "case", "break", "continue"
    };
    return keywords.contains(word);
}

/**
 * @brief Проверяет, соответствует ли имя переменной синтаксическим правилам языка C.
 * * Имя должно начинаться с буквы или подчеркивания и содержать только буквы, цифры и подчеркивания.
 * * @param name Имя переменной для проверки.
 * @return true Если имя синтаксически корректно.
 * @return false Если имя содержит недопустимые символы или начинается с цифры.
 */
bool isValidVariableName(const QString& name)
{
    QRegularExpression regex("^[a-zA-Z_][a-zA-Z0-9_]*$");
    return regex.match(name).hasMatch();
}

/**
 * @brief Читает файл со списком переменных и проверяет их на корректность.
 * * Накладывает ограничения: не более одного элемента в строке, имя не должно совпадать
 * с типами или ключевыми словами языка, отсутствие дубликатов, максимум 100 переменных.
 * * @param varsPath Путь к текстовому файлу с переменными (.txt).
 * @param varsList Ссылка на список (QStringList), в который будут сохранены валидные имена переменных.
 * @return true Если файл успешно прочитан и все переменные корректны.
 * @return false В случае ошибки доступа к файлу или нарушения ограничений спецификации.
 */
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

        if (var.length() > 0 && var[0].isDigit()) {
            printError(QString("Имя переменной не должно начинаться с цифры, а начинается с «%1»").arg(var[0]));
            return false;
        }

        if (!isValidVariableName(var))
        {
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

/**
 * @brief Очищает исходный текст кода от однострочных и многострочных комментариев C/C++.
 * * @param text Исходный код в виде единой строки.
 * @return QString Код без комментариев, готовый к токенизации.
 */
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

/**
 * @brief Выполняет лексический анализ, разбивая очищенный исходный код на токены.
 * * Функция окружает пробелами все знаки препинания и математические операторы,
 * а затем разбивает получившийся текст по пробелам.
 * * @param text Очищенный от комментариев исходный код.
 * @return QStringList Список извлеченных токенов (слов и символов).
 */
QStringList tokenizeCode(const QString& text)
{
    QString prepared = text;
    QString separators = "();,[]{}=+-*/<>!&|";

    for (QChar ch : separators)
    {
        prepared.replace(QString(ch), " " + QString(ch) + " ");
    }

    prepared.replace("\n", " ");
    prepared.replace("\r", " ");

    return prepared.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
}

/**
 * @brief Главная функция приложения (точка входа).
 * * Управляет процессом выполнения программы: считывает аргументы командной строки,
 * проверяет файлы, токенизирует код, выполняет поиск переменных и записывает
 * результаты в выходной файл согласно спецификации.
 * * @param argc Количество переданных аргументов (ожидается 4).
 * @param argv Массив переданных аргументов командной строки.
 * @return int 0 в случае успешного завершения, 1 в случае ошибки.
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

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

    QStringList codeLines;
    if (!readCodeFile(codeFilePath, codeLines))
    {
        return 1;
    }

    QStringList varsList;
    if (!readVariablesFile(varsFilePath, varsList))
    {
        return 1;
    }

    QString codeText = codeLines.join("\n");
    codeText = removeComments(codeText);

    if (codeText.contains(QRegularExpression("struct\\s*\\{"))) {
        printError("Анонимные структуры не поддерживаются");
        return 1;
    }

    QStringList tokens = tokenizeCode(codeText);

    QList<QPair<QString, bool>> results;
    for (const QString& var : varsList) {
        bool found = tokens.contains(var);
        results.append(qMakePair(var, found));
    }

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
