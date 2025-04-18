/**
 * Ahthor:Code By LenLe
 * Date: 2025/4/13
 */

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <iomanip> // 添加格式化头文件

void SplitLine(const std::string &line, std::vector<std::string> &line_split)
{
    size_t index = 0;
    while (index < line.size())
    {
        // 跳过空格
        while (index < line.size() && std::isspace(line[index]))
        {
            ++index;
        }

        if (index >= line.size())
            break;

        char ch = line[index];

        // 字母可能和撇号组成 T'
        if (std::isalpha(ch))
        {
            std::string token(1, ch);
            ++index;
            if (index < line.size() && line[index] == '\'')
            {
                token += '\'';
                ++index;
            }
            line_split.push_back(token);
        }
        // 符号（如 *）单独处理
        else
        {
            line_split.push_back(std::string(1, ch));
            ++index;
        }
    }
}

void ConstructPredictiveAnalysisTables(
    std::map<std::string, std::map<std::string, std::vector<std::string>>> &
        predictive_analysis_tables,
    const std::string &file_path)
{
    std::ifstream input_file(file_path);
    std::string line;

    std::vector<std::string> line_split;

    if (!input_file.is_open())
    {
        std::cerr << "File Not Found" << std::endl;
        return;
    }

    while (std::getline(input_file, line))
    {
        line_split.clear();
        SplitLine(line, line_split);

        std::vector<std::string> production(line_split.begin() + 2, line_split.end());
        predictive_analysis_tables[line_split[0]][line_split[1]] = production;
    }
}

void PrintPredictiveAnalysisTables(
    const std::map<std::string, std::map<std::string, std::vector<std::string>>> &tables)
{
    for (const auto &[non_terminal, inner_map] : tables)
    {
        std::cout << "non_terminal: " << non_terminal << std::endl;

        for (const auto &[terminal, production] : inner_map)
        {
            std::cout << "    terminal: " << terminal << " => ";

            for (const auto &symbol : production)
            {
                std::cout << symbol << " ";
            }

            std::cout << std::endl;
        }

        std::cout << std::endl;
    }
}

// 辅助函数：将栈转换为字符串
std::string StackToString(const std::vector<std::string> &stack)
{
    std::string result;
    for (auto it = stack.rbegin(); it != stack.rend(); ++it)
    {
        result += *it;
    }
    return result;
}

// 辅助函数：判断是否为终结符
bool isTerminal(const std::string &symbol, const std::vector<std::string> &terminals)
{
    return std::find(terminals.begin(), terminals.end(), symbol) != terminals.end();
}

void MatchString(std::string input, const std::vector<std::string> &non_terminals,
                 const std::vector<std::string> &terminals,
                 std::map<std::string, std::map<std::string, std::vector<std::string>>> predictive_analysis_tables)
{
    // 列宽常量定义
    const int STEP_WIDTH = 4;
    const int STACK_WIDTH = 15;
    const int INPUT_WIDTH = 15;
    const int PRODUCTION_WIDTH = 20;

    std::ofstream outfile("../result.txt");
    if (!outfile)
    {
        std::cerr << "无法打开文件 result.txt" << std::endl;
        return;
    }

    input = input + "#";
    std::vector<std::string> stack = {"#", "E"};

    int input_ptr = 0;
    int step = 1;

    // 输出表头
    auto PrintHeader = [&](std::ostream &os)
    {
        os << std::left
           << std::setw(STEP_WIDTH) << "步骤"
           << std::setw(STACK_WIDTH) << "分析栈"
           << std::setw(INPUT_WIDTH) << "剩余输入"
           << "产生式"
           << std::endl;
    };

// 双重输出宏
#define OUTPUT(step_val, stack_str, input_str, production_str) \
    do                                                         \
    {                                                          \
        std::cout << std::left                                 \
                  << std::setw(STEP_WIDTH) << step_val         \
                  << std::setw(STACK_WIDTH) << stack_str       \
                  << std::setw(INPUT_WIDTH) << input_str       \
                  << production_str << std::endl;              \
        outfile << std::left                                   \
                << std::setw(STEP_WIDTH) << step_val           \
                << std::setw(STACK_WIDTH) << stack_str         \
                << std::setw(INPUT_WIDTH) << input_str         \
                << production_str << std::endl;                \
    } while (0)

    // 输出表头
    PrintHeader(std::cout);
    PrintHeader(outfile);

    while (!stack.empty())
    {
        std::string top = stack.back();
        std::string current_char = (input_ptr < input.size()) ? std::string(1, input[input_ptr]) : "#";

        // 准备输出内容
        std::string stack_str = StackToString(stack);
        std::string remaining_char = input.substr(input_ptr);
        std::string production_str;

        // 处理逻辑
        if (top == "#" && current_char == "#")
        {
            OUTPUT(step, stack_str, remaining_char, "Acc");
            break;
        }

        if (isTerminal(top, terminals) || top == "#")
        {
            if (top == current_char)
            {
                production_str = "match " + top;
                stack.pop_back();
                input_ptr++;
            }
            else
            {
                production_str = "错误: 期望 " + top + "，实际输入 " + current_char;
                OUTPUT(step, stack_str, remaining_char, production_str);
                break;
            }
        }
        else
        {
            if (!predictive_analysis_tables.count(top))
            {
                production_str = "错误：无效非终结符 " + top;
                OUTPUT(step, stack_str, remaining_char, production_str);
                break;
            }

            auto &row = predictive_analysis_tables[top];
            if (!row.count(current_char))
            {
                production_str = "错误：无产生式，当前输入 " + current_char;
                OUTPUT(step, stack_str, remaining_char, production_str);
                break;
            }

            auto &production = row[current_char];
            stack.pop_back();
            for (auto it = production.rbegin(); it != production.rend(); ++it)
            {
                if (*it != "$")
                    stack.push_back(*it);
            }

            // 构建产生式字符串
            production_str = top + "→";
            for (const auto &s : production)
                production_str += s;
        }

        // 输出当前步骤
        OUTPUT(step, stack_str, remaining_char, production_str);
        step++;
    }

#undef OUTPUT
}

int main(int argc, char *argv[])
{
    std::vector<std::string> non_terminal = {"E", "E'", "T", "T'", "F"};
    std::vector<std::string> terminal = {"i", "(", ")", "*", "+"};

    std::string input;
    std::cin >> input;

    std::map<std::string, std::map<std::string, std::vector<std::string>>> predictive_analysis_tables;

    const std::string file_path = "../input.txt";
    ConstructPredictiveAnalysisTables(predictive_analysis_tables, file_path);

    MatchString(input, non_terminal, terminal, predictive_analysis_tables);

    return 0;
}
