#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>

using namespace std;

vector<string> tokens;
vector<string> variables;
stack<string> numbers;
int lineNo = 0;
int tempNo = 1;
int whileNo = 1;
int ifNo = 1;
string allocateString = "";
string storeDefaultString = "";
string normalExpressions = "";
bool hasError = false;
int openParanthesis = 0;
int curlyBracket = 0;

void printError() {
	hasError = true;
	cout << "Line " << lineNo << ": syntax error" << endl;
}

void deleteEdgeSpaces(string& str) {
	int firstCharIndex = 0;
	int lastCharIndex = str.length()-1;
	for(int i=0; i<str.length(); i++) {
		if(str[i] != ' ' && firstCharIndex == 0) {
			firstCharIndex = i;
			break;
		}
	}
	for(int j=str.length()-1; j>=0; j--) {
		if(str[j] != ' ' && lastCharIndex == str.length()-1) {
			lastCharIndex = j;
			break;
		}
	}
	
	str = str.substr(firstCharIndex, lastCharIndex - firstCharIndex+1);
}

bool isOperator(const string str) {
	return (str == "*" || str == "/" || str == "+" || str == "-");
}

bool isValidChar(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

bool is_number(const string& s) {
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool hasArithmeticOperations(string &line) {
	return !(line.find("+") == string::npos && line.find("-") == string::npos && line.find("*") == string::npos && line.find("/") == string::npos);
}

bool letterCheck(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool variableCheck(string str) {
	deleteEdgeSpaces(str);
	if(!letterCheck(str[0])) {
		return false;
	}
	for(int i=1; i<str.length(); i++) {
		if(!isValidChar(str[i])) {
			return false;
		}
	}
	return true;
}

bool chooseCheck(string str) {
    if(str.length() < 6) {
        return false;
    } else if(str.substr(0, 6) == "choose") {
        string tempVar = str.substr(6);
        int openParanthesisIndex = tempVar.find("(");
        if(openParanthesisIndex == -1) {
            return false;
        } else {
            for(int i=0; i<openParanthesisIndex; i++) {
                if(tempVar[i] != ' ') {
                    return false;
                }
            }
        }
        int commaIndex = tempVar.find(",");
        if(commaIndex == -1) {
            return false;
        }
        string first = tempVar.substr(openParanthesisIndex+1, commaIndex - openParanthesisIndex - 1);
        if(expressionCheck(first)) {
            tempVar = tempVar.substr(commaIndex+1);
            commaIndex = tempVar.find(",");
            if(commaIndex == -1) {
                return false;
            }
            string second = tempVar.substr(0, commaIndex);
            if(expressionCheck(second)) {
                tempVar = tempVar.substr(commaIndex+1);
                commaIndex = tempVar.find(",");
                if(commaIndex == -1) {
                    return false;
                }
                string third = tempVar.substr(0, commaIndex);
                if(expressionCheck(third)) {
                    string forth = tempVar.substr(commaIndex+1);
                    tempVar = forth;
                    int closeParanthesisIndex = forth.find_last_of(")");
                    if(closeParanthesisIndex != -1) {
                        forth = forth.substr(0, closeParanthesisIndex);
                        if(expressionCheck(forth)) {
                            if(closeParanthesisIndex == tempVar.length()-1) {
                                return true;
                            }
                        }
                    }
                }
            } 
        } 
    }
    return false;
}

bool factorCheck(string str) {
	if(chooseCheck(str)) {
		return true;
	} else {		
		string tempVar = str;
		for(int i=0; i<tempVar.length(); i++) {
			if(tempVar[i] == '(') {
				if(str[tempVar.length()-1] != ')') {
					return false;
				} else {
					str = str.substr(1, str.length()-2);
				}
			} else {
				break;
			}
		}
		if(variableCheck(str)) {
			return true;
		} else if(is_number(str)) {
			return true;
		} else {
			return false;
		}
	}
}

bool expressionCheck(string str) {
	deleteEdgeSpaces(str);
	if(factorCheck(str)) {
		return true;
	} else if(hasArithmeticOperations(str)) {
		str = convertToPostfix(str);
		return true;
	} else {
		return false;
	}
}

int main(int argc, char* argv[]) {
	ifstream infile;
	infile.open(argv[1]);

	ofstream outfile;
	outfile.open(argv[2]);

	outfile << "; ModuleID = 'mylang2ir'";
	outfile << "\ndeclare i32 @printf(i8*, ...)";
	outfile << "\n@print.str = constant [4 x i8] c\"%d\\0A\\00\"";
	outfile << "\n\ndefine i32 @main() {\n";

	string token;

	while(getline(infile, token)) {
		tokens.push_back(token);	
	}

	for(int k=0; k<tokens.size() && !hasError; k++) {
		lineNo++;
		string line = tokens[k];
		
		
	}

	outfile << allocateString;
	outfile << "\n" + storeDefaultString;
	outfile << "\n" + normalExpressions;
	outfile << "\n\tret i32 0\n}";

	infile.close();
	outfile.close();
    return 0;
}
