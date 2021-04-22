#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>

using namespace std;

bool expressionCheck(string& str);
bool ifCheck(string& str);
bool whileCheck(string& str);

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


bool findVar(const string str) {
	return find(variables.begin(), variables.end(), str) != variables.end();
}

bool writeToAllocateString(string str) {
	if(!findVar(str) && variableCheck(str)) {
		allocateString += "\n\t%" + str + " = alloca i32";
		storeDefaultString += "\n\tstore i32 0, i32* %" + str;
		variables.push_back(str);
		return true;
	}
	return false;
}

void writeToStoreString(string str1, string str2) {
	normalExpressions += "\n\tstore i32 " + str2 + ", i32* %" + str1;
}

void loadToTemp(string var) {
	normalExpressions = normalExpressions + "\n\t%t" + to_string(tempNo++) + " = load i32* %" + var;
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
                    if(closeParanthesisIndex == -1) {
                        return false;
                    } else {
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

bool factorCheck(string& str) {
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

bool expressionCheck(string& str) {
	if(factorCheck(str)) {
		return true;
	} else if(hasArithmeticOperations(str)) {
		//TODO:
		return true;
	} else {
		return false;
	}
}

void assignmentHandler(string firstPart, string secondPart) {
	writeToAllocateString(firstPart);
	writeToAllocateString(secondPart);
	if(variableCheck(secondPart)) {
		loadToTemp(secondPart);
		writeToStoreString(firstPart, "%t" + to_string(tempNo-1));
	} else if(is_number(secondPart)) {
		writeToStoreString(firstPart, secondPart);
	} else if(false) {
		
	}
}

void assignmentHelper(string line) {
	int equalIndex = line.find("=");
	string firstPart = line.substr(0, equalIndex);
	string secondPart = line.substr(equalIndex+1);
	deleteEdgeSpaces(firstPart);
	deleteEdgeSpaces(secondPart);
	if(!variableCheck(firstPart) || !expressionCheck(secondPart)) {
		printError();
	} else {
		assignmentHandler(firstPart, secondPart);
	}
}

bool ifCheck(string& str){

	//deleteEdgeSpaces(str);

	if(str.length()<5)
		return false;

	if(str.substr(0,2)!="if")
		return false;

	int openParanthesisIndex=str.find("(");
	int lastCloseParanthes=str.find_last_of(")");

	if(openParanthesisIndex==-1 || lastCloseParanthes==-1)
		return false;
	
	for(int i=2;i<openParanthesisIndex;i++){
	
		if(str[i]!=' ')
			return false;
	}

	string condition=str.substr(openParanthesisIndex+1,lastCloseParanthes);
	
	if(!expressionCheck(condition))
		return false;

	int curlyBracketIndex=str.find("{");
	
	if(curlyBracketIndex==-1)
		return false;

	for(int i=lastCloseParanthes+1;i<curlyBracketIndex;i++){

		if(str[i]!=' ')
			return false;
	}

	if(curlyBracketIndex!=str.length()-1)
		return false;

	return true;
}

bool whileCheck(string& str){

	//deleteEdgeSpaces(str);

	if(str.length()<5)
		return false;

	if(str.substr(0,5)!="while")
		return false;

	int openParanthesisIndex=str.find("(");
	int lastCloseParanthes=str.find_last_of(")");

	if(openParanthesisIndex==-1 || lastCloseParanthes==-1)
		return false;
	
	for(int i=5;i<openParanthesisIndex;i++){
	
		if(str[i]!=' ')
			return false;
	}

	string condition=str.substr(openParanthesisIndex+1,lastCloseParanthes);
	
	if(!expressionCheck(condition))
		return false;

	int curlyBracketIndex=str.find("{");
	
	if(curlyBracketIndex==-1)
		return false;

	for(int i=lastCloseParanthes+1;i<curlyBracketIndex;i++){

		if(str[i]!=' ')
			return false;
	}

	if(curlyBracketIndex!=str.length()-1)
		return false;

	return true;

}

bool printCheck(string& str){

	//deleteEdgeSaces(str);
	if(str.length()<5)
		return false;

	if(str.substr(0,5)!="print")
		return false;
	

	int openParanthesisIndex=str.find("(");
	int lastCloseParanthes=str.find_last_of(")");

	if(openParanthesisIndex==-1 || lastCloseParanthes==-1)
		return false;
	
	for(int i=5;i<openParanthesisIndex;i++){
	
		if(str[i]!=' ')
			return false;
	}

	string inside=str.substr(openParanthesisIndex+1,lastCloseParanthes);

	if(!expressionCheck(inside))
		return false;

	if(lastCloseParanthes!=str.length()-1)
		return false;

	return true;

}

void conditionHandler(string& condition){

	//expressionHandler(condition);
}

void ifHandler(string& line){

	string condition=line.substr(line.find("(")+1,line.find_last_of(")"));
	normalExpressions=normalExpressions+"\nbr label \%ifcond"+to_string(ifNo);
	normalExpressions=normalExpressions+"\n\nif"+to_string(ifNo)+"cond:";
	conditionHandler(condition);
	ifNo++;
}

void whileHandler(string& line){

	string condition=line.substr(line.find("(")+1,line.find_last_of(")"));
	normalExpressions=normalExpressions+"\nbr label %whcond"+to_string(whileNo);
	normalExpressions=normalExpressions+"\n\nwh"+to_string(whileNo)+"cond:";
	conditionHandler(condition);	
	whileNo++;
	
}

void printHandler(string& line){

	string inside=line.substr(line.find("(")+1,line.find_last_of(")"));
	//expressionHandler(inside);

	normalExpressions=normalExpressions+"\ncall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 %t"+to_string(tempNo-1)+" )";
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
		deleteEdgeSpaces(line);
		int equalIndex = line.find("=");
		if(equalIndex != -1) {
			assignmentHelper(line);
		} else if(ifCheck(line)){

			ifHandler(line);
		} else if(whileCheck(line)){

			whileHandler(line);
		} else if(printCheck(line)){

			printHandler(line);
		}

		else{

			printError();
		}
	}

	outfile << allocateString;
	outfile << "\n" + storeDefaultString;
	outfile << "\n" + normalExpressions;
	outfile << "\n\tret i32 0\n}";

	infile.close();
	outfile.close();
    return 0;
}
