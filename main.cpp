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

// Tokens vector to read the lines from the input file.
vector<string> tokens;
// Variables vector to store the allocated variables.
vector<string> variables;
// Numbers stack to store the numbers while doing calculations.
stack<string> numbers;
// Number of line, temporary variables, while and if loops.
int lineNo = -1;
int tempNo = 1;
int whileNo = 1;
int ifNo = 1;
// Strings will be written to the outputfile. Stores the allocation and other texts.
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

// Prints the error when the program encounters with it.
void printError() {
	if(!hasError) {
		hasError = true;
		cout << "Line " << lineNo << ": syntax error" << endl;
		errorText = "Line " + to_string(lineNo) + ": syntax error";
	}
}

// Deletes the all spaces and tabs in a string.
void spaceDeleter(string& str) {
	string::iterator end_pos = remove(str.begin(), str.end(), ' ');
	str.erase(end_pos, str.end());
	string::iterator end_pos2 = remove(str.begin(), str.end(), '	');
	str.erase(end_pos2, str.end());
}

// Deletes the spaces at the both end of a stirng. Does not delete the middle ones.
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

// Finds the nth substring of a string in another string if it contains.
int nthSubstr(int n, const string& s, const string& p) {
   string::size_type i = s.find(p);

   int j;
   for (j = 1; j < n && i != string::npos; ++j) {
      i = s.find(p, i+1);
   }

   if (j == n) {
     return i;
   }
   else {
     return -1;
   }
}

// Checks whether a string is operator or not.
bool isOperator(const string str) {
	return (str == "*" || str == "/" || str == "+" || str == "-");
}

// Checks whether a char is a valid char (alphanumerical character) or not.
bool isValidChar(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_');
}

// Checks whether a string is a number or not.
bool is_number(const string& s) {
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

// Checks whether a string contains any arithmetic operator or not.
bool hasArithmeticOperations(string &line) {
	return !(line.find("+") == string::npos && line.find("-") == string::npos && line.find("*") == string::npos && line.find("/") == string::npos);
}

// Checks whether a char is a letter or not.
bool letterCheck(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Checks the string is allocated before by looking up to the variables vector.
bool findVar(string str) {
	return find(variables.begin(), variables.end(), str) != variables.end();
}

// Checks whether the choose string is processed before.
bool findChoose(string str) {
	return find(chooseStore.begin(), chooseStore.end(), str) != chooseStore.end();
}

// Checks the string is a valid variable or not.
// A valid variable should start with a letter and followed by alphanumerical characters.
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

// Allocates the string with default 0 value.
bool writeToAllocateString(string str) {
	if(!findVar(str) && variableCheck(str)) {
		allocateString += "\n\t%" + str + " = alloca i32";
		storeDefaultString += "\n\tstore i32 0, i32* %" + str;
		variables.push_back(str);
		return true;
	}
	return false;
}

// Stores a value or temp variable in a string.
void writeToStoreString(string str1, string str2) {
	normalExpressions += "\n\tstore i32 " + str2 + ", i32* %" + str1;
}

// Loads a value to a temp variable. The value can be a number or a variable.
void loadToTemp(string var) {
	normalExpressions = normalExpressions + "\n\t%t" + to_string(tempNo++) + " = load i32* %" + var;
}

// Checks the nested choose if there exists. Changes the comma index with a correct index if there are nested chooses.
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
		// If passes enough commas when it is compared with the choose num, breaks the loop.
		if((4+3*(chooseNum-1) == commaNum && chooseNum != 0) || (chooseNum == 0 && commaNum == 1)) {
			break;
		}
	}
	if(chooseNum>0) {
		// Finding the nth substring of "," by using the number of choose.
		commaIndex = nthSubstr(4+3*(chooseNum-1), tempVar, ",");
		isChecked = true;
	}
	return isChecked;
}

