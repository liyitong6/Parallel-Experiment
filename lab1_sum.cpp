#include <iostream>
#include <sys/time.h>
#include <cmath>
using namespace std;
const int n = 10000000; 
float a[n];

void init() {
    for (int i = 0; i < n; i++) 
    a[i] = static_cast<float>(i);
} //初始化

int main() {
    init();
    struct timeval start, end;
    double time_used;
    float sum_1 = 0, sum_2 = 0, sum_4 = 0;

    gettimeofday(&start, NULL); 
    for (int j = 0; j < 100; j++) { 
        sum_1 = 0;
        for (int i = 0; i < n; i++) 
        sum_1 += a[i];
    }
    gettimeofday(&end, NULL);
    time_used = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    cout << "平凡耗时: " << time_used << " ms" << endl;

    gettimeofday(&start, NULL);
    for (int j = 0; j < 100; j++) {
        float s1 = 0, s2 = 0;
        for (int i = 0; i < n; i += 2) {
            s1 += a[i];
            s2 += a[i+1];
        }
        sum_2 = s1 + s2;
    }
    gettimeofday(&end, NULL);
    time_used = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    cout << "优化 (双路) 耗时: " << time_used << " ms" << endl;

    gettimeofday(&start, NULL);
    for (int j = 0; j < 100; j++) {
        float s1 = 0, s2 = 0, s3 = 0, s4 = 0;
        for (int i = 0; i < n; i += 4) {
            s1 += a[i];
            s2 += a[i+1];
            s3 += a[i+2];
            s4 += a[i+3];
        }
        sum_4 = (s1 + s2) + (s3 + s4);
    }
    gettimeofday(&end, NULL);
    time_used = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    cout << "优化（四路）耗时: " << time_used << " ms" << endl;

   cout << "单路结果: " << sum_1 << endl;
    cout << "双路结果: " << sum_2 << endl;
    cout << "四路结果: " << sum_4 << endl;

    if (abs(sum_1 - sum_4) / sum_1 < 1e-5 && abs(sum_1 - sum_2) / sum_1 < 1e-5) {
        cout << "结果一致" << endl;
    } else {
        cout << "存在差异" << endl;
    }

    return 0;
}
