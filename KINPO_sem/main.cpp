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
            // Пропускаем все модификаторы и составные типы (например, unsigned long int, struct Point)
            while (i < tokens.size() && isType(tokens[i])) {
                if (tokens[i] == "struct" || tokens[i] == "enum" || tokens[i] == "union") {
                    i++; // Пропускаем ключевое слово
                    if (i < tokens.size() && isValidVariableName(tokens[i])) {
                        i++; // Пропускаем имя тега (например, Point)
                    }
                    // Если сразу идет определение структуры {...}
                    if (i < tokens.size() && tokens[i] == "{") {
                        i = skipBalanced(tokens, i, "{", "}");
                    }
                } else {
                    i++; // Пропускаем обычный тип
                }
            }

            // Теперь извлекаем имена переменных до конца выражения (до точки с запятой)
            while (i < tokens.size() && tokens[i] != ";") {
                if (isValidVariableName(tokens[i]) && !isType(tokens[i]) && !isKeyword(tokens[i])) {

                    // Проверка: не функция ли это? (в случае если токен это имя функции)
                    if (i + 1 < tokens.size() && tokens[i + 1] == "(") {
                        i = skipBalanced(tokens, i + 1, "(", ")");
                        continue;
                    }

                    // Добавляем переменную в множество
                    declared.insert(tokens[i]);
                    i++;

                    // Пропускаем размерности массивов
                    while (i < tokens.size() && tokens[i] == "[") {
                        i = skipBalanced(tokens, i, "[", "]");
                    }

                    // Пропускаем инициализаторы
                    if (i < tokens.size() && tokens[i] == "=") {
                        i = skipInitializer(tokens, i + 1);
                    }

                    // Если после переменной идет запятая, пропускаем ее к следующей переменной
                    if (i < tokens.size() && tokens[i] == ",") {
                        i++;
                    }
                } else {
                    i++; // Пропускаем неожиданные токены (например, *, &)
                }
            }
        }
        i++; // Переходим к следующему токену или перешагиваем ';'
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

    // 1. Получаем список всех токенов из кода
    QStringList tokens = tokenizeCode(codeText);

    // 2. Ищем все действительно ОБЪЯВЛЕННЫЕ переменные
    QSet<QString> declaredVars = extractDeclaredVariables(tokens);

    // 3. Формируем результаты
    QList<QPair<QString, bool>> results;
    for (const QString& var : varsList) {
        // Теперь мы проверяем наличие в множестве объявленных переменных, а не просто среди токенов
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
        // 1. Проверка встроенного типа
        QCOMPARE(isType("int"), true);

        // 2. Проверка модификатора типа
        QCOMPARE(isType("const"), true);

        // 3. Проверка пользовательского слова
        QCOMPARE(isType("value"), false);

        // 4. Проверка пользовательского идентификатора
        QCOMPARE(isType("count"), false);

        // 5. Проверка float
        QCOMPARE(isType("float"), true);

        // 6. Проверка char
        QCOMPARE(isType("char"), true);

        // 7. Проверка double
        QCOMPARE(isType("double"), true);

        // 8. Проверка signed
        QCOMPARE(isType("signed"), true);

        // 9. Проверка unsigned
        QCOMPARE(isType("unsigned"), true);

        // 10. Проверка struct
        QCOMPARE(isType("struct"), true);

        // 11. Проверка enum
        QCOMPARE(isType("enum"), true);

        // 12. Проверка union
        QCOMPARE(isType("union"), true);

        // 13. Проверка пустой строки
        QCOMPARE(isType(""), false);

        // 14. Проверка встроенного типа с заглавной буквы
        QCOMPARE(isType("Float"), false);
    }

    // Тесты для функции isKeyword (Проверка, является ли слово ключевым словом языка С)
    void testIsKeyword() {
        // 1. Проверка ключевого слова
        QCOMPARE(isKeyword("while"), true);

        // 2. Проверка типа как ключевого слова
        QCOMPARE(isKeyword("int"), false);

        // 3. Проверка обычного идентификатора
        QCOMPARE(isKeyword("counter"), false);

        // 4. Проверка идентификатора с цифрой
        QCOMPARE(isKeyword("var1"), false);

        // 5. Проверка идентификатора с подчёркиванием
        QCOMPARE(isKeyword("_name"), false);

        // 6. Проверка пользовательского имени
        QCOMPARE(isKeyword("Point"), false);
    }

    // Тесты для функции isValidVariableName (Проверка корректности имени переменной)
    void testIsValidVariableName() {
        // 1. Корректное имя
        QCOMPARE(isValidVariableName("count"), true);

        // 2. Имя начинается с цифры
        QCOMPARE(isValidVariableName("1value"), false);

        // 3. Имя содержит недопустимый символ
        QCOMPARE(isValidVariableName("var-name"), false);

        // 4. Имя содержит пробел
        QCOMPARE(isValidVariableName("my var"), false);

        // 5. Имя с подчёркиванием в начале
        // ВНИМАНИЕ: Функция вернет true, так как С это разрешает. Тест упадет.
        QCOMPARE(isValidVariableName("_count"), true);

        // 6. Только цифры
        QCOMPARE(isValidVariableName("123"), false);

        // 7. Имя с точкой
        QCOMPARE(isValidVariableName("a.b"), false);

        // 8. Имя с кириллицей
        QCOMPARE(isValidVariableName("переменная"), false);

        // 9. Однобуквенное имя
        QCOMPARE(isValidVariableName("x"), true);

        // 10. Пустая строка
        QCOMPARE(isValidVariableName(""), false);
    }

    // Тесты для функции removeComments (Удаление однострочных и многострочных комментариев)
    void testRemoveComments() {
        // 1. Удаление однострочного комментария
        // (Остается пробел перед началом комментария)
        QCOMPARE(removeComments("int a; // test"), QString("int a; "));

        // 2. Удаление многострочного комментария (с переносом строки)
        QCOMPARE(removeComments("int a; /* test */\nint b;"), QString("int a; \nint b;"));

        // 3. Комментарий в отдельной строке
        QCOMPARE(removeComments("// comment"), QString(""));

        // 4. Комментарий после инициализации
        QCOMPARE(removeComments("int x = 5; // value"), QString("int x = 5; "));

        // 5. Многострочный комментарий на нескольких строках
        QCOMPARE(removeComments("int a; /*x\ny*/ int b;"), QString("int a;  int b;"));

        // 6. Комментарий в начале строки
        QCOMPARE(removeComments("// comment int a;"), QString(""));

        // 7. Комментарий посреди инициализации
        QCOMPARE(removeComments("int a = /*5*/ 4;"), QString("int a =  4;"));
    }

    // Тесты для функции tokenizeCode (Разбиение строки исходного кода на токены)
    void testTokenizeCode() {
        // 1. Простое объявление
        QStringList exp1 = {"int", "a", ";"};
        QCOMPARE(tokenizeCode("int a;"), exp1);

        // 2. Объявление с инициализацией
        QStringList exp2 = {"int", "x", "=", "5", ";"};
        QCOMPARE(tokenizeCode("int x = 5;"), exp2);

        // 3. Массив
        QStringList exp3 = {"int", "arr", "[", "10", "]", ";"};
        QCOMPARE(tokenizeCode("int arr[10];"), exp3);

        // 4. Объявление структуры
        QStringList exp4 = {"struct", "Point", "p", ";"};
        QCOMPARE(tokenizeCode("struct Point p;"), exp4);

        // 5. Объявление signed переменной
        QStringList exp5 = {"signed", "char", "c", ";"};
        QCOMPARE(tokenizeCode("signed char c;"), exp5);

        // 6. Инициализация с выражением
        QStringList exp6 = {"int", "x", "=", "a", "+", "2", ";"};
        QCOMPARE(tokenizeCode("int x = a + 2;"), exp6);

        // 7. Несколько переменных
        QStringList exp7 = {"int", "a", ",", "b", ",", "c", ";"};
        QCOMPARE(tokenizeCode("int a, b, c;"), exp7);

        // 8. Массив с инициализацией
        QStringList exp8 = {"int", "arr", "[", "3", "]", "=", "{", "1", ",", "2", ",", "3", "}", ";"};
        QCOMPARE(tokenizeCode("int arr[3] = {1,2,3};"), exp8);

        // 9. Функция
        QStringList exp9 = {"void", "func", "(", ")", ";"};
        QCOMPARE(tokenizeCode("void func();"), exp9);

        // 10. Сложное выражение
        QStringList exp10 = {"a1", "*", "(", "-", "2.5", "+", "b", ")", "^", "3", ";"};
        QCOMPARE(tokenizeCode("a1*(-2.5+b)^3;"), exp10);

        // 11. Оператор сравнения
        QStringList exp11 = {"a", ">", "=", "b", ";"};
        QCOMPARE(tokenizeCode("a>=b;"), exp11);

        // 12. Логический оператор
        QStringList exp12 = {"a", "&", "&", "b", ";"};
        QCOMPARE(tokenizeCode("a && b;"), exp12);
    }

    // Тесты для функции skipBalanced (Пропуск сбалансированной группы скобок)
    void testSkipBalanced() {
        // 1. Простая круглая скобка
        QStringList tok1 = {"(", "a", ")"};
        QCOMPARE(skipBalanced(tok1, 0, "(", ")"), 3);

        // 2. Вложенные скобки
        QStringList tok2 = {"(", "(", "a", ")", ")"};
        QCOMPARE(skipBalanced(tok2, 0, "(", ")"), 5);

        // 3. Пропуск квадратных скобок массива
        QStringList tok3 = {"[", "10", "]"};
        QCOMPARE(skipBalanced(tok3, 0, "[", "]"), 3);

        // 4. Фигурные скобки
        QStringList tok4 = {"{", "a", "}", "b"};
        QCOMPARE(skipBalanced(tok4, 0, "{", "}"), 3);

        // 5. Несколько элементов внутри скобок
        QStringList tok5 = {"(", "a", "+", "b", ")", "c"};
        QCOMPARE(skipBalanced(tok5, 0, "(", ")"), 5);

        // 6. Скобки в конце последовательности (нет закрывающей скобки)
        QStringList tok6 = {"(", "a", "+", "b"};
        QCOMPARE(skipBalanced(tok6, 0, "(", ")"), 4); // размер списка = 4

        // 7. Баланс внутри строки объявления
        QStringList tok7 = {"(", "a", "[", "3", "]", ")", ";"};
        QCOMPARE(skipBalanced(tok7, 0, "(", ")"), 6);

        // 8. Одиночная открывающая скобка
        QStringList tok8 = {"("};
        QCOMPARE(skipBalanced(tok8, 0, "(", ")"), 1); // размер списка = 1

        // 9. Массив с индексом-выражением
        QStringList tok9 = {"[", "a", "+", "1", "]", "b"};
        QCOMPARE(skipBalanced(tok9, 0, "[", "]"), 5);

        // 10. Начальный индекс не равен нулю
        QStringList tok10 = {"x", "y", "(", "a", "+", "b", ")", "z"};
        QCOMPARE(skipBalanced(tok10, 2, "(", ")"), 7);
    }

    // Тесты для функции skipInitializer (Пропуск инициализирующего выражения)
    void testSkipInitializer() {
        // 1. Простая инициализация
        QStringList tok1 = {"5", ";"};
        QCOMPARE(skipInitializer(tok1, 0), 1);

        // 2. Инициализация со скобками
        QStringList tok2 = {"(", "1", "+", "2", ")", ";"};
        QCOMPARE(skipInitializer(tok2, 0), 5);

        // 3. Инициализация массива
        QStringList tok3 = {"{", "1", ",", "2", "}", ";"};
        QCOMPARE(skipInitializer(tok3, 0), 5);

        // 4. Инициализация до запятой
        QStringList tok4 = {"5", ",", "b", "]"};
        QCOMPARE(skipInitializer(tok4, 0), 1);

        // 5. Пустая инициализация
        QStringList tok5 = {";"};
        QCOMPARE(skipInitializer(tok5, 0), 0);

        // 6. Инициализация с логическим оператором
        QStringList tok6 = {"a", "&&", "b", ";"};
        QCOMPARE(skipInitializer(tok6, 0), 3);

        // 7. Инициализация массива с фигурными скобками
        QStringList tok7 = {"{", "{", "1", "}", ",", "{", "2", "}", "}", ";"};
        QCOMPARE(skipInitializer(tok7, 0), 9);

        // 8. Инициализация сложного выражения
        QStringList tok8 = {"a", "+", "(", "b", "*", "c", ")", ";"};
        QCOMPARE(skipInitializer(tok8, 0), 7);

        // 9. Инициализация до конца списка
        QStringList tok9 = {"5"};
        QCOMPARE(skipInitializer(tok9, 0), 1);

        // 10. Инициализация с оператором сравнения
        QStringList tok10 = {"a", ">=", "b", ";"};
        QCOMPARE(skipInitializer(tok10, 0), 3);
    }

    // Тесты для функции extractDeclaredVariables (Поиск объявленных переменных)
    void testExtractDeclaredVariables() {
        // 1. Одна переменная
        QStringList tok1 = {"int", "a", ";"};
        QCOMPARE(extractDeclaredVariables(tok1), QSet<QString>({"a"}));

        // 2. Несколько переменных
        QStringList tok2 = {"int", "a", ",", "b", ";"};
        QCOMPARE(extractDeclaredVariables(tok2), QSet<QString>({"a", "b"}));

        // 3. Массив
        QStringList tok3 = {"int", "arr", "[", "10", "]", ";"};
        QCOMPARE(extractDeclaredVariables(tok3), QSet<QString>({"arr"}));

        // 4. struct-переменная (пишем 'struct' так как C регистрозависим)
        QStringList tok4 = {"struct", "Point", "p", ";"};
        QCOMPARE(extractDeclaredVariables(tok4), QSet<QString>({"p"}));

        // 5. enum-переменная
        QStringList tok5 = {"enum", "Color", "c", ";"};
        QCOMPARE(extractDeclaredVariables(tok5), QSet<QString>({"c"}));

        // 6. union-переменная
        QStringList tok6 = {"union", "Data", "d", ";"};
        QCOMPARE(extractDeclaredVariables(tok6), QSet<QString>({"d"}));

        // 7. Модификатор const
        QStringList tok7 = {"const", "int", "value", ";"};
        QCOMPARE(extractDeclaredVariables(tok7), QSet<QString>({"value"}));

        // 8. Структура (вместе с ее внутренними переменными, которые мы должны проигнорировать)
        QStringList tok8 = {"struct", "Point", "{", "int", "a", ";", "int", "y", ";", "}", "p", ";"};
        QCOMPARE(extractDeclaredVariables(tok8), QSet<QString>({"p"}));

        // 9. Длинное объявление
        QStringList tok9 = {"unsigned", "long", "int", "x", ";"};
        QCOMPARE(extractDeclaredVariables(tok9), QSet<QString>({"x"}));
    }
};

// Макрос, который генерирует функцию main() специально для тестов
QTEST_MAIN(TestAnalyzer)
#include "main.moc" // Обязательно для работы сигналов/слотов QObject в одном cpp файле

#endif
