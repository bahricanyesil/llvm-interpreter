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
int chooseNo = 0;

void printError() {
	hasError = true;
	cout << "Line " << lineNo << ": syntax error" << endl;
}

string spaceDeleter(string str) {
	string::iterator end_pos = remove(str.begin(), str.end(), ' ');
	str.erase(end_pos, str.end());
	string::iterator end_pos2 = remove(str.begin(), str.end(), '	');
	str.erase(end_pos2, str.end());
	return str;
}


bool is_number(const string& s) {
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool shouldAllocate(const string& s) {
	bool isNumber = false;
	string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    isNumber = !s.empty() && it == s.end();
    if(isNumber) {
    	return false;
    } else if(spaceDeleter(s) == "(" || spaceDeleter(s) == ")") {
    	return false;
    } else {
    	return true;
    }
}

bool hasArithmeticOperations(string &line) {
	return !(line.find("+") == string::npos && line.find("-") == string::npos && line.find("*") == string::npos && line.find("/") == string::npos);
}

bool findVar(const string& str) {
	return find(variables.begin(), variables.end(), str) != variables.end();
}

int precedence(char c) {
	if(c == '*' || c == '/') {
		return 2;
	} else if(c == '+' || c == '-') {
		return 1;
	} else {
		return -1;
	}
}

int isValidChar(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

bool isChar(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isValidVarName(const string str) {
	if(str.find(" ") != string::npos || !isChar(str[0])) {
		return false;
	}
	for(int i=0; i<str.length(); i++) {
		if(!isValidChar(str[i])) {
			return false;
		}
	}
	return true;
}

void deleteEdgeSpaces(string& str) {
	int firstChar = 0;
	int secondChar = str.length()-1;
	for(int i=0; i<str.length(); i++) {
		if(str[i] != ' ' && firstChar == -1 && secondChar == -1) {
			firstChar = i;
		} else if(str[i] != ' ' && firstChar != -1) {
			secondChar = i;
		}
	}
	str = str.substr(firstChar, secondChar-firstChar+1);
}

string convertToPostfix(string str) {
	stack<char> st;
	st.push('N');
	int length = str.length();
	string newString;

	for(int i=0; i<length; i++) {
		if(isValidChar(str[i])) {
			newString += str[i];
			if(i+1 < str.length()) {
				if(!isValidChar(str[i+1])) {
					newString += " ";
				}
			}
		} else if(str[i] == '(') {
			st.push('(');
		} else if(str[i] == ')') {
			while(st.top() != 'N' && st.top() != '(') {
				char c = st.top();
				st.pop();
				newString += c;
				newString += " ";
			}
			if(st.top() == '(') {
				char c = st.top();
				st.pop();
			}
		} else {
			while(st.top() != 'N' && precedence(str[i]) <= precedence(st.top())) {
				char c = st.top();
				st.pop();
				if(newString.substr(newString.length()-1) != " ") {
					newString += " ";
				}
				newString = newString + c + " ";
			}
			st.push(str[i]);
		}
	}

	while(st.top() != 'N') {
		char c = st.top();
		st.pop();
		if(newString.substr(newString.length()-1) != " ") {
			newString += " ";
		}
		newString = newString + c + " ";
	}

	return newString;
}

void writeToAllocateString(string& str, string& allocateStr, string& storeDefaultString) {
	if(!findVar(str) && shouldAllocate(str)) {
		allocateStr += "\n\t%" + str + " = alloca i32";
		storeDefaultString += "\n\tstore i32 0, i32* %" + str;
		variables.push_back(str);
	}
}

void writeToStoreString(string str1, string str2, string& normalExpressions) {
	normalExpressions += "\n\tstore i32 " + str2 + ", i32* %" + str1;
}

void writeAddition(string& normalExpressions, string& num1, string& num2, string evalOperator) {
	string firstTemp = num1;
	string secondTemp = num2;
	if(!is_number(num1)) {
		if(num1.find("%t") == string::npos) {
			firstTemp = "%t" + to_string(tempNo++);
			normalExpressions = normalExpressions + "\n\t" + firstTemp + " = load i32* %" + num1;
		} else {
			firstTemp = num1;
		}
	}
	if(!is_number(num2)) {
		if(num2.find("%t") == string::npos) {
			secondTemp = "%t" + to_string(tempNo++);
			normalExpressions = normalExpressions + "\n\t" + secondTemp + " = load i32* %" + num2;
		} else {
			secondTemp = num2;
		}
	}
	string newTemp = "%t" + to_string(tempNo++);
	normalExpressions = normalExpressions + "\n\t" + newTemp + " = " + evalOperator + " i32 " + secondTemp + ", " + firstTemp;
	numbers.push(newTemp);
}

void calculator(string& str2, string& normalExpressions) {
	int spaceIndex = str2.find(" ");
	int operatorNumber = 0;
	int varNum = 0;
	while(spaceIndex != -1 && !hasError) {
		string firstPart = str2.substr(0, spaceIndex);
		if(firstPart == "+" || firstPart == "-" || firstPart == "*" || firstPart == "/") {
			operatorNumber++;
			string num1 = numbers.top();
			numbers.pop();
			string num2 = numbers.top();
			numbers.pop();
			if(firstPart == "+") {
				writeAddition(normalExpressions, num1, num2, "add");
			} else if(firstPart == "-") {
				writeAddition(normalExpressions, num1, num2, "sub");
			} else if(firstPart == "*") {
				writeAddition(normalExpressions, num1, num2, "mul");
			} else if (firstPart == "/") {
				writeAddition(normalExpressions, num1, num2, "udiv");
			}
		} else {
			if(!isValidVarName(firstPart) && !is_number(firstPart)) {
				printError();
			}
			varNum++;
			writeToAllocateString(firstPart,allocateString,storeDefaultString);
			numbers.push(firstPart);
		}
		if(spaceIndex == str2.length()-1) {
			break;
		}
		str2 = str2.substr(spaceIndex+1);
		spaceIndex = str2.find(" ");
	}
	if(str2 == "+" || str2 == "-" || str2 == "*" || str2 == "/") {
		operatorNumber++;
	}
	if(varNum -1 != operatorNumber) {
		printError();
	}
}

void printHandler(string line, string& normalExpressions) {
	int firstIndex = line.find("(");
	int secondIndex = line.find_last_of(")");
	string tempPrint = line.substr(firstIndex + 1, secondIndex-firstIndex-1);
	if(!is_number(tempPrint)) {
		if(hasArithmeticOperations(tempPrint)) {
			tempPrint = convertToPostfix(tempPrint);
			calculator(tempPrint, normalExpressions);
			tempPrint = "%t" + to_string(tempNo-1);
		} else {
			if(!findVar(tempPrint)) {
				writeToAllocateString(tempPrint, allocateString, storeDefaultString);
			}
			string tempVar = "%t" + to_string(tempNo++);
			normalExpressions = normalExpressions + "\n\t" + tempVar + " = load i32* %" + tempPrint;
			tempPrint = tempVar;
		}
	}
	normalExpressions = normalExpressions + "\n\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 " + tempPrint + " )";
}

void assignmentHandler(string& str1, string& str2, string& normalExpressions) {
	if(!is_number(str2)) {
		string newTempT = "%t" + to_string(tempNo++);
		normalExpressions = normalExpressions + "\n\t" + newTempT + " = load i32* %" + str2;
		writeToStoreString(str1, newTempT, normalExpressions);
	} else {
		writeToStoreString(str1, str2, normalExpressions);
	}
}

void conditionHandler(string& condition, string& normalExpressions, string type) {
	if(condition.empty()){
		cout << "Line " << lineNo << ": " << "syntax error" << endl;
		return;
	}
	string equality = "ne";
	if(type == "choose") {
		equality = "eq";
	} else if(type == "chooseGreater") {
		equality = "sgt";
	} else if (type == "chooseLess") {
		equality = "slt";
	}
	while(!numbers.empty()) {
		numbers.pop();
	}
	if(!hasArithmeticOperations(condition)){
		if(!findVar(condition)) {
			writeToAllocateString(condition, allocateString, storeDefaultString);
		}
		if(is_number(condition)) {
			normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo) +" = icmp " + equality + " i32 "+condition+", 0";
		} else {
			normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo++) +" = load i32* %"+condition;
			normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo) +" = icmp " + equality + " i32 %t"+to_string(tempNo-1)+", 0";
		}	
	}
	else{
		condition = convertToPostfix(condition);
		calculator(condition,normalExpressions);
		normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo) +" = icmp " + equality + " i32 %t"+to_string(tempNo-1)+", 0";
	}
}

