#include <iostream>
#include <string>
#include <unordered_map>
#include <regex>
#include <sstream>
#include <cmath>
#include <stdexcept>

// Хранилище для констант
std::unordered_map<std::string, std::string> constants;

// Функция для проверки, является ли строка числом
bool is_number(const std::string& value) {
    return std::regex_match(value, std::regex(R"(^-?\d+$)"));
}

// Функция для проверки, является ли строка строкой
bool is_string(const std::string& value) {
    return std::regex_match(value, std::regex(R"(^'(.*?)'$)"));
}

// Функция для проверки, является ли строка массивом
bool is_array(const std::string& value) {
    return std::regex_match(value, std::regex(R"(^\(\{\s*(.*?)\s*\}\)$)"));
}

// Функция для проверки, является ли строка ссылкой на константу
bool is_constant(const std::string& value) {
    return std::regex_match(value, std::regex(R"(^@\[(.*?)\]$)"));
}

// Замена ссылок на константы в выражении
std::string replace_constants(const std::string& expression) {
    std::regex constant_regex(R"(@\[(.*?)\])");
    std::string result = expression;
    std::smatch match;

    // Находим и заменяем все ссылки на константы
    while (std::regex_search(result, match, constant_regex)) {
        std::string constant_name = match[1].str();
        if (constants.find(constant_name) == constants.end()) {
            throw std::runtime_error("Ошибка: константа не определена: " + constant_name);
        }
        // Заменяем ссылку на значение константы
        result.replace(match.position(0), match.length(0), constants[constant_name]);
    }

    return result;
}

// Разбор математического выражения
int evaluate_expression(const std::string& expression) {
    std::istringstream iss(expression);
    int result;
    char op;
    iss >> result;

    while (iss >> op) {
        int value;
        iss >> value;
        switch (op) {
        case '+': result += value; break;
        case '-': result -= value; break;
        case '*': result *= value; break;
        case '/':
            if (value == 0) throw std::runtime_error("Ошибка: деление на ноль.");
            result /= value;
            break;
        case '^': result = std::pow(result, value); break;
        default: throw std::runtime_error("Ошибка: неизвестная операция.");
        }
    }

    return result;
}

// Разбор массивов
std::string parse_array(const std::string& value) {
    std::string content = value.substr(2, value.size() - 4); // Убираем ({ и })
    std::stringstream ss(content);
    std::string item;
    std::vector<std::string> elements;

    while (std::getline(ss, item, ',')) {
        // Убираем пробелы в начале и конце
        item = std::regex_replace(item, std::regex(R"(^\s+|\s+$)"), "");

        if (is_number(item) || is_string(item)) {
            // Если элемент - число или строка, добавляем его в массив
            elements.push_back(item);
        }
        else if (is_constant(item)) {
           
            std::string constant_name = item.substr(2, item.size() - 3); // Убираем @[ и ]
            if (constants.find(constant_name) != constants.end()) {
                elements.push_back(constants[constant_name]);
            }
            else {
                throw std::runtime_error("Ошибка: константа не определена: " + constant_name);
            } 
            
        }
        else {
            // Если элемент некорректный, выбрасываем исключение
            throw std::runtime_error("Ошибка: неверный элемент массива: " + item);
        }
    }

    // Формируем результат в виде массива
    std::ostringstream result;
    result << "[";
    for (size_t i = 0; i < elements.size(); ++i) {
        result << elements[i];
        if (i < elements.size() - 1) {
            result << ", ";
        }
    }
    result << "]";
    return result.str();
}


// Разбор значений
std::string parse_value(const std::string& value) {
    if (is_number(value)) {
        return value;
    }
    else if (is_string(value)) {
        return value;
    }
    else if (is_array(value)) {
        return parse_array(value);
    }
    else if (is_constant(value)) {
        std::string constant_name = value.substr(2, value.size() - 3); // Убираем @[ и ]
        if (constants.find(constant_name) != constants.end()) {
            return constants[constant_name];
        }
        else {
            throw std::runtime_error("Ошибка: константа не определена: " + constant_name);
        }
    }
    else if (value.find_first_of("+-*/^") != std::string::npos) {
        std::string replaced_value = replace_constants(value); // Заменяем ссылки на константы
        return std::to_string(evaluate_expression(replaced_value)); // Вычисляем результат
    }
    else {
        throw std::runtime_error("Ошибка: неверное значение: " + value);
    }
}

// Разбор строк конфигурационного языка
void parse_line(const std::string& line) {
    std::regex var_declaration(R"(var\s+([A-Z]+)\s*:=\s*(.+);)");
    std::smatch match;

    if (std::regex_match(line, match, var_declaration)) {
        std::string name = match[1];
        std::string value = match[2];
        constants[name] = parse_value(value); // Сохраняем значение переменной
    }
    else {
        throw std::runtime_error("Ошибка: неверный синтаксис: " + line);
    }
}

// Преобразование данных в TOML
std::string to_toml() {
    std::ostringstream toml_output;
    for (const auto& pair : constants) {
        toml_output << pair.first << " = " << pair.second << "\n";
    }
    return toml_output.str();
}

// Основная программа
int main() {

    setlocale(0, "");

    try {
        std::string line;
        std::cout << "Введите код на учебном конфигурационном языке. Для завершения введите EOF (Ctrl+D):\n";

        while (std::getline(std::cin, line)) {
            if (line != "EOF") {
                parse_line(line);
            }
            else {
                break;
            }
        }

        std::cout << "\nРезультат в формате TOML:\n";
        std::cout << to_toml();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}




