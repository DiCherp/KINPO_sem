#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>
#include <QSet>
#include <QRegularExpression>
#include <QFile>
#include <QtTest>

/**
 * @brief Выводит сообщение об ошибке в стандартный поток вывода (stdout).
 * В соответствии со спецификацией, сообщение выводится без префиксов
 * (например, без "Error: "), строго в том виде, в котором оно передано.
 * @param message Текст сообщения об ошибке из Таблицы 1.
 */
void printError(const QString& message)
{
    QTextStream out(stdout);
    out << message << "\n";
}

/**
 * @brief Читает файл с исходным кодом и проводит его проверку на соответствие ограничениям.
 * Накладывает следующие ограничения:
 * - Длина строки не превышает 256 символов.
 * - Отсутствуют директивы препроцессора #define.
 * - Общее количество строк не превышает 1000.
 * @param codePath Путь к проверяемому файлу (.c или .cpp).
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
 * @param path Путь к проверяемому файлу.
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
 * @param word Проверяемое слово (токен).
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
 * @param word Проверяемое слово (токен).
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
 * Имя должно начинаться с буквы или подчеркивания и содержать только буквы, цифры и подчеркивания.
 * @param name Имя переменной для проверки.
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
 * Накладывает ограничения: не более одного элемента в строке, имя не должно совпадать
 * с типами или ключевыми словами языка, отсутствие дубликатов, максимум 100 переменных.
 * @param varsPath Путь к текстовому файлу с переменными (.txt).
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
 * @param text Исходный код в виде единой строки.
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
 * Функция окружает пробелами все знаки препинания и математические операторы,
 * а затем разбивает получившийся текст по пробелам.
 * @param text Очищенный от комментариев исходный код.
 * @return QStringList Список извлеченных токенов (слов и символов).
 */
QStringList tokenizeCode(const QString& text)
{
    QString prepared = text;
    QString separators = "();,[]{}=+-*/<>!&|^";

    for (QChar ch : separators)
    {
        prepared.replace(QString(ch), " " + QString(ch) + " ");
    }

    prepared.replace("\n", " ");
    prepared.replace("\r", " ");

    return prepared.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
}

/**
 * @brief Пропускает сбалансированную группу скобок.
 * Функция ищет соответствующую закрывающую скобку для открывающей,
 * учитывая возможную вложенность таких же скобок внутри.
 * @param tokens Список токенов, полученный после лексического анализа.
 * @param index Индекс токена, с которого начинается чтение (ожидается открывающая скобка).
 * @param openTok Строковое представление открывающей скобки (например, "(" или "[").
 * @param closeTok Строковое представление закрывающей скобки (например, ")" или "]").
 * @return int Индекс следующего токена сразу после закрывающей скобки. Если закрывающая
 * скобка не найдена, возвращает размер списка токенов.
 */
int skipBalanced(const QStringList& tokens, int index, const QString& openTok, const QString& closeTok)
{
    if (index < 0 || index >= tokens.size()) {
        return tokens.size();
    }

    int balance = 0;

    for (int i = index; i < tokens.size(); ++i) {
        if (tokens[i] == openTok) {
            balance++;
        } else if (tokens[i] == closeTok) {
            balance--;
        }

        if (balance == 0) {
            return i + 1;
        }
    }

    return tokens.size();
}

/**
 * @brief Пропускает инициализирующее выражение после символа '='.
 * Функция перебирает токены, начиная с index. Она игнорирует содержимое
 * любых вложенных скобок (круглых, квадратных, фигурных), используя skipBalanced,
 * и останавливается только тогда, когда встречает запятую ',' или точку с запятой ';'
 * на базовом уровне вложенности.
 * @param tokens Список токенов.
 * @param index Индекс, с которого начинается чтение выражения.
 * @return int Индекс токена запятой или точки с запятой (или размер списка, если они не найдены).
 */