void whileHandler(string line, string& normalExpressions) {
	normalExpressions = normalExpressions + "\n\tbr label %whcond" + to_string(whileNo) + "\n\nwhcond" + to_string(whileNo) + ":";
	int firstIndex = line.find("(");
	int secondIndex = line.find_last_of(")");
	int bracketIndex = line.find("{");

	if(firstIndex==string::npos || secondIndex==string::npos || bracketIndex==string::npos){
		cout << "Line " << lineNo << ": " << "syntax error" << endl;
		return ;
	}

	string condition = line.substr(firstIndex+1, secondIndex-firstIndex-1);
	
	conditionHandler(condition,normalExpressions, "while");
	normalExpressions=normalExpressions+"\n\tbr i1 %t" + to_string(tempNo) +", label %whbody" + to_string(whileNo) + ", label %whend" + to_string(whileNo);
	normalExpressions=normalExpressions+"\n\nwhbody" + to_string(whileNo) + ":";
	tempNo++;
}

void ifHandler(string line, string& normalExpressions, string type){
	normalExpressions = normalExpressions + "\n\tbr label %ifcond" + to_string(ifNo) + "\n\nifcond" + to_string(ifNo) + ":";
	int firstIndex = line.find("(");
	int secondIndex = line.find_last_of(")");
	int bracketIndex = line.find("{");

	if(firstIndex==string::npos || secondIndex==string::npos || bracketIndex==string::npos){
		cout << "Line " << lineNo << ": " << "syntax error" << endl;
		return ;
	}
	
	string condition = line.substr(firstIndex+1, secondIndex-firstIndex-1);

	conditionHandler(condition,normalExpressions, type);
	normalExpressions=normalExpressions+"\n\tbr i1 %t" + to_string(tempNo) +", label %ifbody" + to_string(ifNo) + ", label %ifend" + to_string(ifNo);
	normalExpressions=normalExpressions+"\n\nifbody" + to_string(ifNo) + ":";
	tempNo++;
}

