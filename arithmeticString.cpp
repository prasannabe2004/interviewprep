#include <iostream>
#include <string>
/**
 * Evaluates an arithmetic expression with only '+' and '*' operators.
 * Handles multi-digit numbers and ignores spaces. Uses O(1) extra space.
 * @param expression The input string expression.
 * @return The result of the evaluation.
 */
long long evaluateExpression(const std::string& expression) {
    long long result = 0;       // Stores the final result of all additions.
    long long current_term = 0; // Stores the value of the current term being multiplied.
    char last_op = '+';         // Stores the last operator encountered.
    long long current_num = 0;  // Stores the multi-digit number currently being built.

    for (int i = 0; i < expression.length(); ++i) {
        char c = expression[i];

        if (isdigit(c)) {
            current_num = current_num * 10 + (c - '0');
        } else if (c == '+' || c == '*') {
            if (last_op == '+') {
                result += current_term;
                current_term = current_num;
            } else if (last_op == '*') {
                current_term *= current_num;
            }
            last_op = c;
            current_num = 0;
        }
        // Ignores any spaces in the expression.
    }

    // After the loop, handle the final number/term.
    if (last_op == '+') {
        result += current_term;
        current_term = current_num;
    } else if (last_op == '*') {
        current_term *= current_num;
    }
    result += current_term;
    return result;
}

// Example usage
int main() {
    std::string expr1 = "12 + 3 * 5";
    std::string expr2 = "10*20+30";
    std::string expr3 = "123*2+1";
    std::string expr4 = "9*80*7";

    std::cout << "Evaluation of \"" << expr1 << "\": " << evaluateExpression(expr1)<< std::endl; // Expected: 12 + 15 = 27
    std::cout << "Evaluation of \"" << expr2 << "\": " << evaluateExpression(expr2)<< std::endl; // Expected: 200 + 30 = 230
    std::cout << "Evaluation of \"" << expr3 << "\": " << evaluateExpression(expr3)<< std::endl; // Expected: 246 + 1 = 247
    std::cout << "Evaluation of \"" << expr4 << "\": " << evaluateExpression(expr4)<< std::endl; // Expected: 5040

    return 0;
}