int skipInitializer(const QStringList& tokens, int index)
{
    int i = index;
    while (i < tokens.size())
    {
        if (tokens[i] == "(") {
            i = skipBalanced(tokens, i, "(", ")");
            continue;
        } else if (tokens[i] == "[") {
            i = skipBalanced(tokens, i, "[", "]");
            continue;
        } else if (tokens[i] == "{") {
            i = skipBalanced(tokens, i, "{", "}");
            continue;
        } else if (tokens[i] == ";" || tokens[i] == ",") {
            return i;
        }
        i++;
    }
    return tokens.size();
}

/**
 * @brief Ищет объявленные переменные в токенизированном исходном коде.
 * Функция проходит по списку токенов, находит объявления типов и извлекает
 * имена переменных, пропуская модификаторы типов, инициализаторы, размерности
 * массивов и определения структур/перечислений/объединений.
 * @param tokens Список токенов, полученный после лексического анализа.
 * @return QSet<QString> Множество уникальных имен объявленных переменных.
 */
QSet<QString> extractDeclaredVariables(const QStringList& tokens)
{
    QSet<QString> declared;
    int i = 0;

    while (i < tokens.size()) {
        if (isType(tokens[i])) {
            while (i < tokens.size() && isType(tokens[i])) {
                if (tokens[i] == "struct" || tokens[i] == "enum" || tokens[i] == "union") {
                    i++;
                    if (i < tokens.size() && isValidVariableName(tokens[i])) {
                        i++;
                    }
                    if (i < tokens.size() && tokens[i] == "{") {
                        i = skipBalanced(tokens, i, "{", "}");
                    }
                } else {
                    i++;
                }
            }

            while (i < tokens.size() && tokens[i] != ";") {
                if (isValidVariableName(tokens[i]) && !isType(tokens[i]) && !isKeyword(tokens[i])) {

                    if (i + 1 < tokens.size() && tokens[i + 1] == "(") {
                        i = skipBalanced(tokens, i + 1, "(", ")");
                        continue;
                    }

                    declared.insert(tokens[i]);
                    i++;

                    while (i < tokens.size() && tokens[i] == "[") {
                        i = skipBalanced(tokens, i, "[", "]");
                    }

                    if (i < tokens.size() && tokens[i] == "=") {
                        i = skipInitializer(tokens, i + 1);
                    }
                    if (i < tokens.size() && tokens[i] == ",") {
                        i++;
                    }
                } else {
                    i++;
                }
            }
        }
        i++;
    }

    return declared;
}

#ifndef RUN_TESTS

