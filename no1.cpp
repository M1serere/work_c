#include <iostream>
#include <vector>

using namespace std;

vector<vector<int>> findElements(
    const vector<vector<vector<int>>>& mtrx,
    int z
) {
    vector<vector<int>> result;

    for (int i = 0; i < mtrx.size(); i++) {
        for (int j = 0; j < mtrx[i].size(); j++) {
            for (int k = 0; k < mtrx[i][j].size(); k++) {
                if (mtrx[i][j][k] == z) {
                    result.push_back({ i, j, k });
                }
            }
        }
    }

    return result;
}


int main()
{
    vector<vector<vector<int>>> mtrx = {
        {
            {1, 2},
            {3, 4}
        },
        {
            {5, 3},
            {7, 8}
        }
    };

    int z = 3;

    vector<vector<int>> result = findElements(mtrx, z);

    for (auto coord : result) {
        cout << "("
            << coord[0] << ", "
            << coord[1] << ", "
            << coord[2] << ")\n";
    }

    return 0;
}