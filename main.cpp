#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>

using namespace std;

bool expressionCheck(string str);
void conditionHandler(string str, string type);
void chooseHandler(string str);
void calculator(string str);
void chooseArithmetic(string& str);
bool writeToAllocateString(string str);

vector<string> tokens;
vector<string> variables;
stack<string> numbers;
int lineNo = -1;
int tempNo = 1;
int whileNo = 1;
int ifNo = 1;
string allocateString = "";
string storeDefaultString = "";
string normalExpressions = "";
string errorText = "";
bool hasError = false;
int openParanthesis = 0;
int curlyBracket = 0;
bool inWhileBody = false;
bool inIfBody = false;
int inIfNo = 1;
stack<int> chooseIndexes;
vector<string> chooseStore;
int chooseNo = 0;
string lastChoose = "";

void printError() {
	if(!hasError) {
		hasError = true;
		cout << "Line " << lineNo << ": syntax error" << endl;
		errorText = "Line " + to_string(lineNo) + ": syntax error";
	}
}

void spaceDeleter(string& str) {
	string::iterator end_pos = remove(str.begin(), str.end(), ' ');
	str.erase(end_pos, str.end());
	string::iterator end_pos2 = remove(str.begin(), str.end(), '	');
	str.erase(end_pos2, str.end());
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
	str.erase(remove(str.begin(), str.end(), '\t'), str.end());
}

int nthSubstr(int n, const string& s, const string& p) {
   string::size_type i = s.find(p);

   int j;
   for (j = 1; j < n && i != string::npos; ++j)
      i = s.find(p, i+1);

   if (j == n)
     return(i);
   else
     return(-1);
}

bool isOperator(const string str) {
	return (str == "*" || str == "/" || str == "+" || str == "-");
}