void chooseHandler(string line, string& normalExpressions){
	int firstIndex = line.find("(");
	int secondIndex = line.find_last_of(")");
	string variableLine = line.substr(firstIndex+1, secondIndex-firstIndex-1);
	spaceDeleter(variableLine);
	string var1 = variableLine.substr(0,variableLine.find(","));
	variableLine = variableLine.substr(variableLine.find(",")+1);
	string var2 = variableLine.substr(0,variableLine.find(","));
	variableLine = variableLine.substr(variableLine.find(",")+1);
	string var3 = variableLine.substr(0,variableLine.find(","));
	variableLine = variableLine.substr(variableLine.find(",")+1);
	string var4 = variableLine;

	var1= "(" + var1 + ") {";
	ifHandler(var1, normalExpressions, "choose");
	if(hasArithmeticOperations(var2)) {
		var2 = convertToPostfix(var2);
		calculator(var2, normalExpressions);
		writeToStoreString("c"+to_string(chooseNo), "%t" + to_string(tempNo-1), normalExpressions);
	} else {
		writeToStoreString("c"+to_string(chooseNo), var2, normalExpressions);
	}

	normalExpressions = normalExpressions + "\n\tbr label %ifend" + to_string(ifNo);
	normalExpressions = normalExpressions + "\n\nifend" + to_string(ifNo) + ":\n";
	ifNo++;

	ifHandler(var1, normalExpressions, "chooseGreater");
	if(hasArithmeticOperations(var3)) {
		var3 = convertToPostfix(var3);
		calculator(var3, normalExpressions);
		writeToStoreString("c"+to_string(chooseNo), "%t" + to_string(tempNo-1), normalExpressions);
	} else {
		writeToStoreString("c"+to_string(chooseNo), var3, normalExpressions);
	}

	normalExpressions = normalExpressions + "\n\tbr label %ifend" + to_string(ifNo);
	normalExpressions = normalExpressions + "\n\nifend" + to_string(ifNo) + ":\n";
	ifNo++;

	ifHandler(var1, normalExpressions, "chooseLess");
	if(hasArithmeticOperations(var4)) {
		var4 = convertToPostfix(var4);
		calculator(var4, normalExpressions);
		writeToStoreString("c"+to_string(chooseNo), "%t" + to_string(tempNo-1), normalExpressions);
	} else {
		writeToStoreString("c"+to_string(chooseNo), var4, normalExpressions);
	}

	normalExpressions = normalExpressions + "\n\tbr label %ifend" + to_string(ifNo);
	normalExpressions = normalExpressions + "\n\nifend" + to_string(ifNo) + ":\n";
	ifNo++;
}