/**
 * @brief Главная функция приложения (точка входа).
 * Управляет процессом выполнения программы: считывает аргументы командной строки,
 * проверяет файлы, токенизирует код, выполняет поиск переменных и записывает
 * результаты в выходной файл согласно спецификации.
 * @param argc Количество переданных аргументов (ожидается 4).
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

    QSet<QString> declaredVars = extractDeclaredVariables(tokens);

    QList<QPair<QString, bool>> results;
    for (const QString& var : varsList) {
        bool found = declaredVars.contains(var);
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

#else
class TestAnalyzer : public QObject
{
    Q_OBJECT

private slots:
    void testIsType() {
        QCOMPARE(isType("int"), true);

        QCOMPARE(isType("const"), true);

        QCOMPARE(isType("value"), false);

        QCOMPARE(isType("count"), false);

        QCOMPARE(isType("float"), true);
        QCOMPARE(isType("char"), true);

        QCOMPARE(isType("double"), true);

        QCOMPARE(isType("signed"), true);

        QCOMPARE(isType("unsigned"), true);

        QCOMPARE(isType("struct"), true);

        QCOMPARE(isType("enum"), true);

        QCOMPARE(isType("union"), true);

        QCOMPARE(isType(""), false);

        QCOMPARE(isType("Float"), false);
    }

    void testIsKeyword() {
        QCOMPARE(isKeyword("while"), true);

        QCOMPARE(isKeyword("int"), false);

        QCOMPARE(isKeyword("counter"), false);

        QCOMPARE(isKeyword("var1"), false);

        QCOMPARE(isKeyword("_name"), false);

        QCOMPARE(isKeyword("Point"), false);
    }

    void testIsValidVariableName() {
        QCOMPARE(isValidVariableName("count"), true);

        QCOMPARE(isValidVariableName("1value"), false);

        QCOMPARE(isValidVariableName("var-name"), false);

        QCOMPARE(isValidVariableName("my var"), false);

        QCOMPARE(isValidVariableName("_count"), true);

        QCOMPARE(isValidVariableName("123"), false);

        QCOMPARE(isValidVariableName("a.b"), false);

        QCOMPARE(isValidVariableName("переменная"), false);

        QCOMPARE(isValidVariableName("x"), true);

        QCOMPARE(isValidVariableName(""), false);
    }

    void testRemoveComments() {
        QCOMPARE(removeComments("int a; // test"), QString("int a; "));

        QCOMPARE(removeComments("int a; /* test */\nint b;"), QString("int a; \nint b;"));

        QCOMPARE(removeComments("// comment"), QString(""));

        QCOMPARE(removeComments("int x = 5; // value"), QString("int x = 5; "));

        QCOMPARE(removeComments("int a; /*x\ny*/ int b;"), QString("int a;  int b;"));

        QCOMPARE(removeComments("// comment int a;"), QString(""));

        QCOMPARE(removeComments("int a = /*5*/ 4;"), QString("int a =  4;"));
    }

    void testTokenizeCode() {
        QStringList exp1 = {"int", "a", ";"};
        QCOMPARE(tokenizeCode("int a;"), exp1);

        QStringList exp2 = {"int", "x", "=", "5", ";"};
        QCOMPARE(tokenizeCode("int x = 5;"), exp2);

        QStringList exp3 = {"int", "arr", "[", "10", "]", ";"};
        QCOMPARE(tokenizeCode("int arr[10];"), exp3);

        QStringList exp4 = {"struct", "Point", "p", ";"};
        QCOMPARE(tokenizeCode("struct Point p;"), exp4);

        QStringList exp5 = {"signed", "char", "c", ";"};
        QCOMPARE(tokenizeCode("signed char c;"), exp5);

        QStringList exp6 = {"int", "x", "=", "a", "+", "2", ";"};
        QCOMPARE(tokenizeCode("int x = a + 2;"), exp6);

        QStringList exp7 = {"int", "a", ",", "b", ",", "c", ";"};
        QCOMPARE(tokenizeCode("int a, b, c;"), exp7);

        QStringList exp8 = {"int", "arr", "[", "3", "]", "=", "{", "1", ",", "2", ",", "3", "}", ";"};
        QCOMPARE(tokenizeCode("int arr[3] = {1,2,3};"), exp8);


        QStringList exp9 = {"void", "func", "(", ")", ";"};
        QCOMPARE(tokenizeCode("void func();"), exp9);
        QStringList exp10 = {"a1", "*", "(", "-", "2.5", "+", "b", ")", "^", "3", ";"};
        QCOMPARE(tokenizeCode("a1*(-2.5+b)^3;"), exp10);

        QStringList exp11 = {"a", ">", "=", "b", ";"};
        QCOMPARE(tokenizeCode("a>=b;"), exp11);

        QStringList exp12 = {"a", "&", "&", "b", ";"};
        QCOMPARE(tokenizeCode("a && b;"), exp12);
    }

    void testSkipBalanced() {
        QStringList tok1 = {"(", "a", ")"};
        QCOMPARE(skipBalanced(tok1, 0, "(", ")"), 3);

        QStringList tok2 = {"(", "(", "a", ")", ")"};
        QCOMPARE(skipBalanced(tok2, 0, "(", ")"), 5);

        QStringList tok3 = {"[", "10", "]"};
        QCOMPARE(skipBalanced(tok3, 0, "[", "]"), 3);
        QStringList tok4 = {"{", "a", "}", "b"};
        QCOMPARE(skipBalanced(tok4, 0, "{", "}"), 3);

        QStringList tok5 = {"(", "a", "+", "b", ")", "c"};
        QCOMPARE(skipBalanced(tok5, 0, "(", ")"), 5);

        QStringList tok6 = {"(", "a", "+", "b"};
        QCOMPARE(skipBalanced(tok6, 0, "(", ")"), 4); // размер списка = 4

        QStringList tok7 = {"(", "a", "[", "3", "]", ")", ";"};
        QCOMPARE(skipBalanced(tok7, 0, "(", ")"), 6);

        QStringList tok8 = {"("};
        QCOMPARE(skipBalanced(tok8, 0, "(", ")"), 1); // размер списка = 1

        QStringList tok9 = {"[", "a", "+", "1", "]", "b"};
        QCOMPARE(skipBalanced(tok9, 0, "[", "]"), 5);

        QStringList tok10 = {"x", "y", "(", "a", "+", "b", ")", "z"};
        QCOMPARE(skipBalanced(tok10, 2, "(", ")"), 7);
    }

    void testSkipInitializer() {
        QStringList tok1 = {"5", ";"};
        QCOMPARE(skipInitializer(tok1, 0), 1);

        QStringList tok2 = {"(", "1", "+", "2", ")", ";"};
        QCOMPARE(skipInitializer(tok2, 0), 5);

        QStringList tok3 = {"{", "1", ",", "2", "}", ";"};
        QCOMPARE(skipInitializer(tok3, 0), 5);

        QStringList tok4 = {"5", ",", "b", "]"};
        QCOMPARE(skipInitializer(tok4, 0), 1);

        QStringList tok5 = {";"};
        QCOMPARE(skipInitializer(tok5, 0), 0);

        QStringList tok6 = {"a", "&&", "b", ";"};
        QCOMPARE(skipInitializer(tok6, 0), 3);

        QStringList tok7 = {"{", "{", "1", "}", ",", "{", "2", "}", "}", ";"};
        QCOMPARE(skipInitializer(tok7, 0), 9);

        QStringList tok8 = {"a", "+", "(", "b", "*", "c", ")", ";"};
        QCOMPARE(skipInitializer(tok8, 0), 7);

        QStringList tok9 = {"5"};
        QCOMPARE(skipInitializer(tok9, 0), 1);

        QStringList tok10 = {"a", ">=", "b", ";"};
        QCOMPARE(skipInitializer(tok10, 0), 3);
    }

    void testExtractDeclaredVariables() {
        QStringList tok1 = {"int", "a", ";"};
        QCOMPARE(extractDeclaredVariables(tok1), QSet<QString>({"a"}));

        QStringList tok2 = {"int", "a", ",", "b", ";"};
        QCOMPARE(extractDeclaredVariables(tok2), QSet<QString>({"a", "b"}));

        QStringList tok3 = {"int", "arr", "[", "10", "]", ";"};
        QCOMPARE(extractDeclaredVariables(tok3), QSet<QString>({"arr"}));

        QStringList tok4 = {"struct", "Point", "p", ";"};
        QCOMPARE(extractDeclaredVariables(tok4), QSet<QString>({"p"}));

        QStringList tok5 = {"enum", "Color", "c", ";"};
        QCOMPARE(extractDeclaredVariables(tok5), QSet<QString>({"c"}));

        QStringList tok6 = {"union", "Data", "d", ";"};
        QCOMPARE(extractDeclaredVariables(tok6), QSet<QString>({"d"}));

        QStringList tok7 = {"const", "int", "value", ";"};
        QCOMPARE(extractDeclaredVariables(tok7), QSet<QString>({"value"}));

        QStringList tok8 = {"struct", "Point", "{", "int", "a", ";", "int", "y", ";", "}", "p", ";"};
        QCOMPARE(extractDeclaredVariables(tok8), QSet<QString>({"p"}));

        QStringList tok9 = {"unsigned", "long", "int", "x", ";"};
        QCOMPARE(extractDeclaredVariables(tok9), QSet<QString>({"x"}));
    }
};

QTEST_MAIN(TestAnalyzer)
#include "main.moc"

#endif
