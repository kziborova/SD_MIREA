#include "SHA-1Service.hpp"

const std::string SHA1Service::CHARSET =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "!@#$%^&*()-_=+[]{}|;:,.<>?";

std::string SHA1Service::uint8ToHex(const std::vector<uint8_t>& bytes) {
    std::ostringstream oss;
    for (auto b : bytes)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    return oss.str();
}

static const char* SHA1_KERNEL = R"(
#define ROTL(x,n) (((x)<<(n))|((x)>>(32-(n))))

void sha1(const uchar* msg, int len, uchar* digest) {
    uint h0=0x67452301, h1=0xEFCDAB89,
         h2=0x98BADCFE, h3=0x10325476, h4=0xC3D2E1F0;

    uchar block[64];
    for(int i=0;i<64;i++) block[i]=0;
    for(int i=0;i<len;i++) block[i]=msg[i];
    block[len]=0x80;

    ulong bitLen=(ulong)len*8;
    for(int i=0;i<8;i++)
        block[63-i]=(uchar)(bitLen>>(i*8));

    uint w[80];
    for(int j=0;j<16;j++)
        w[j]=((uint)block[j*4]<<24)|((uint)block[j*4+1]<<16)|
             ((uint)block[j*4+2]<<8)|(uint)block[j*4+3];
    for(int j=16;j<80;j++)
        w[j]=ROTL(w[j-3]^w[j-8]^w[j-14]^w[j-16],1);

    uint a=h0,b=h1,c=h2,d=h3,e=h4;
    for(int j=0;j<80;j++){
        uint f,k;
        if(j<20){f=(b&c)|((~b)&d);k=0x5A827999u;}
        else if(j<40){f=b^c^d;k=0x6ED9EBA1u;}
        else if(j<60){f=(b&c)|(b&d)|(c&d);k=0x8F1BBCDCu;}
        else{f=b^c^d;k=0xCA62C1D6u;}
        uint temp=ROTL(a,5)+f+e+k+w[j];
        e=d;d=c;c=ROTL(b,30);b=a;a=temp;
    }
    h0+=a;h1+=b;h2+=c;h3+=d;h4+=e;

    uint hs[5]={h0,h1,h2,h3,h4};
    for(int i=0;i<5;i++)
        for(int j=0;j<4;j++)
            digest[i*4+j]=(uchar)(hs[i]>>((3-j)*8));
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

    uchar digest[20];
    sha1(candidate, strLen, digest);

    int match = 1;
    for(int i=0;i<20;i++)
        if(digest[i]!=target[i]){match=0;break;}

    if(match && atomic_cmpxchg(foundFlag,0,1)==0){
        for(int i=0;i<strLen&&i<maxFoundLen;i++)
            foundStr[i]=candidate[i];
        foundStr[strLen]=0;
    }
}
)";

std::string SHA1Service::openclBruteforce(const std::string& targetHash) {
    const std::string TAG = "[SHA1 " + targetHash + "]";

    std::vector<uint8_t> targetBytes(20);
    for (int i = 0; i < 20; i++)
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

    const char* src = SHA1_KERNEL;
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
                            20, targetBytes.data(), &err);
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
            long long batchEnd  = std::min(startIdx + BATCH_SIZE, total);
            size_t globalSize   = (size_t)(batchEnd - startIdx);

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

bool SHA1Service::checkType(const std::string& hash) {
    if (hash.size() != 40) return false;
    for (char c : hash)
        if (!isxdigit(c)) return false;
    return true;
}

std::string SHA1Service::hashing(const std::string& hash) {
    const std::string TAG = "[SHA1 " + hash + "]";

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