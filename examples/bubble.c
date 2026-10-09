void swap(int* a, int* b) {
    int c = *a;
    *a = *b;
    *b = c;
}

int isSorted(int* int_p, int size) {
    int i = 0;

    while (i < size - 1) {
        int curr = int_p[i];
        int next = int_p[i + 1];

        if (curr > next)
            return 0;

        i = i + 1;
    }

    return 1;
}

int sort(int* int_p, int size) {
    int i = 0;

    while (isSorted(int_p, size) != 1) {
        i = 0;
        while (i < size - 1) {
            int curr = int_p[i];
            int next = int_p[i + 1];

            if (curr > next)
                swap(&int_p[i], &int_p[i + 1]);

            i = i + 1;
        }
    }

    return 0;
}

int main() {
    int arr[5];

    arr[0] = 3;
    arr[1] = 5;
    arr[2] = 2;
    arr[3] = 1;
    arr[4] = 0;

    sort(&arr[0], 5);

    return arr[2];
}
