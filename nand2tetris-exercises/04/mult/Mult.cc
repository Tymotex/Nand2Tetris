#include <iostream>

using namespace std;

int multiply(const int a, const int b) {
    int i = 0;
    int product = 0;
    while (i < a) {
        product += b;
        ++i;
    }
    return product; 
}

int main() {
  cout << "Enter two numbers (eg. 3 5): ";
  int a, b;
  cin >> a >> b;

  int result = multiply(a, b);
  cout << result << endl;
  return 0;
}
