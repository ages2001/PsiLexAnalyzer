#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define UNLIMITED 1000  // assume it's unlimited

size_t lineNumber = 1;


int exitProgram(FILE *inputFile, FILE *outputFile, int code);

char parseWord(FILE *inputFile, FILE *outputFile, char ch);

char parseNumber(FILE *inputFile, FILE *outputFile, char ch);

int isKeyword(char *word);

int isSpace(char ch);

char skipSpace(FILE *inputFile, char ch);

int isBracket(char ch);

char parseBracket(FILE *inputFile, FILE *outputFile, char ch);

int isOperator(char ch);

char parseOperator(FILE *inputFile, FILE *outputFile, char ch);

char parseStringConst(FILE *inputFile, FILE *outputFile, char ch);


int main(int argc, char *argv[]) {
    if (argc < 2) { // get file name from command prompt
        printf("You should enter file name from command prompt! Example: PsiLexAnalyzer.exe code.psi\n");
        return -1;
    }

    char *fileName = argv[1];

    FILE *inputFile = fopen(fileName, "r");

    if (inputFile == NULL) {
        printf("Can't open the input file!\n");
        return 1;
    }

    FILE *outputFile = fopen("code.lex", "w");

    if (outputFile == NULL) {
        printf("Can't create the output file!\n");
        fclose(inputFile);
        return 1;
    }

    char ch = fgetc(inputFile);

    while (ch != EOF) {
        if (isalpha(ch))
            ch = parseWord(inputFile, outputFile, ch);

        else if (isdigit(ch))
            ch = parseNumber(inputFile, outputFile, ch);

        else if (isOperator(ch))
            ch = parseOperator(inputFile, outputFile, ch);

        else if (ch == '"')
            ch = parseStringConst(inputFile, outputFile, ch);

        else if (isBracket(ch))
            ch = parseBracket(inputFile, outputFile, ch);

        else if (isSpace(ch))
            ch = skipSpace(inputFile, ch);

        else if (ch == ';') {
            fprintf(outputFile, "EndOfLine\n");
            ch = fgetc(inputFile);
        } else {
            printf("Invalid character ' %c ' ! (at line %zu)\n", ch, lineNumber);
            exitProgram(inputFile, outputFile, ch);
        }
    }

    exitProgram(inputFile, outputFile, 0);
}

int exitProgram(FILE *inputFile, FILE *outputFile, int code) {
    fclose(inputFile);
    fclose(outputFile);
    if (code == 0) printf("Lexical analyzed successfully!\n");
    else remove("code.lex");

    exit(code);
}

char parseWord(FILE *inputFile, FILE *outputFile, char ch) {
    char *word = (char *) malloc(sizeof(char) * (30 + 1));
    if (word == NULL) {
        printf("Memory allocation failed! (at line %zu)\n", lineNumber);
        exitProgram(inputFile, outputFile, 1);
    }
    size_t size = 0;

    while (ch != EOF && (isalnum(ch) || ch == '_')) {
        if (size > 29) {
            printf("Identifier can't longer than 30 characters! (at line %zu)\n", lineNumber);
            exitProgram(inputFile, outputFile, 1);
        }
        word[size++] = tolower(ch);
        ch = fgetc(inputFile);
    }
    word[size] = '\0';

    if (isKeyword(word)) fprintf(outputFile, "Keyword(%s)\n", word);
    else fprintf(outputFile, "Identifier(%s)\n", word);

    free(word);
    return ch;
}

char parseNumber(FILE *inputFile, FILE *outputFile, char ch) {
    char *word = (char *) malloc(sizeof(char) * (10 + 1));
    if (word == NULL) {
        printf("Memory allocation failed! (at line %zu)\n", lineNumber);
        exitProgram(inputFile, outputFile, 1);
    }
    size_t size = 0;

    while (ch != EOF && isdigit(ch)) {
        if (size > 9) {
            printf("Integer size can't longer than 10 digits! (at line %zu)\n", lineNumber);
            exitProgram(inputFile, outputFile, 1);
        }
        word[size++] = tolower(ch);
        ch = fgetc(inputFile);
    }

    if (isalpha(ch)) {
        printf("Invalid identifier! (at line %zu)\n", lineNumber);
        exitProgram(inputFile, outputFile, 1);
    }

    word[size] = '\0';
    fprintf(outputFile, "IntConst(%s)\n", word);

    free(word);
    return ch;
}

int isKeyword(char *word) {
    const char keywords[18][9] = {"break", "case", "char", "const", "continue", "do", "else", "enum", "float",
                                  "for", "goto", "if", "int", "long", "record", "return", "static", "while"};
    size_t size = sizeof(keywords) / sizeof(keywords[0]);

    for (size_t i = 0; i < size; i++) {
        if (strcmp(keywords[i], word) == 0)
            return 1;
    }
    return 0;
}

int isSpace(char ch) {
    return ch == ' ' || ch == '\n' || ch == '\t';
}

char skipSpace(FILE *inputFile, char ch) {
    while (ch != EOF && isSpace(ch)) {
        if (ch == '\n')
            lineNumber++;
        ch = fgetc(inputFile);
    }
    return ch;
}

int isBracket(char ch) {
    return ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']';
}

