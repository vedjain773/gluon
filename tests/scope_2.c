//EXPECTED: 1

int main() {
    int x;
    x = 1;

    if (x == 1) {
        int x = 4;
    }

    return x;
}
