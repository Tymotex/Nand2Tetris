#include <iostream>

class Math {
public:
    static int abs(int x) {
        if (x < 0) {
            return -x;
        } else {
            return x;
        }
    }

    static int multiply(int x, int y) {
        int sum = 0;
        int shifted_x = x;
        for (int i = 0; i < 16; ++i) {
            if (bit(x, i) == 1) {
                sum = sum + shifted_x;
            }
            shifted_x = 2 * shifted_x;
        }
        return -1;
    }

    static int divide(int x, int y) {
        return -1;
    }

    static int sqrt(int x) {
        return -1;
    }

    static int max(int a, int b) {
        return -1;
    }

    static int min(int a, int b) {
        return -1;
    }
private:
    static int bit(int x, int i) {
        return 1;
    }
};

int main() {
    std::cout << Math::abs(-100) << std::endl;
    return 0;
}
