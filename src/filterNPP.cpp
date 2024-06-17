#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#pragma warning(disable : 4819)
#endif

#include <Exceptions.h>
#include <ImageIO.h>
#include <ImagesCPU.h>
#include <ImagesNPP.h>

#include <string.h>
#include <fstream>
#include <iostream>

#include <cuda_runtime.h>
#include <npp.h>

#include <helper_cuda.h>
#include <helper_string.h>

void printNPPInfo()
{
  const NppLibraryVersion *libVer = nppGetLibVersion();
  std::cout << "NPP Library Version: " << libVer->major << '.' << libVer->minor << '.' << libVer->build << '\n';

  int driverVersion, runtimeVersion;
  cudaDriverGetVersion(&driverVersion);
  cudaRuntimeGetVersion(&runtimeVersion);

  std::cout << "CUDA Driver Version: " << driverVersion / 1000 << '.' << (driverVersion % 100) / 10 << '\n';
  std::cout << "CUDA Runtime Version: " << runtimeVersion / 1000 << '.' << (runtimeVersion % 100) / 10 << '\n';
}

struct CommandLineArgs
{
  std::string inputFilePath;
  std::string outputFilePath;
};

CommandLineArgs processCommandLine(int argc, char **argv)
{
  CommandLineArgs args;
  char *inputPath = nullptr;
  if (checkCmdLineFlag(argc, (const char **)argv, "input"))
  {
    getCmdLineArgumentString(argc, (const char **)argv, "input", &inputPath);
    args.inputFilePath = inputPath;
  }
  else
  {
    args.inputFilePath = sdkFindFilePath("Lena.pgm", argv[0]);
    if (args.inputFilePath.empty())
    {
      throw std::runtime_error("Input file Lena.pgm not found.");
    }
  }

  if (checkCmdLineFlag(argc, (const char **)argv, "output"))
  {
    char *outputPath = nullptr;
    getCmdLineArgumentString(argc, (const char **)argv, "output", &outputPath);
    args.outputFilePath = outputPath;
  }
  else
  {
    args.outputFilePath = args.inputFilePath.substr(0, args.inputFilePath.rfind('.')) + "_boxFilter.pgm";
  }
  return args;
}

int main(int argc, char **argv)
{
  std::cout << argv[0] << " Starting...\n\n";

  try
  {
    findCudaDevice(argc, (const char **)argv);
    printNPPInfo();

    CommandLineArgs args = processCommandLine(argc, argv);

    npp::ImageCPU_8u_C1 oHostSrc;
    npp::loadImage(args.inputFilePath, oHostSrc);
    npp::ImageNPP_8u_C1 oDeviceSrc(oHostSrc);

    NppiSize maskSize = {5, 5};
    NppiSize srcSize = {static_cast<int>(oDeviceSrc.width()), static_cast<int>(oDeviceSrc.height())};
    NppiPoint srcOffset = {0, 0};

    npp::ImageNPP_8u_C1 oDeviceDst(srcSize.width, srcSize.height);
    NppiPoint anchor = {maskSize.width / 2, maskSize.height / 2};

    NPP_CHECK_NPP(nppiFilterBoxBorder_8u_C1R(
        oDeviceSrc.data(), oDeviceSrc.pitch(), srcSize, srcOffset,
        oDeviceDst.data(), oDeviceDst.pitch(), srcSize, maskSize, anchor,
        NPP_BORDER_REPLICATE));

    npp::ImageCPU_8u_C1 oHostDst(oDeviceDst.size());
    oDeviceDst.copyTo(oHostDst.data(), oHostDst.pitch());

    saveImage(args.outputFilePath, oHostDst);
    std::cout << "Saved image: " << args.outputFilePath << std::endl;

    nppiFree(oDeviceSrc.data());
    nppiFree(oDeviceDst.data());
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}