void expressionHandler(string line, string& normalExpressions, string& storeDefaultString, string& allocateString, int found) {
	while(!numbers.empty()) {
		numbers.pop();
	}
	string str1 = line.substr(0, found);
	string str2 = line.substr(found+1);
	deleteEdgeSpaces(str1);
	deleteEdgeSpaces(str2);
	if(!isValidVarName(str1)) {
		hasError = true;
		cout << "Line " << lineNo << ": syntax error" << endl;
		return;
	}
	int chooseIndex = str2.find("choose");
	if(!isValidVarName(str2) && !is_number(str2) && !hasArithmeticOperations(str2) && chooseIndex == -1) {
		hasError = true;
		cout << "Line " << lineNo << ": syntax error" << endl;
		return;
	}
	if(chooseIndex != -1) {
		string tempChoose = str2.substr(chooseIndex);
		int openIndex = 0;
		int closeIndex = tempChoose.length()-1;
		int openParanthesisNum = 0;
		for(int t=0; t<tempChoose.length()-1; t++) {
			if(tempChoose[t] == '(') {
				if(openIndex == 0) {
					openIndex = t;
				}
				openParanthesisNum++;
			}
			if(tempChoose[t] == ')') {
				if(openParanthesisNum == 1) {
					closeIndex = t;
				}
				openParanthesisNum--;
			}
		}
		tempChoose = tempChoose.substr(openIndex, closeIndex-openIndex+1);
		chooseHandler(tempChoose, normalExpressions);
		str2 = str2.substr(0, chooseIndex) + "c"+to_string(chooseNo++) + str2.substr(closeIndex+chooseIndex+1);
	}
	str2 = convertToPostfix(str2);
	str1 = spaceDeleter(str1);
	writeToAllocateString(str1, allocateString, storeDefaultString);
	if(!hasArithmeticOperations(line)) {
		if(!findVar(str2) && str2[0] != '%') {
			writeToAllocateString(str2, allocateString, storeDefaultString);
		}
		assignmentHandler(str1, str2, normalExpressions);
	} else {
		calculator(str2, normalExpressions);
		writeToStoreString(str1, "%t" + to_string(tempNo-1), normalExpressions);
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

	bool inWhileBody = false;
	bool inIfBody = false;
	for(int k=0; k<tokens.size() && !hasError; k++) {
		lineNo++;
		string line = tokens[k];
		string::iterator end_pos = remove(line.begin(), line.end(), ' ');
		line.erase(end_pos, line.end());
		int found = line.find("=");
		int thirdFound = line.find("}");
		if(found != -1) {
			expressionHandler(line, normalExpressions, storeDefaultString, allocateString, found);
		} else if(thirdFound != -1) {
			if(inWhileBody) {
				inWhileBody = false;
				normalExpressions = normalExpressions + "\n\tbr label %whcond" + to_string(whileNo);
				normalExpressions = normalExpressions + "\n\nwhend" + to_string(whileNo) + ":\n";
				whileNo++;
				curlyBracket--;
			} else if(inIfBody) {
				inIfBody = false;
				normalExpressions = normalExpressions + "\n\tbr label %ifend" + to_string(ifNo);
				normalExpressions = normalExpressions + "\n\nifend" + to_string(ifNo) + ":\n";
				ifNo++;
				curlyBracket--;
			}
		} else {
			if(line.find("print") != string::npos) {
				printHandler(line, normalExpressions);
			} else if (line.find("while") != string::npos) {
				string tempLine = spaceDeleter(line);
				if(inWhileBody || inIfBody || tempLine.substr(0, 6) != "while(") {
					hasError = true;
					cout << "Line " << lineNo << ": syntax error" << endl;
					break;
				}
				if(inWhileBody || inIfBody) {
					hasError = true;
					cout << "Line " << lineNo << ": syntax error" << endl;
					break;
				}
				curlyBracket++;
				inWhileBody = true;
				whileHandler(line, normalExpressions);
			} else if (line.find("if") != string::npos) {
				string tempLine = spaceDeleter(line);
				if(inWhileBody || inIfBody || tempLine.substr(0, 3) != "if(") {
					hasError = true;
					cout << "Line " << lineNo << ": syntax error" << endl;
					break;
				}
				curlyBracket++;
				inIfBody = true;
				ifHandler(line,normalExpressions, "if");
			} else if (line.find("choose") != string::npos) {
				chooseHandler(line, normalExpressions); 
			} else {
				cout << "Not found" << endl;
			}
		}
	}

	if(curlyBracket != 0 || openParanthesis != 0) {
		cout << "Line " << lineNo << ": syntax error" << endl;
	}

	outfile << allocateString;
	outfile << "\n" + storeDefaultString;
	outfile << "\n" + normalExpressions;
	outfile << "\n\tret i32 0\n}";

	infile.close();
	outfile.close();
    return 0;
}
