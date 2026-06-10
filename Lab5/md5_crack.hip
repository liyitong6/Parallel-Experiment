#include <hip/hip_runtime.h>
#include <iostream>
#include <iomanip>
#include <chrono>

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

#define FF(a, b, c, d, x, s, ac) { \
    (a) += F((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a), (s)) + (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
    (a) += G((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a), (s)) + (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
    (a) += H((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a), (s)) + (b); \
}
#define II(a, b, c, d, x, s, ac) { \
    (a) += I((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTATE_LEFT((a), (s)) + (b); \
}

__host__ __device__ void md5_hash_6bytes(const unsigned char* msg, uint32_t* digest) {
    uint32_t x[16] = {0};
    
    x[0] = (uint32_t)msg[0] | ((uint32_t)msg[1] << 8) | ((uint32_t)msg[2] << 16) | ((uint32_t)msg[3] << 24);
    x[1] = (uint32_t)msg[4] | ((uint32_t)msg[5] << 8) | (0x80 << 16); 
    x[14] = 48; 

    uint32_t a = 0x67452301, b = 0xefcdab89, c = 0x98badcfe, d = 0x10325476;

    FF(a, b, c, d, x[0],  7, 0xd76aa478); FF(d, a, b, c, x[1], 12, 0xe8c7b756);
    FF(c, d, a, b, x[2], 17, 0x242070db); FF(b, c, d, a, x[3], 22, 0xc1bdceee);
    FF(a, b, c, d, x[4],  7, 0xf57c0faf); FF(d, a, b, c, x[5], 12, 0x4787c62a);
    FF(c, d, a, b, x[6], 17, 0xa8304613); FF(b, c, d, a, x[7], 22, 0xfd469501);
    FF(a, b, c, d, x[8],  7, 0x698098d8); FF(d, a, b, c, x[9], 12, 0x8b44f7af);
    FF(c, d, a, b, x[10], 17, 0xffff5bb1); FF(b, c, d, a, x[11], 22, 0x895cd7be);
    FF(a, b, c, d, x[12],  7, 0x6b901122); FF(d, a, b, c, x[13], 12, 0xfd987193);
    FF(c, d, a, b, x[14], 17, 0xa679438e); FF(b, c, d, a, x[15], 22, 0x49b40821);

    GG(a, b, c, d, x[1],  5, 0xf61e2562); GG(d, a, b, c, x[6],  9, 0xc040b340);
    GG(c, d, a, b, x[11], 14, 0x265e5a51); GG(b, c, d, a, x[0], 20, 0xe9b6c7aa);
    GG(a, b, c, d, x[5],  5, 0xd62f105d); GG(d, a, b, c, x[10],  9,  0x2441453);
    GG(c, d, a, b, x[15], 14, 0xd8a1e681); GG(b, c, d, a, x[4], 20, 0xe7d3fbc8);
    GG(a, b, c, d, x[9],  5, 0x21e1cde6); GG(d, a, b, c, x[14],  9, 0xc33707d6);
    GG(c, d, a, b, x[3], 14, 0xf4d50d87); GG(b, c, d, a, x[8], 20, 0x455a14ed);
    GG(a, b, c, d, x[13],  5, 0xa9e3e905); GG(d, a, b, c, x[2],  9, 0xfcefa3f8);
    GG(c, d, a, b, x[7], 14, 0x676f02d9); GG(b, c, d, a, x[12], 20, 0x8d2a4c8a);

    HH(a, b, c, d, x[5],  4, 0xfffa3942); HH(d, a, b, c, x[8], 11, 0x8771f681);
    HH(c, d, a, b, x[11], 16, 0x6d9d6122); HH(b, c, d, a, x[14], 23, 0xfde5380c);
    HH(a, b, c, d, x[1],  4, 0xa4beea44); HH(d, a, b, c, x[4], 11, 0x4bdecfa9);
    HH(c, d, a, b, x[7], 16, 0xf6bb4b60); HH(b, c, d, a, x[10], 23, 0xbebfbc70);
    HH(a, b, c, d, x[13],  4, 0x289b7ec6); HH(d, a, b, c, x[0], 11, 0xeaa127fa);
    HH(c, d, a, b, x[3], 16, 0xd4ef3085); HH(b, c, d, a, x[6], 23,  0x4881d05);
    HH(a, b, c, d, x[9],  4, 0xd9d4d039); HH(d, a, b, c, x[12], 11, 0xe6db99e5);
    HH(c, d, a, b, x[15], 16, 0x1fa27cf8); HH(b, c, d, a, x[2], 23, 0xc4ac5665);

    II(a, b, c, d, x[0],  6, 0xf4292244); II(d, a, b, c, x[7], 10, 0x432aff97);
    II(c, d, a, b, x[14], 15, 0xab9423a7); II(b, c, d, a, x[5], 21, 0xfc93a039);
    II(a, b, c, d, x[12],  6, 0x655b59c3); II(d, a, b, c, x[3], 10, 0x8f0ccc92);
    II(c, d, a, b, x[10], 15, 0xffeff47d); II(b, c, d, a, x[1], 21, 0x85845dd1);
    II(a, b, c, d, x[8],  6, 0x6fa87e4f); II(d, a, b, c, x[15], 10, 0xfe2ce6e0);
    II(c, d, a, b, x[6], 15, 0xa3014314); II(b, c, d, a, x[13], 21, 0x4e0811a1);
    II(a, b, c, d, x[4],  6, 0xf7537e82); II(d, a, b, c, x[11], 10, 0xbd3af235);
    II(c, d, a, b, x[2], 15, 0x2ad7d2bb); II(b, c, d, a, x[9], 21, 0xeb86d391);

    digest[0] = a + 0x67452301; digest[1] = b + 0xefcdab89;
    digest[2] = c + 0x98badcfe; digest[3] = d + 0x10325476;
}

__global__ void crack_kernel(uint32_t target0, uint32_t target1, uint32_t target2, uint32_t target3, 
                             long long total_combinations, long long* d_found_idx) {
    long long start_idx = blockIdx.x * (long long)blockDim.x + threadIdx.x;
    long long stride = gridDim.x * (long long)blockDim.x;

    for (long long idx = start_idx; idx < total_combinations; idx += stride) {
        unsigned char passwd[7];
        passwd[6] = '\0';
        long long temp = idx;
        for (int i = 5; i >= 0; i--) {
            passwd[i] = 'a' + (temp % 26);
            temp /= 26;
        }

        uint32_t digest[4];
        md5_hash_6bytes(passwd, digest);

        if (digest[0] == target0 && digest[1] == target1 && 
            digest[2] == target2 && digest[3] == target3) {
            atomicExch((unsigned long long*)d_found_idx, (unsigned long long)idx);
        }
    }
}

int main() {
    uint32_t target[4];
    md5_hash_6bytes((const unsigned char*)"nankai", target);

    long long total_combinations = 26LL * 26 * 26 * 26 * 26 * 26; 
    std::cout << "=== GPU (HIP) Password Cracking Task ===" << std::endl;
    std::cout << "Secret Target: nankai" << std::endl;
    std::cout << "Search Space Size: " << total_combinations << " combinations." << std::endl << std::endl;

    long long h_found_idx = -1;
    long long *d_found_idx;
    hipMalloc((void**)&d_found_idx, sizeof(long long));
    hipMemcpy(d_found_idx, &h_found_idx, sizeof(long long), hipMemcpyHostToDevice);

    int block_size = 256;
    int grid_size = 8192; 

    std::cout << "Launching Kernel with Grid Size: " << grid_size << " blocks..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    crack_kernel<<<grid_size, block_size>>>(target[0], target[1], target[2], target[3], 
                                            total_combinations, d_found_idx);
    
    hipDeviceSynchronize();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

    hipMemcpy(&h_found_idx, d_found_idx, sizeof(long long), hipMemcpyDeviceToHost);

    std::cout << "Kernel Execution Completed." << std::endl;
    std::cout << "GPU Compute Time: " << elapsed.count() << " ms" << std::endl;

    if (h_found_idx != -1) {
        char cracked[7];
        cracked[6] = '\0';
        long long temp = h_found_idx;
        for (int i = 5; i >= 0; i--) {
            cracked[i] = 'a' + (temp % 26);
            temp /= 26;
        }
        std::cout << "CRACK SUCCESSFUL! Found Password: " << cracked << std::endl;
    } else {
        std::cout << "Password not found in search space." << std::endl;
    }

    hipFree(d_found_idx);
    return 0;
}
