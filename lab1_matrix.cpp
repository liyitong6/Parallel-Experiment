Lab1/
#include <iostream>
#include <sys/time.h>
#include <cmath>
using namespace std;
const int n = 3000; 
double a[n];
double b[n][n];
double sum_1[n];
double sum_2[n];

void init() {
    for (int i = 0; i < n; i++) {
        a[i] = i;
        for (int j = 0; j < n; j++) {
            b[i][j] = i + j; 
        }
    }
}

int main() {
    init();
    struct timeval start, end;
    double time_used;
    gettimeofday(&start, NULL);
    for(int i = 0; i < n; i++) {
        sum_1[i] = 0.0;
        for(int j = 0; j < n; j++) {
            sum_1[i] += b[j][i] * a[j]; 
        }
    }
    
    gettimeofday(&end, NULL);
    time_used = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    cout << "平凡算法耗时: " << time_used << " ms" << endl;

    gettimeofday(&start, NULL);

    for(int i = 0; i < n; i++) 
    sum_2[i] = 0.0; 
    
    for(int j = 0; j < n; j++) { 
        for(int i = 0; i < n; i++) { 
            sum_2[i] += b[j][i] * a[j]; 
        }
    }
    
    gettimeofday(&end, NULL);
    time_used = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    cout << "优化算法耗时: " << time_used << " ms" << endl;

bool match = true;
for (int i = 0; i < n; i++) {
    if (abs(sum_1[i] - sum_2[i]) > 1e-6) {
        match = false;
        break;
    }
}

if (match) cout << "结果一致" << endl;
else cout << "结果不一致" << endl;

    return 0;
}