bool isValidChar(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_');
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

bool findVar(string str) {
	return find(variables.begin(), variables.end(), str) != variables.end();
}

bool findChoose(string str) {
	return find(chooseStore.begin(), chooseStore.end(), str) != chooseStore.end();
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

bool checkNestedChoose(string tempVar, int& commaIndex) {
	int chooseNum = 0;
    int commaNum = 0;
    bool isChecked = false;
	for(int i=0; i<tempVar.length()-6; i++) {
		string checkTemp = tempVar.substr(i, 6);
		if(checkTemp == "choose") {
			chooseNum++;
		}
		if(tempVar[i] == ',') {
			commaIndex = i;
			isChecked = true;
			commaNum++;
		}
		if((4+3*(chooseNum-1) == commaNum && chooseNum != 0) || (chooseNum == 0 && commaNum == 1)) {
			break;
		}
	}
	if(chooseNum>0) {
		commaIndex = nthSubstr(4+3*(chooseNum-1), tempVar, ",");
		isChecked = true;
	}
	return isChecked;
}

bool chooseCheck(string str) {
    if(str.length() < 12) {
        return false;
    } else if(str.substr(0, 6) == "choose") {
        string tempVar = str.substr(6);
        deleteEdgeSpaces(tempVar);
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
        checkNestedChoose(tempVar, commaIndex);
		if(commaIndex == -1) {
            return false;
        }
        string first = tempVar.substr(openParanthesisIndex+1, commaIndex - openParanthesisIndex - 1);
        deleteEdgeSpaces(first);
        if(expressionCheck(first)) {
            tempVar = tempVar.substr(commaIndex+1);
            commaIndex = tempVar.find(",");
            if(commaIndex == -1) {
	            return false;
	        }
            checkNestedChoose(tempVar, commaIndex);
            if(commaIndex == -1) {
                return false;
            }
            string second = tempVar.substr(0, commaIndex);
            deleteEdgeSpaces(second);

            if(expressionCheck(second)) {
                tempVar = tempVar.substr(commaIndex+1);
                commaIndex = tempVar.find(",");
                if(commaIndex == -1) {
		            return false;
		        }
                checkNestedChoose(tempVar, commaIndex);
                if(commaIndex == -1) {
                    return false;
                }
                string third = tempVar.substr(0, commaIndex);
                deleteEdgeSpaces(third);
                if(expressionCheck(third)) {
                    string forth = tempVar.substr(commaIndex+1);
                    deleteEdgeSpaces(forth);
                    tempVar = forth;
                    int closedIndex = -1;
                    int openedParantNum = 0;
                    for(int i=0; i<tempVar.length(); i++) {
                    	if(tempVar[i] == '(') {
                    		openedParantNum++;
                    	}
                    	if(tempVar[i] == ')') {
                    		if(openedParantNum == 0) {
                    			closedIndex = i;
                    			break;
                    		}
                    		openedParantNum--;
                    	}
                    }
                    if(closedIndex == -1) {
                        return false;
                    } else {
                    	forth = forth.substr(0, closedIndex);
                    	deleteEdgeSpaces(forth);
                        if(expressionCheck(forth)) {
                            if(closedIndex == tempVar.length()-1) {
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
		int openIndex = str.find("(");
		while(openIndex != -1) {
			int closeIndex = str.find_last_of(")");
			if(closeIndex == -1) {
				return false;
			} else {
				str = str.substr(openIndex+1, closeIndex - openIndex -1);
				deleteEdgeSpaces(str);
				openIndex = str.find("(");
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
	if(factorCheck(str)) {
		return true;
	} else if(hasArithmeticOperations(str)) {
		return true;
	} else {
		return false;
	}
}

int precedence(char c) {
    int temp = 0;
    if(c == '+' || c == '-') {
        temp = 1;
    } else if(c == '*' || c == '/') {
        temp = 2;
    }
    return temp;
}

bool isOperator(char input) {
    if(input == '+' || input == '-' || input == '*' || input == '/') {
        return true;
    }
    return false;
}

void writeOperation(string num1, string num2, string type) {
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
	normalExpressions = normalExpressions + "\n\t" + newTemp + " = " + type + " i32 " + secondTemp + ", " + firstTemp;
	numbers.push(newTemp);
}

string convertToPostfix(string& str) {
	stack<char> st;
	st.push('E');
    string newString;
    int operatorNum = 0;
    int variableNum = 0;
    deleteEdgeSpaces(str);
    bool spaceFound = false;
    bool afterOperator = false;
    for(int i = 0; i < str.length() && !hasError; i++) {
       if(isValidChar(str[i])) {
       		if(spaceFound && !afterOperator) {
       			printError();
       			return "";
       		}
			newString += str[i];
			if(afterOperator) {
				afterOperator = false;
				spaceFound = false;
			}
		} else if(str[i] == '(') {
			st.push('(');
			spaceFound = false;
            afterOperator = true;
		} else if(str[i] == ')') {
			while(st.top() != 'E' && st.top() != '(') {
               char c = st.top();
               st.pop();
               newString = newString + " " + c + " ";
            }
            if(st.top() == '(') {
                char c = st.top();
                st.pop();
            }
            spaceFound = false;
            afterOperator = true;
		} else if(isOperator(str[i])) {
			newString += " ";
			spaceFound = false;
			afterOperator = true;
			while(st.top() != 'E' && precedence(str[i]) <= precedence(st.top())) {
                char c = st.top();
                st.pop();
                newString = newString + " " + c + " ";
            }
            st.push(str[i]);
		} else if(str[i] == ' ') {
			spaceFound = true;
		} else {
			printError();
			return "";
		}
	}

	while(st.top() != 'E') {
		char c = st.top();
		st.pop();
		newString = newString + " " + c + " ";
	}
	string::size_type pos;
	while((pos = newString.find("  ")) != string::npos) {
	  newString.replace(pos, 2, " ");
	}

	string temp = newString;
	int spaceIndex = temp.find(" ");
	while(spaceIndex != -1) {
		string var = temp.substr(0, spaceIndex);
		if(isOperator(var)) {
			operatorNum++;
		} else if(variableCheck(var) || is_number(var)) {
			variableNum++;
		} else {
			printError();
			return "";
		}
		temp = temp.substr(spaceIndex+1);
		spaceIndex = temp.find(" ");
	}
	if(variableNum-1 != operatorNum) {
		printError();
		return "";
	}
	return newString;
}

void calculator(string str) {
	while(!numbers.empty()) {
		numbers.pop();
	}
	string tempVar = str;
	int spaceIndex = tempVar.find(" ");
	deleteEdgeSpaces(str);
	while(spaceIndex != -1) {
		string variable = tempVar.substr(0, spaceIndex);
		if(isOperator(variable)) {
			string num1 = numbers.top();
			numbers.pop();
			string num2 = numbers.top();
			numbers.pop();
			if(variable == "+") {
				writeOperation(num1, num2, "add");
			} else if(variable == "-") {
				writeOperation(num1, num2, "sub");
			} else if(variable == "*") {
				writeOperation(num1, num2, "mul");
			} else if (variable == "/") {
				if(num1 == "0") {
					printError();
					return;
				}
				writeOperation(num1, num2, "udiv");
			}
		} else if (is_number(variable) || variableCheck(variable)) {
			writeToAllocateString(variable);
			numbers.push(variable);
		} else {
			printError();
			return;
		}
		if(spaceIndex == tempVar.length()-1) {
			break;
		}
		tempVar = tempVar.substr(spaceIndex+1);
		spaceIndex = tempVar.find(" ");
	}
}

void conditionHandler(string str, string type) {
	deleteEdgeSpaces(str);
	string equality = "ne";
	if(type == "choose") {
		equality = "eq";
	} else if(type == "chooseGreater") {
		equality = "sgt";
	} else if (type == "chooseLess") {
		equality = "slt";
	}
	if(variableCheck(str)) {
		writeToAllocateString(str);
		normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo++) +" = load i32* %" + str;
		normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo) +" = icmp " + equality + " i32 %t"+to_string(tempNo-1)+", 0";
	} else if(is_number(str)) {
		normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo) +" = icmp " + equality + " i32 "+ str +", 0";
	} else if(chooseCheck(str)) {
		if(findChoose(str)) {
			normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo++) +" = load i32* %" + "mylocalsupervar" +to_string(chooseNo);
			normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo) +" = icmp " + equality + " i32 %t" + to_string(tempNo-1) +", 0";
		} else {
			chooseStore.push_back(str);
			chooseHandler(str);
			normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo++) +" = load i32* %" + "mylocalsupervar" +to_string(chooseNo);
			normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo) +" = icmp " + equality + " i32 %t" + to_string(tempNo-1) +", 0";
		}
	} else if(hasArithmeticOperations(str)) {
		chooseArithmetic(str);
		str = convertToPostfix(str);
		if(hasError) {
			return;
		}
		calculator(str);
		normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo) +" = icmp " + equality + " i32 %t"+to_string(tempNo-1)+", 0";
	} else {
		printError();
	}
}

bool ifWhileCheck(string& str, string type) {
	deleteEdgeSpaces(str);
	int length = 4;
	int startIndex = 2;
	if(type == "while") {
		length = 7;
		startIndex = 5;
	}

	if((str.length() < length) || (type == "while" && str.substr(0,5) != "while") || (type == "if" && str.substr(0,2) != "if")) {
		return false;
	}

	int openParanthesisIndex = str.find("(");
	int lastCloseParanthes = str.find_last_of(")");

	if(openParanthesisIndex == -1 || lastCloseParanthes == -1) {
		return false;
	}
	
	for(int i=startIndex; i<openParanthesisIndex; i++){
		if(str[i] != ' ') {
			return false;
		}
	}

	string condition = str.substr(openParanthesisIndex+1, lastCloseParanthes - openParanthesisIndex-1);
	int curlyBracketIndex = str.find("{");
	deleteEdgeSpaces(condition);
	if(!expressionCheck(condition) || curlyBracketIndex == -1) {
		return false;
	}

	for(int i=lastCloseParanthes+1; i<curlyBracketIndex; i++){
		if(str[i] != ' ') {
			return false;
		}
	}

	if(curlyBracketIndex != str.length()-1) {
		return false;
	}

	return true;
}

void ifWhileHandler(string line, string type){
	int no = ifNo;
	string cond = "ifcond";
	string body = "ifbody";
	string end = "ifend";
	if(type == "while") {
		no = whileNo;
		cond = "whcond";
		body = "whbody";
		end = "whend";
	}
	int openIndex = line.find("(") + 1;
	int closeIndex = line.find_last_of(")");
	string condition = line.substr(openIndex, closeIndex - openIndex);
	normalExpressions = normalExpressions+"\n\tbr label %" + cond + to_string(no);
	normalExpressions = normalExpressions+"\n\n" + cond + to_string(no) + ":";
	if(type != "while") {
		chooseIndexes.push(ifNo);
		ifNo++;
	}
	conditionHandler(condition, type);
	if(type != "while") {
		no = chooseIndexes.top();
		chooseIndexes.pop();
		inIfNo = no;
	}
	normalExpressions = normalExpressions+"\n\tbr i1 %t" + to_string(tempNo++) +", label %" + body + to_string(no) + ", label %" + end + to_string(no);
	normalExpressions = normalExpressions+"\n\n" + body + to_string(no) + ":";
}

bool printCheck(string& str) {
	deleteEdgeSpaces(str);
	if(str.length() < 7) {
        return false;
    } else if(str.substr(0, 5) == "print") {
        str = str.substr(5);
        deleteEdgeSpaces(str);
        int openParanthesisIndex = str.find("(");
        if(openParanthesisIndex == -1) {
            return false;
        } else {
            for(int i=0; i<openParanthesisIndex; i++) {
                if(str[i] != ' ') {
                    return false;
                }
            }
        }
        int closeParanthesisIndex = str.find_last_of(")");
        if(closeParanthesisIndex == -1) {
        	return false;
        } else {
        	for(int k=closeParanthesisIndex+1; k<str.length(); k++) {
	        	if(str[k] != ' ') {
	        		return false;
	        	}
	        }
	        str = str.substr(openParanthesisIndex+1, closeParanthesisIndex - openParanthesisIndex -1);
	        deleteEdgeSpaces(str);
	        if(expressionCheck(str)) {
	        	return true;
	        } else {
	        	return false;
	        }
        }
    }
    return false;
}

void chooseIfHelper(string var1, string var2, string type) {
	int no = ifNo;
	ifNo++;
	writeToAllocateString("mylocalsupervar" + to_string(chooseNo));
	normalExpressions = normalExpressions+"\n\tbr label %ifcond" + to_string(no);
	normalExpressions = normalExpressions+"\n\nifcond" + to_string(no) + ":";
	conditionHandler(var1, type);
	normalExpressions = normalExpressions+"\n\tbr i1 %t" + to_string(tempNo++) +", label %ifbody" + to_string(no) + ", label %ifend" + to_string(no);
	normalExpressions = normalExpressions+"\n\nifbody" + to_string(no) + ":";
	if(variableCheck(var2)) {
		writeToAllocateString(var2);
		loadToTemp(var2);
		writeToStoreString("mylocalsupervar" + to_string(chooseNo), "%t"+to_string(tempNo-1));
	} else if (is_number(var2)) {
		writeToStoreString("mylocalsupervar" + to_string(chooseNo), var2);
	}  else if(chooseCheck(var2)) {
		chooseHandler(var2);
		normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo++) +" = load i32* %" + "mylocalsupervar" +to_string(chooseNo);
	}  else if(hasArithmeticOperations(var2)) {
		chooseArithmetic(var2);
		var2 = convertToPostfix(var2);
		if(hasError) {
			return;
		}
		calculator(var2);
		writeToStoreString("mylocalsupervar" + to_string(chooseNo), "%t" + to_string(tempNo-1));
	} else {
		printError();
		return;
	}

	normalExpressions = normalExpressions + "\n\tbr label %ifend" + to_string(no);
	normalExpressions = normalExpressions + "\n\nifend" + to_string(no) + ":\n";
}

void chooseHandler(string line) {
	int commaIndex = line.find(",");
	int firstParanthesis = line.find("(");
	int lastParanthesis = line.find_last_of(")");
	line = line.substr(firstParanthesis+1, lastParanthesis - firstParanthesis);
    checkNestedChoose(line, commaIndex);
    string first = line.substr(0, commaIndex);

    line = line.substr(commaIndex+1);
    if(!checkNestedChoose(line, commaIndex)) {
    	commaIndex = line.find(",");
    }
    string second = line.substr(0, commaIndex);
    deleteEdgeSpaces(first);
    deleteEdgeSpaces(second);
	chooseIfHelper(first, second, "choose");
	if(hasError) {
		return;
	}
    
    line = line.substr(commaIndex+1);
    if(!checkNestedChoose(line, commaIndex)) {
    	commaIndex = line.find(",");
    }
    string third = line.substr(0, commaIndex);
    deleteEdgeSpaces(third);
	chooseIfHelper(first, third, "chooseGreater");
	if(hasError) {
		return;
	}

    line = line.substr(commaIndex+1);
    string forth = line.substr(0, line.length()-1);
    deleteEdgeSpaces(forth);
    chooseIfHelper(first, forth, "chooseLess");
}

void printHandler(string line) {
	spaceDeleter(line);
	if(variableCheck(line)) {
		writeToAllocateString(line);
		loadToTemp(line);
		normalExpressions = normalExpressions + "\n\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 %t" + to_string(tempNo-1) + " )";
	} else if(is_number(line)) {
		normalExpressions = normalExpressions + "\n\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 " + line + " )";
	} else if(chooseCheck(line)) {
		chooseHandler(line);
		normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo++) +" = load i32* %" + "mylocalsupervar" +to_string(chooseNo);
		normalExpressions = normalExpressions + "\n\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 %t" + to_string(tempNo-1) + " )";
	} else if(hasArithmeticOperations(line)) {
		chooseArithmetic(line);
		line = convertToPostfix(line);
		if(hasError) {
			return;
		}
		calculator(line);
		normalExpressions = normalExpressions + "\n\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 %t" + to_string(tempNo-1) + " )";
	} else {
		printError();
	}
}

void chooseArithmetic(string& secondPart) {
	int chooseIndex = secondPart.find("choose");
	int opened = 0;
	int closedIndex = 0;
	bool isEntered = false;
	while(chooseIndex != -1) {
		for(int i=chooseIndex; i<secondPart.length(); i++) {
			if(secondPart[i] == '(') {
				opened++;
				isEntered = true;
			}
			if(secondPart[i] == ')') {
				opened--;
			}
			if(isEntered && opened == 0) {
				closedIndex = i;
				break;
			}
		}
		isEntered = false;
		string chooseContent = secondPart.substr(chooseIndex, closedIndex - chooseIndex+1);
		chooseHandler(chooseContent);
		int max = secondPart.length();
		string willAdd = "";
		if(closedIndex < max-1) {
			willAdd = secondPart.substr(closedIndex+1);
		}
		secondPart = secondPart.substr(0, chooseIndex) + "mylocalsupervar" + to_string(chooseNo) + willAdd;
		chooseIndex = secondPart.find("choose");
	}
}

void assignmentHandler(string firstPart, string secondPart) {
	spaceDeleter(firstPart);
	spaceDeleter(secondPart);
	writeToAllocateString(firstPart);
	writeToAllocateString(secondPart);
	if(variableCheck(secondPart)) {
		writeToAllocateString(secondPart);
		loadToTemp(secondPart);
		writeToStoreString(firstPart, "%t" + to_string(tempNo-1));
	} else if(is_number(secondPart)) {
		writeToStoreString(firstPart, secondPart);
	} else if(chooseCheck(secondPart)) {
		chooseHandler(secondPart);
		normalExpressions=normalExpressions+"\n\t%t"+to_string(tempNo++) +" = load i32* %" + "mylocalsupervar" +to_string(chooseNo);
		writeToStoreString(firstPart, "%t"+to_string(tempNo-1));
	} else if(hasArithmeticOperations(secondPart)) {
		chooseArithmetic(secondPart);
		secondPart = convertToPostfix(secondPart);
		if(hasError) {
			return;
		}
		calculator(secondPart);
		writeToStoreString(firstPart, "%t" + to_string(tempNo-1));
	} else {
		printError();
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

int main(int argc, char* argv[]) {
	ifstream infile;
	infile.open(argv[1]);

	ofstream outfile;
	outfile.open(argv[2]);

	string token;

	while(getline(infile, token)) {
		tokens.push_back(token);	
	}
	lastChoose = "";

	for(int k=0; k<tokens.size() && !hasError; k++) {
		lineNo++;
		string line = tokens[k];
		deleteEdgeSpaces(line);
		int commentIndex = line.find("#");
		if(commentIndex != -1) {
			line = line.substr(0, commentIndex);
		}
		int equalIndex = line.find("=");
		int printIndex = line.find("print");
		if(line == "") {
			continue;
		}
		if(equalIndex != -1) {
			assignmentHelper(line);
		} else if(ifWhileCheck(line, "if")){
			if(inWhileBody || inIfBody) {
				printError();
				break;
			}
			inIfBody = true;
			curlyBracket++;
			ifWhileHandler(line, "if");
		} else if(ifWhileCheck(line, "while")){
			if(inWhileBody || inIfBody) {
				printError();
				break;
			}
			inWhileBody = true;
			curlyBracket++;
			ifWhileHandler(line, "while");
		} else if(printCheck(line)){
			printHandler(line);
		} else if(line == "}") {
			if(inWhileBody) {
				inWhileBody = false;
				normalExpressions = normalExpressions + "\n\tbr label %whcond" + to_string(whileNo);
				normalExpressions = normalExpressions + "\n\nwhend" + to_string(whileNo) + ":\n";
				whileNo++;
				curlyBracket--;
			} else if(inIfBody) {
				inIfBody = false;
				normalExpressions = normalExpressions + "\n\tbr label %ifend" + to_string(inIfNo);
				normalExpressions = normalExpressions + "\n\nifend" + to_string(inIfNo) + ":\n";
				curlyBracket--;
			} else {
				printError();
				break;
			}
		} else {
			printError();
		}
	}

	if(curlyBracket != 0 && !hasError) {
		printError();
	}

	if(!hasError) {
		outfile << "; ModuleID = 'mylang2ir'";
		outfile << "\ndeclare i32 @printf(i8*, ...)";
		outfile << "\n@print.str = constant [4 x i8] c\"%d\\0A\\00\"";
		outfile << "\n\ndefine i32 @main() {\n";
		outfile << allocateString;
		outfile << "\n" + storeDefaultString;
		outfile << "\n" + normalExpressions;
		outfile << "\n\tret i32 0\n}";
	} else {
		outfile << errorText << endl;
	}

	infile.close();
	outfile.close();
    return 0;
}
