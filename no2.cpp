#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

class Matrix {
private:
    int N;
    vector<vector<int>> mat;

public:
    Matrix(int size) {
        N = size;
        mat.resize(N, vector<int>(N, 0));
    }

    void fillSpiral(bool clockwise) {
        int top = 0, bottom = N - 1;
        int left = 0, right = N - 1;
        int num = 1;

        while (num <= N * N) {
            if (clockwise) {
                // → вправо
                for (int i = left; i <= right; i++)
                    mat[top][i] = num++;
                top++;

                // ↓ вниз
                for (int i = top; i <= bottom; i++)
                    mat[i][right] = num++;
                right--;

                // ← влево
                if (top <= bottom) {
                    for (int i = right; i >= left; i--)
                        mat[bottom][i] = num++;
                    bottom--;
                }

                // ↑ вверх
                if (left <= right) {
                    for (int i = bottom; i >= top; i--)
                        mat[i][left] = num++;
                    left++;
                }
            }
            else {
                // ↓ вниз
                for (int i = top; i <= bottom; i++)
                    mat[i][left] = num++;
                left++;

                // → вправо
                for (int i = left; i <= right; i++)
                    mat[bottom][i] = num++;
                bottom--;

                // ↑ вверх
                if (left <= right) {
                    for (int i = bottom; i >= top; i--)
                        mat[i][right] = num++;
                    right--;
                }

                // ← влево
                if (top <= bottom) {
                    for (int i = right; i >= left; i--)
                        mat[top][i] = num++;
                    top++;
                }
            }
        }
    }

    void print() {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                cout << mat[i][j] << "\t";
            }
            cout << endl;
        }
    }
};

int main() {
    srand(time(0));

    int N = rand() % 8 + 3; // от 3 до 10
    bool clockwise = rand() % 2;

    cout << "Размер: " << N << endl;
    cout << "Направление: " << (clockwise ? "по часовой" : "против часовой") << endl << endl;

    Matrix m(N);
    m.fillSpiral(clockwise);
    m.print();

    return 0;
}