#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

#define SEPARATOR "---"
using namespace std;

enum TokenType
{
    SEMICOLON,
    COMMA,
    EQUAL,
    LBRACKET,
    RBRACKET,
    INTEGER,
    IDENTIFIER,
    SUM,
    RAND,
    ERROR,
    END_OF_FILE,
    NONE,
    N_ROOT,
    N_LINE
};

string Tokens[] = {
    "SEMICOLON", "COMMA", "EQUAL", "LBRACKET", "RBRACKET", "INTEGER", "IDENTIFIER", "SUM", "RAND", "ERROR", "END_OF_FILE", "NONE",
    "N_ROOT", "N_LINE"};

struct Token
{
    TokenType type;
    string lexeme;
    int line;

    Token(TokenType type, string lexeme, int line = 0) : type(type), lexeme(lexeme), line(line){};
};

class LexicalAnalyzer
{
public:
    LexicalAnalyzer(string input) : input(input), line(1), pos(0) {}
    Token getNextToken();

private:
    string input;
    int line, pos;
    char currentChar();
    char nextChar();
    void skipWhitespace();
    Token scanIdentifier();
    Token scanNumber();
    Token scanOperator();
};

void LexicalAnalyzer::skipWhitespace()
{
    while (isspace(currentChar()))
    {
        if (currentChar() == '\n')
            line++;
        nextChar();
    }
}

char LexicalAnalyzer::currentChar()
{
    if (pos >= input.length())
        return '\0';
    return input[pos];
}

char LexicalAnalyzer::nextChar()
{
    pos++;
    return currentChar();
}

Token LexicalAnalyzer::scanIdentifier()
{
    string lexeme;
    while (isalnum(currentChar()) || currentChar() == '_')
    {
        lexeme += currentChar();
        nextChar();
    }
    if (lexeme == "sum")
    {
        return Token{SUM, lexeme, line};
    }
    else if (lexeme == "random_choise")
    {
        return Token{RAND, lexeme, line};
    }
    else
    {
        return Token{IDENTIFIER, lexeme, line};
    }
}

Token LexicalAnalyzer::scanNumber()
{
    string lexeme;
    bool has_num = false;
    while (isdigit(currentChar()))
    {
        lexeme += currentChar();
        has_num = true;
        nextChar();
    }
    if (has_num)
        return Token{INTEGER, lexeme, line};
    else
        return Token{ERROR, lexeme, line};
}

Token LexicalAnalyzer::scanOperator()
{
    switch (currentChar())
    {
    case '(':
        nextChar();
        return Token{LBRACKET, "(", line};
    case ')':
        nextChar();
        return Token{RBRACKET, ")", line};
    case ',':
        nextChar();
        return Token{COMMA, ",", line};
    case ':':
        nextChar();
        if (currentChar() == '=')
        {
            nextChar();
            return Token{EQUAL, ":=", line};
        }
        else
        {
            string ans = "";
            ans += currentChar();
            return Token{ERROR, ans, line};
        }
    case ';':
        nextChar();
        return Token{SEMICOLON, ";", line};
    default:
        string ans = "";
        ans += currentChar();
        return Token{ERROR, ans, line};
    }
}

Token LexicalAnalyzer::getNextToken()
{
    skipWhitespace();
    if (currentChar() == '\0')
    {
        return Token{END_OF_FILE, "", line};
    }
    else if (isalpha(currentChar()))
    {
        return scanIdentifier();
    }
    else if (currentChar() == '-' || isdigit(currentChar()))
    {
        return scanNumber();
    }
    else
    {
        return scanOperator();
    }
}

string getInput(string filename)
{
    ifstream file;
    file.open(filename);
    string input;

    while (file)
    {
        string newLine;
        getline(file, newLine);
        input += newLine + "\n";
    }

    file.close();
    return input;
}

struct Node
{
    Token token = Token(NONE, "", 0);
    vector<Node *> nodes;

    Node(Token token) : token(token){};
};

struct SyntacticalAnalyzer
{
    int curInd = 0;
    Node *root;
    vector<Token> tokens;
    SyntacticalAnalyzer(vector<Token> tokens) : tokens(tokens)
    {
        root = new Node(Token(N_ROOT, "Root", 0));
    };

    void syntaxError()
    {
        cout << "Syntax ERROR in some line";
        exit(0);
    }

    Token curToken()
    {
        if (curInd >= tokens.size())
            return Token(NONE, "", 0);
        return tokens[curInd];
    }

    Token nextToken()
    {
        curInd++;
        return this->curToken();
    }

    void expressions(Node *prewExpression)
    {
        prewExpression->nodes.push_back(this->expression());
        if (this->curToken().type == COMMA)
        {
            this->nextToken();
            this->expressions(prewExpression);
        }
    }

    Node *expression()
    {
        Node *expressionNode;
        if (this->curToken().type == INTEGER)
        {
            expressionNode = new Node(this->curToken());
            this->nextToken();
        }
        else if (this->curToken().type == IDENTIFIER)
        {
            expressionNode = new Node(this->curToken());
            this->nextToken();
        }
        else if (this->curToken().type == SUM || this->curToken().type == RAND)
        {
            expressionNode = new Node(this->curToken());

            if (this->nextToken().type != LBRACKET)
            {
                this->syntaxError();
            }
            this->nextToken();
            this->expressions(expressionNode);

            if (this->curToken().type == RBRACKET)
            {
                /*TODO*/
                this->nextToken();
            }
            else
            {
                this->syntaxError();
            }
        }
        else
        {
            this->syntaxError();
        }
        return expressionNode;
    }

