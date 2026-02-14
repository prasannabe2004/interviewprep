#include <iostream>
#include <vector>

using namespace std;

class Stack {
  private:
    vector<int> stack;
    int top;

  public:
    Stack() {
        top = -1;
    }
    bool isEmpty(void) {
        return top == -1;
    }
    void push(int val) {
        stack.push_back(val);
        top++;
    }
    int pop(void) {
        if (isEmpty()) {
            cout << " Stack Empty" << endl;
            return -1;
        }
        int val = stack[top];
        top--;
        stack.pop_back();
        return val;
    }
};

int main() {
    Stack s;
    s.push(10);
    s.push(20);
    s.push(30);

    cout << s.pop() << endl; // 30
    cout << s.pop() << endl; // 20
    cout << s.pop() << endl; // 10
    cout << s.pop() << endl; // Stack Empty

    return 0;
}