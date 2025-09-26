#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cctype>

using namespace std;

// Класс для узла дерева
class Node {
public:
    virtual ~Node() {}
    virtual double calc() = 0;
    virtual string toDot() = 0;
};

// Узел для числа
class NumNode : public Node {
    double num;
public:
    NumNode(double n) : num(n) {}
    double calc() override { return num; }
    string toDot() override {
        return "n" + to_string((long long)this) + " [label=\"" + to_string((int)num) + "\"];\n";
    }
};

// Узел для операции
class OpNode : public Node {
    char operation;
    Node* left;
    Node* right;
public:
    OpNode(char op, Node* l, Node* r) : operation(op), left(l), right(r) {}
    ~OpNode() {
        delete left;
        delete right;
    }
    
    double calc() override {
        double l_val = left->calc();
        double r_val = right->calc();
        
        switch(operation) {
            case '+': return l_val + r_val;
            case '-': return l_val - r_val;
            case '*': return l_val * r_val;
            case '/': 
                if (r_val == 0) {
                    cout << "Ошибка: деление на 0!" << endl;
                    return 0;
                }
                return l_val / r_val;
            default: return 0;
        }
    }
    
    string toDot() override {
        string result = "n" + to_string((long long)this) + " [label=\"" + string(1, operation) + "\"];\n";
        result += "n" + to_string((long long)this) + " -> n" + to_string((long long)left) + ";\n";
        result += "n" + to_string((long long)this) + " -> n" + to_string((long long)right) + ";\n";
        result += left->toDot() + right->toDot();
        return result;
    }
};

// Основной класс калькулятора
class Calc {
    vector<string> rpn; // обратная польская запись
    Node* treeRoot;     // корень дерева

    // Приоритет операций
    int getPriority(char op) {
        if (op == '+' || op == '-') return 1;
        if (op == '*' || op == '/') return 2;
        return 0;
    }

    // Разбиение на токены
    vector<string> splitExpr(string expr) {
        vector<string> tokens;
        string current = "";
        
        for (char c : expr) {
            if (c == ' ') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current = "";
                }
            } else if (c == '(' || c == ')' || c == '+' || c == '-' || c == '*' || c == '/') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current = "";
                }
                tokens.push_back(string(1, c));
            } else if (isdigit(c)) {
                current += c;
            }
        }
        
        if (!current.empty()) {
            tokens.push_back(current);
        }
        
        return tokens;
    }

    // Перевод в ОПН
    void toRPN(string expr) {
        stack<char> ops;
        vector<string> tokens = splitExpr(expr);
        rpn.clear();
        
        for (string token : tokens) {
            if (isdigit(token[0])) {
                rpn.push_back(token);
            } else if (token == "(") {
                ops.push('(');
            } else if (token == ")") {
                while (!ops.empty() && ops.top() != '(') {
                    rpn.push_back(string(1, ops.top()));
                    ops.pop();
                }
                if (!ops.empty()) ops.pop();
            } else {
                char op = token[0];
                while (!ops.empty() && getPriority(ops.top()) >= getPriority(op)) {
                    rpn.push_back(string(1, ops.top()));
                    ops.pop();
                }
                ops.push(op);
            }
        }
        
        while (!ops.empty()) {
            rpn.push_back(string(1, ops.top()));
            ops.pop();
        }
    }

    // Построение дерева из ОПН
    void buildTree() {
        stack<Node*> nodes;
        
        for (string token : rpn) {
            if (isdigit(token[0])) {
                nodes.push(new NumNode(stod(token)));
            } else {
                if (nodes.size() < 2) {
                    cout << "Ошибка в выражении" << endl;
                    return;
                }
                Node* right = nodes.top(); nodes.pop();
                Node* left = nodes.top(); nodes.pop();
                nodes.push(new OpNode(token[0], left, right));
            }
        }
        
        if (nodes.size() == 1) {
            treeRoot = nodes.top();
        } else {
            treeRoot = nullptr;
        }
    }

public:
    Calc() : treeRoot(nullptr) {}
    
    ~Calc() {
        delete treeRoot;
    }
    
    double calculate(string expr) {
        delete treeRoot;
        treeRoot = nullptr;
        
        toRPN(expr);
        buildTree();
        
        if (treeRoot != nullptr) {
            return treeRoot->calc();
        }
        return 0;
    }
    
    void makeDotFile(string filename) {
        if (treeRoot == nullptr) return;
        
        ofstream file(filename);
        file << "digraph G {\n";
        file << "node [shape=circle];\n";
        file << treeRoot->toDot();
        file << "}\n";
        file.close();
    }
};

// Главная функция
int main() {
    Calc calculator;
    string input;
    
    cout << "Калькулятор" << endl;
    cout << "Введите выражение: ";
    getline(cin, input);
    
    if (input.empty()) {
        cout << "Не введено выражение" << endl;
        return 1;
    }
    
    double result = calculator.calculate(input);
    cout << "Ответ: " << result << endl;
    
    calculator.makeDotFile("tree.dot");
    cout << "Дерево сохранено в tree.dot" << endl;
    
    // Пробуем создать картинку если установлен graphviz
    if (system("which dot > /dev/null 2>&1") == 0) {
        system("dot -Tpng tree.dot -o tree.png");
        cout << "Создана картинка tree.png" << endl;
    } 
    
    return 0;
}