    Node *command()
    {
        Node *equalNode;

        if (this->curToken().type == IDENTIFIER)
        {
            Node *IdNode = new Node(this->curToken());
            if (this->nextToken().type == EQUAL)
            {
                equalNode = new Node(this->curToken());
                equalNode->nodes.push_back(IdNode);

                this->nextToken();
                equalNode->nodes.push_back(this->expression());
            }
            else
            {
                this->syntaxError();
            }
        }
        else
        {
            this->syntaxError();
        }
        return equalNode;
    }

    Node *line()
    {
        Node *lineNode = new Node(Token(N_LINE, "N_LINE", curToken().line));

        if (this->curToken().type == INTEGER)
        {
            Node *lineNumber = new Node(this->curToken());
            lineNode->nodes.push_back(lineNumber);
            this->nextToken();
            lineNode->nodes.push_back(this->command());
        }
        else
        {
            this->syntaxError();
        }

        return lineNode;
    }

    void program()
    {
        while (this->curToken().type != END_OF_FILE)
        {
            this->root->nodes.push_back(this->line());
            if (this->curToken().type != SEMICOLON)
            {
                this->syntaxError();
            }
            this->nextToken();
        }
    }

    void printAST(Node *curNode, int depth = 0)
    {
        for (int i = 0; i < depth; ++i)
            cout << SEPARATOR;

        if (curNode->nodes.empty())
            cout << "| ";
        else
            cout << "> ";

        cout << curNode->token.lexeme << endl;

        for (Node *nextNode : curNode->nodes)
        {
            this->printAST(nextNode, depth + 1);
        }
    }

    void secondConditionCheck(
        Node *curNode,
        map<string, bool> &variables,
        map<string, bool> &undeclaredVariables)
    {
        if (curNode->token.type == IDENTIFIER && variables[curNode->token.lexeme] != true)
        {
            undeclaredVariables[curNode->token.lexeme] = true;
        }
        for (Node *nextNode : curNode->nodes)
        {
            this->secondConditionCheck(nextNode, variables, undeclaredVariables);
        }
    }

    void checkVariables()
    {
        vector<pair<int, Node *>> lines;
        for (Node *line : this->root->nodes)
        {
            lines.push_back({stoi(line->nodes[0]->token.lexeme), line});
        }
        sort(lines.begin(), lines.end(),
             [](pair<int, Node *> lhs, pair<int, Node *> rhs)
             {
                 return lhs.first < rhs.first;
             });

        map<int, vector<int>> mp;

        bool firstContidion = true;
        for (auto [lineNumb, node] : lines)
        {
            mp[lineNumb].push_back(node->token.line);

            if (mp[lineNumb].size() != 1)
                firstContidion = false;
        }

        if (!firstContidion)
        {
            for (auto [lineNumb, textLines] : mp)
            {
                if (textLines.size() > 1)
                {
                    cout << "line number " << lineNumb << " is used several times:\n";
                    for (auto textLine : textLines)
                    {
                        cout << "In line: " << textLine << endl;
                    }
                }
            }
        }
        else
        {
            map<string, bool> variables;
            for (auto [lineNumb, lineNode] : lines)
            {
                Node *commandNode = lineNode->nodes.back();
                map<string, bool> undeclaredVariables;

                secondConditionCheck(commandNode->nodes.back(), variables, undeclaredVariables);

                if (variables[commandNode->nodes[0]->token.lexeme] == 0)
                {
                    variables[commandNode->nodes[0]->token.lexeme] = 1;
                    cout << "In line " << lineNumb << " we declare new variable " << commandNode->nodes[0]->token.lexeme << endl;
                }
                else
                {
                    cout << "In line " << lineNumb << " we assign existing variable " << commandNode->nodes[0]->token.lexeme << endl;
                }

                if (!undeclaredVariables.empty())
                {
                    cout << "For identifier " << commandNode->nodes[0]->token.lexeme << " in line " << lineNumb << " we have undeclared variable[s] :\n";
                    for (auto [key, value] : undeclaredVariables)
                    {
                        cout << "Identifier " << key << endl;
                    }
                }
            }
        }
    }
};

int main()
{
    string tests[10];
    tests[0] = R"(1 x := 3;)"; /// good test

    tests[1] = R"(1 x := sum(1, 5);
2 y := sum(random_choise(1, x), x);)"; /// good test

    tests[2] = R"(1 x := )"; /// syntax error

    tests[3] = R"(1 x := give_me_sum(1, 5);
2 y := sum(give_me_random_number(1, x), x);)"; /// syntax error

    tests[4] = R"(1 x:= x;
1 y := y;
3 x := 1;
3 z := sum(x, y, a, b);)"; /// first condition violation

    tests[5] = R"(3 x:= x;
2 y := sum(1, 5);
1 x := 1;
6 z := sum(x, y, a);
9 a := sum(a);)"; /// second condition violation

    tests[6] = R"(1 x := sum(a, b, c, d);)"; /// second condition violation

    string input = tests[0];

    cout << "CHECKING TEST WITH PLAIN TEXT: \n\n";
    cout << input << "\n\n";
    cout << "END RESULT IS: \n\n";

    // string input = getInput("./examples/fib.nice");
    // cout << input << endl;

    LexicalAnalyzer lexer(input);
    Token token(NONE, "", 0);
    vector<Token> tokens;
    do
    {
        token = lexer.getNextToken();

        if (token.type == ERROR)
        {
            cout << "source input has syntax errors in line " << token.line << " with token '" << token.lexeme << "'" << endl;
            return 0;
        }
        tokens.push_back(token);
    } while (token.type != END_OF_FILE);

    SyntacticalAnalyzer syntacticalAnalyzer(tokens);
    syntacticalAnalyzer.program();
    syntacticalAnalyzer.printAST(syntacticalAnalyzer.root);
    syntacticalAnalyzer.checkVariables();

    return 0;
}
