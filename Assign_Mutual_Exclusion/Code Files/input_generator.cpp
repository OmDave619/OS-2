//generates input.txt file with matrix a of size n by n of random numbers

#include<bits/stdc++.h>
using namespace std;

#define n 1024// size of matrix
#define k 4 // number of threads
#define rowInc 23 //Number of rows calculated by a thread in one go

int main() {
     
    //for sparse matrix
    vector<int> input_arr(10,0);
    for(int i = 0; i < 25; i++) {
        input_arr[rand()%10] = rand()%10;
    }

    FILE* out = fopen("../Input Files/input.txt", "w");
    if (out == NULL) {
        cout << "Output file not found" << endl;
        return 1;
    }

    fprintf(out, "%d %d %d\n", n, k,rowInc);

    //generate matrix of size n by n with random numbers
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(out, "%d ", input_arr[rand()% 10]);
        }
        fprintf(out, "\n");
    }
}