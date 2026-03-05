#include "MD5Service.hpp"

const std::string MD5Service::CHARSET =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "!@#$%^&*()-_=+[]{}|;:,.<>?";

std::string MD5Service::uint8ToHex(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    for (auto b : bytes)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    return oss.str();
}

static const char* MD5_KERNEL = R"(
__constant uint T[64] = {
    0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,
    0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
    0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,
    0x6b901122,0xfd987193,0xa679438e,0x49b40821,
    0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,
    0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
    0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,
    0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
    0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,
    0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
    0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,
    0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
    0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,
    0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
    0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,
    0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};

__constant uint S[64] = {
    7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
    5, 9,14,20, 5, 9,14,20, 5, 9,14,20, 5, 9,14,20,
    4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
    6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21
};

#define ROTL(x,n) (((x)<<(n))|((x)>>(32-(n))))

void md5(const uchar* msg, int len, uchar* digest) {
    uint a0=0x67452301, b0=0xefcdab89,
         c0=0x98badcfe, d0=0x10325476;

    uchar block[64];
    for(int i=0;i<64;i++) block[i]=0;
    for(int i=0;i<len;i++) block[i]=msg[i];
    block[len]=0x80;
    ulong bitLen=(ulong)len*8;

    for(int i=0;i<8;i++)
        block[56+i]=(uchar)(bitLen>>(i*8));

    uint M[16];
    for(int i=0;i<16;i++)
        M[i]=((uint)block[i*4])|((uint)block[i*4+1]<<8)|
             ((uint)block[i*4+2]<<16)|((uint)block[i*4+3]<<24);

    uint A=a0, B=b0, C=c0, D=d0;
    for(int i=0;i<64;i++){
        uint F, g;
        if(i<16){
            F=(B&C)|((~B)&D); g=i;
        } else if(i<32){
            F=(D&B)|((~D)&C); g=(5*i+1)%16;
        } else if(i<48){
            F=B^C^D; g=(3*i+5)%16;
        } else {
            F=C^(B|(~D)); g=(7*i)%16;
        }
        F=F+A+T[i]+M[g];
        A=D; D=C; C=B;
        B=B+ROTL(F,S[i]);
    }
    a0+=A; b0+=B; c0+=C; d0+=D;

    digest[0]=(uchar)a0; digest[1]=(uchar)(a0>>8);
    digest[2]=(uchar)(a0>>16); digest[3]=(uchar)(a0>>24);
    digest[4]=(uchar)b0; digest[5]=(uchar)(b0>>8);
    digest[6]=(uchar)(b0>>16); digest[7]=(uchar)(b0>>24);
    digest[8]=(uchar)c0; digest[9]=(uchar)(c0>>8);
    digest[10]=(uchar)(c0>>16); digest[11]=(uchar)(c0>>24);
    digest[12]=(uchar)d0; digest[13]=(uchar)(d0>>8);
    digest[14]=(uchar)(d0>>16); digest[15]=(uchar)(d0>>24);
}

__kernel void bruteforce(
    __global const uchar* charset, int csLen,
    int strLen, long startIdx,
    __global const uchar* target,
    __global int* foundFlag,
    __global uchar* foundStr, int maxFoundLen)
{
    long idx = get_global_id(0) + startIdx;
    uchar candidate[8];
    long tmp = idx;
    for(int p = strLen-1; p >= 0; p--){
        candidate[p] = charset[tmp % csLen];
        tmp /= csLen;
    }

    uchar digest[16];
    md5(candidate, strLen, digest);

    int match = 1;
    for(int i=0;i<16;i++)
        if(digest[i]!=target[i]){match=0;break;}

    if(match && atomic_cmpxchg(foundFlag,0,1)==0){
        for(int i=0;i<strLen&&i<maxFoundLen;i++)
            foundStr[i]=candidate[i];
        foundStr[strLen]=0;
    }
}
)";