char parseBracket(FILE *inputFile, FILE *outputFile, char ch) {
    if (ch == '(') fprintf(outputFile, "LeftPar\n");
    else if (ch == ')') fprintf(outputFile, "RightPar\n");
    else if (ch == '{') fprintf(outputFile, "LeftCurlyBracket\n");
    else if (ch == '}') fprintf(outputFile, "RightCurlyBracket\n");
    else if (ch == '[') fprintf(outputFile, "LeftSquareBracket\n");
    else fprintf(outputFile, "RightSquareBracket\n");

    ch = fgetc(inputFile);

    return ch;
}

int isOperator(char ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == ':';
}

char parseOperator(FILE *inputFile, FILE *outputFile, char ch) {
    char *operatorStr = (char *) malloc(sizeof(char) * (2 + 1));
    if (operatorStr == NULL) {
        printf("Memory allocation failed! (at line %zu)\n", lineNumber);
        exitProgram(inputFile, outputFile, 1);
    }

    char first = ch, next = '\0';
    operatorStr[0] = first;

    if (first == ':') {
        ch = fgetc(inputFile);
        if (ch != '=') {
            printf("Invalid operator ':' ! (at line %zu)\n", lineNumber);
            exitProgram(inputFile, outputFile, 1);
        } else {
            operatorStr[1] = ch;
            operatorStr[2] = '\0';
        }
        next = fgetc(inputFile);

        if (isOperator(next)) {
            printf("Invalid operator ' %c%c ' ! (at line %zu)\n", first, next, lineNumber);
            exitProgram(inputFile, outputFile, 1);
        }
    } else if (first == '/') {
        next = fgetc(inputFile);
        if (next == '*') {
            ch = fgetc(inputFile);
            while (ch != EOF) {
                if (ch == '\n') lineNumber++;

                if (ch == '*') {
                    ch = fgetc(inputFile);
                    if (ch == '/') {
                        ch = fgetc(inputFile);
                        return ch;
                    }
                } else ch = fgetc(inputFile);
            }

            printf("End of file before terminating comment! (at line %zu)\n", lineNumber);
            exitProgram(inputFile, outputFile, 1);
        } else if (isOperator(next)) {
            printf("Invalid operator ' %c%c ' ! (at line %zu)\n", first, next, lineNumber);
            exitProgram(inputFile, outputFile, 1);
        } else
            operatorStr[1] = '\0';

    } else if (first == '+') {
        ch = fgetc(inputFile);
        if (ch == '+') {
            operatorStr[1] = ch;
            operatorStr[2] = '\0';

            next = fgetc(inputFile);
            if (isOperator(next)) {
                printf("Invalid operator ' %c%c ' ! (at line %zu)\n", first, next, lineNumber);
                exitProgram(inputFile, outputFile, 1);
            }
        } else {
            operatorStr[1] = '\0';
            next = ch;
            if (isOperator(next)) {
                printf("Invalid operator ' %c%c ' ! (at line %zu)\n", first, next, lineNumber);
                exitProgram(inputFile, outputFile, 1);
            }
        }
    } else if (first == '-') {
        ch = fgetc(inputFile);
        if (ch == '-') {
            operatorStr[1] = ch;
            operatorStr[2] = '\0';

            next = fgetc(inputFile);
            if (isOperator(next)) {
                printf("Invalid operator ' %c%c ' ! (at line %zu)\n", first, next, lineNumber);
                exitProgram(inputFile, outputFile, 1);
            }
        } else if (isdigit(ch) != 0) {
            printf("Negative values are not allowed! (at line %zu)\n", lineNumber);
            exitProgram(inputFile, outputFile, 1);
        } else {
            next = ch;
            if (isOperator(next)) {
                printf("Invalid operator ' %c%c ' ! (at line %zu)\n", first, next, lineNumber);
                exitProgram(inputFile, outputFile, 1);
            }
            operatorStr[1] = '\0';
        }
    } else {
        next = fgetc(inputFile);
        if (isOperator(next)) {
            printf("Invalid operator ' %c%c ' ! (at line %zu)\n", first, next, lineNumber);
            exitProgram(inputFile, outputFile, 1);
        }
        operatorStr[1] = '\0';
    }
    fprintf(outputFile, "Operator(%s)\n", operatorStr);

    free(operatorStr);
    return next;
}

char parseStringConst(FILE *inputFile, FILE *outputFile, char ch) {
    ch = fgetc(inputFile);

    char *strConst = (char *) malloc(sizeof(char) * UNLIMITED);
    if (strConst == NULL) {
        printf("Memory allocation failed ! (at line %zu)\n", lineNumber);
        exitProgram(inputFile, outputFile, 1);
    }
    size_t size = 0;

    while (ch != EOF && ch != '"') {
        if (ch == '\n') lineNumber++;
        strConst[size++] = ch;
        ch = fgetc(inputFile);
    }
    strConst[size] = '\0';

    if (ch == EOF) {
        printf("End of file before terminating string constant! (at line %zu)\n", lineNumber);
        exitProgram(inputFile, outputFile, 1);
    }

    ch = fgetc(inputFile);
    fprintf(outputFile, "StrConst(%s)\n", strConst);

    free(strConst);
    return ch;
}