// Checks whether the string has a valid choose syntax or not.
bool chooseCheck(string str) {
    if(str.length() < 12) {
        return false;
    } else if(str.substr(0, 6) == "choose") {
        string tempVar = str.substr(6);
        deleteEdgeSpaces(tempVar);
        int openParanthesisIndex = tempVar.find("(");
        // Finds the open paranthesis index and checks whether there is any character other than space between choose text and the paranthesis
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
        // Takes the first expression of the choose function parameters by using the comma index.
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
            // Takes the second expression of the choose function parameters by using the comma index.
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
                // Takes the third expression of the choose function parameters by using the comma index.
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
                    	// Takes the forth expression of the choose function parameters by using the close paranthesis index.
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

// Checks whether the string is a factor: can be a choose function, variable or number.
bool factorCheck(string str) {
	if(chooseCheck(str)) {
		return true;
	} else {		
		int openIndex = str.find("(");
		// Takes the code inside of the paranthesis.
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

// Checks whether the string is a valid expression or not. An expression could be a facor or it may have arithmetic operations.
bool expressionCheck(string str) {
	if(factorCheck(str)) {
		return true;
	} else if(hasArithmeticOperations(str)) {
		return true;
	} else {
		return false;
	}
}

// Returns the precedence of an operator. * and / has the highest precedence.
int precedence(char c) {
    int temp = 0;
    if(c == '+' || c == '-') {
        temp = 1;
    } else if(c == '*' || c == '/') {
        temp = 2;
    }
    return temp;
}

// Checks whether the char is a valid operator or not.
bool isOperator(char input) {
    if(input == '+' || input == '-' || input == '*' || input == '/') {
        return true;
    }
    return false;
}

// Writes the appropriate llvm code for an airthmetic operation according to the operation type.
void writeOperation(string num1, string num2, string type) {
	string firstTemp = num1;
	string secondTemp = num2;
	// Takes the first and second num and checks they are a number or a variable.
	if(!is_number(num1)) {
		// If it is a variable, stores their value in a temp variable to use.
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

// Converts an expression from infix notation to the appropriate postfix notation.
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
    	// Checks a character is a valid char, paranthsesis, operation or a space. Performs corresponding operation. 
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
			// Uses stack to store the paranthesis and the operators. 
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
	// Controls the converted string and count the operator and operand numbers. If operand number is not operator number + 1, prints an error.
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

// Outputs the corresponding llvm codes for the arithmetic operations by using the converted postfix.
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
			// If it is an operator, takes the first and second operands and outputs the corresponding llvm code.
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
			// If it is a variable or number, pushes it to the numbers stack and if it is a variable, allocates it.
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

//takes a string, str, which is a condition statement of an if or a while statement and another string, type, which determines the special word of this condition statement
//creates a condition statement for our language
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

//takes a string that might be an if or a while statement and its type, then checks if given string is a valid if or while statement
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

//takes an if or a while statement, find its condition string and create an if or a while statement in our language
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

//takes a string and checks if it is a valid print statement 
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

//creates if statements according to given rule of choose function, if expression contains another choose statement, then executes that choose first
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

//finds four expressions takes place in the outmost choose statement separated by comma and sends them to chooseIfHelper method to create appropriate if statements
void chooseHandler(string line) {
	int commaIndex = line.find(",");
	int firstParanthesis = line.find("(");
	int lastParanthesis = line.find_last_of(")");
	line = line.substr(firstParanthesis+1, lastParanthesis - firstParanthesis);
    checkNestedChoose(line, commaIndex);
    string first = line.substr(0, commaIndex);

    line = line.substr(commaIndex+1);
    //finds next comma which is an element of the outmost choose statement
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

//takes a string, line, which takes place inside a print statement in the given code, determines which types of statemens it contains and does necessary operations
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

//finds outmost opening and closing parantheses indexes, takes the string between them and does necessary operations repeatedly until there are no choose statements left
void chooseArithmetic(string& secondPart) {
	int chooseIndex = secondPart.find("choose");
	int opened = 0;
	int closedIndex = 0;
	bool isEntered = false;
	//finds beginning and ending of each choose statement and sends them to chooseHandler method to compute the results of these statements
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

// Takes two strings, left hand side and right hand side of an assignment statement, checks if left hand side is valid and determines which type of statements 
// right hand side statement contains
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

// Takes a string, line, which is an assignment line, separates left hand side and right hand side, checks if there is an error in these parts
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

	//takes every line and stores them in a vector separately
	while(getline(infile, token)) {
		tokens.push_back(token);	
	}
	lastChoose = "";

	for(int k=0; k<tokens.size() && !hasError; k++) {
		lineNo++;
		string line = tokens[k];
		deleteEdgeSpaces(line);
		int commentIndex = line.find("#");
		//if there is a comment, ignores it
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
			//checks if it is an if statement
		} else if(ifWhileCheck(line, "if")){
			//checks if there is a nested ifs or if and while
			if(inWhileBody || inIfBody) {
				printError();
				break;
			}
			inIfBody = true;
			curlyBracket++;
			ifWhileHandler(line, "if");
			//checks if it is an if statement
		} else if(ifWhileCheck(line, "while")){
			//checks if there is a nested ifs or if and while
			if(inWhileBody || inIfBody) {
				printError();
				break;
			}
			inWhileBody = true;
			curlyBracket++;
			ifWhileHandler(line, "while");
		} else if(printCheck(line)){
			printHandler(line);
			//checks if it is ending of a while or an if statement
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