std::string MD5Service::openclBruteforce(const std::string& targetHash) {
    const std::string TAG = "[MD5 " + targetHash + "]";

    std::vector<uint8_t> targetBytes(16);
    for (int i = 0; i < 16; i++)
        targetBytes[i] = std::stoul(targetHash.substr(i*2, 2), nullptr, 16);

    cl_int err;
    cl_uint numPlatforms;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (numPlatforms == 0) {
        std::cout << TAG << " No OpenCL platforms\n";
        return "";
    }

    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);

    cl_device_id device = nullptr;
    for (auto& plat : platforms) {
        cl_uint numDevices;
        clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
        if (numDevices > 0) {
            std::vector<cl_device_id> devs(numDevices);
            clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, numDevices, devs.data(), nullptr);
            device = devs[0];
            break;
        }
    }
    if (!device) {
        std::cout << TAG << " No GPU found\n";
        return "";
    }

    char devName[128];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(devName), devName, nullptr);
    std::cout << TAG << " GPU: " << devName << "\n";

    cl_context ctx = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    cl_queue_properties props[] = {0};
    cl_command_queue queue = clCreateCommandQueueWithProperties(ctx, device, props, &err);

    const char* src = MD5_KERNEL;
    cl_program program = clCreateProgramWithSource(ctx, 1, &src, nullptr, &err);
    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        char log[4096];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, nullptr);
        std::cout << TAG << " OpenCL build error:\n" << log << "\n";
        return "";
    }

    cl_kernel kernel = clCreateKernel(program, "bruteforce", &err);

    cl_mem charsetBuf   = clCreateBuffer(ctx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                            CHARSET.size(), (void*)CHARSET.data(), &err);
    cl_mem targetBuf    = clCreateBuffer(ctx, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                            16, targetBytes.data(), &err);
    cl_mem foundFlagBuf = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(int), nullptr, &err);
    cl_mem foundStrBuf  = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, 16, nullptr, &err);

    int zero = 0;
    clEnqueueFillBuffer(queue, foundFlagBuf, &zero, sizeof(int), 0, sizeof(int), 0, nullptr, nullptr);

    const int csLen = CHARSET.size();
    const int maxFoundLen = 15;
    const long long BATCH_SIZE = 50'000'000LL;
    std::string result;

    for (int length = 1; length <= MAX_LENGTH && result.empty(); length++) {
        long long total = (long long)std::pow(csLen, length);
        int strLen = length;
        std::cout << TAG << " Trying length: " << length
                  << " (" << total << " combinations)\n";

        for (long long startIdx = 0; startIdx < total && result.empty(); startIdx += BATCH_SIZE) {
            long long batchEnd = std::min(startIdx + BATCH_SIZE, total);
            size_t globalSize  = (size_t)(batchEnd - startIdx);

            clSetKernelArg(kernel, 0, sizeof(cl_mem), &charsetBuf);
            clSetKernelArg(kernel, 1, sizeof(int),    &csLen);
            clSetKernelArg(kernel, 2, sizeof(int),    &strLen);
            clSetKernelArg(kernel, 3, sizeof(long),   &startIdx);
            clSetKernelArg(kernel, 4, sizeof(cl_mem), &targetBuf);
            clSetKernelArg(kernel, 5, sizeof(cl_mem), &foundFlagBuf);
            clSetKernelArg(kernel, 6, sizeof(cl_mem), &foundStrBuf);
            clSetKernelArg(kernel, 7, sizeof(int),    &maxFoundLen);

            clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
            clFinish(queue);

            int flag = 0;
            clEnqueueReadBuffer(queue, foundFlagBuf, CL_TRUE, 0, sizeof(int), &flag, 0, nullptr, nullptr);
            if (flag) {
                char buf[16] = {};
                clEnqueueReadBuffer(queue, foundStrBuf, CL_TRUE, 0, 15, buf, 0, nullptr, nullptr);
                result = std::string(buf);
                std::cout << TAG << " GPU FOUND: " << result << "\n";
                break;
            }

            if (startIdx % 500'000'000LL == 0 && startIdx > 0) {
                double percent = (double)startIdx / total * 100.0;
                std::cout << TAG << " Progress length=" << length
                          << " : " << std::fixed << std::setprecision(1)
                          << percent << "%\n";
            }
        }

        if (result.empty())
            std::cout << TAG << " Length " << length << " exhausted\n";
    }

    clReleaseMemObject(charsetBuf);
    clReleaseMemObject(targetBuf);
    clReleaseMemObject(foundFlagBuf);
    clReleaseMemObject(foundStrBuf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(ctx);

    return result;
}

bool MD5Service::checkType(const std::string& hash) {
    if (hash.size() != 32) return false;
    for (char c : hash)
        if (!isxdigit(c)) return false;
    return true;
}

std::string MD5Service::hashing(const std::string& hash) {
    const std::string TAG = "[MD5 " + hash + "]";

    std::cout << TAG << " Starting brute-force\n";
    std::cout << TAG << " Charset size: " << CHARSET.size()
              << ", max length: " << MAX_LENGTH << "\n";

    auto start = std::chrono::high_resolution_clock::now();

    std::string result = openclBruteforce(hash);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    timesec  = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

    std::cout << TAG << " Time elapsed: " << ms << "ms (" << timesec << "s)\n";
    std::cout << TAG << " Result: " << (result.empty() ? "NOT FOUND" : result) << "\n";

    return result;
}