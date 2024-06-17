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
#include <iostream>
#include <map>
#include <string>

enum Filter
{
  GAUSS,
  BOX,
  UNKNOWN
};

std::map<std::string, Filter> filterMap = {
    {"gauss_filter", GAUSS},
    {"box_filter", BOX}};

Filter getFilter(const std::string &filter)
{
  auto it = filterMap.find(filter);
  if (it != filterMap.end())
  {
    return it->second;
  }
  return UNKNOWN;
}

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
  Filter filter;
};

CommandLineArgs processCommandLine(int argc, char **argv)
{
  CommandLineArgs args;
  char *inputPath = nullptr;
  if (checkCmdLineFlag(argc, (const char **)argv, "input"))
  {
    getCmdLineArgumentString(argc, (const char **)argv, "input", &inputPath);
    args.inputFilePath = inputPath;
    std::cout << "input file path is taken from args " << args.inputFilePath << std::endl;
  }

  if (checkCmdLineFlag(argc, (const char **)argv, "output"))
  {
    char *outputPath = nullptr;
    getCmdLineArgumentString(argc, (const char **)argv, "output", &outputPath);
    args.outputFilePath = outputPath;
  }

  if (checkCmdLineFlag(argc, (const char **)argv, "filter"))
  {
    char *filter = nullptr;
    getCmdLineArgumentString(argc, (const char **)argv, "filter", &filter);
    args.filter = getFilter(filter);
  }

  return args;
}

void applyGaussFilter(const std::string &filePath, const std::string &outputFile)
{

  npp::ImageCPU_8u_C1 hostSrc;
  npp::loadImage(filePath, hostSrc);
  npp::ImageNPP_8u_C1 deviceSrc(hostSrc);
  const NppiSize srcSize = {(int)deviceSrc.width(), (int)deviceSrc.height()};
  const NppiPoint srcOffset = {0, 0};

  const NppiSize filterROI = {(int)deviceSrc.width(), (int)deviceSrc.height()};
  npp::ImageNPP_8u_C1 deviceDst(filterROI.width, filterROI.height);

  NPP_CHECK_NPP(nppiFilterGaussBorder_8u_C1R(deviceSrc.data(), deviceSrc.pitch(), srcSize, srcOffset,
                                             deviceDst.data(), deviceDst.pitch(), filterROI,
                                             NppiMaskSize::NPP_MASK_SIZE_3_X_3, NPP_BORDER_REPLICATE));

  npp::ImageCPU_8u_C1 hostDst(deviceDst.size());
  deviceDst.copyTo(hostDst.data(), hostDst.pitch());
  saveImage(outputFile, hostDst);

  nppiFree(deviceSrc.data());
  nppiFree(deviceDst.data());
  nppiFree(hostSrc.data());
  nppiFree(hostDst.data());
}

void applyBoxFilter(const std::string &inputFilePath, const std::string &outputFilePath)
{
  npp::ImageCPU_8u_C1 oHostSrc;
  npp::loadImage(inputFilePath, oHostSrc);
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

  saveImage(outputFilePath, oHostDst);
  std::cout << "Saved image: " << outputFilePath << std::endl;

  nppiFree(oDeviceSrc.data());
  nppiFree(oDeviceDst.data());
  nppiFree(oHostDst.data());
  nppiFree(oHostSrc.data());
}

int main(int argc, char **argv)
{
  freopen("data/output.txt", "w", stdout);
  std::cout << argv[0] << " Starting...\n\n";

  try
  {
    findCudaDevice(argc, (const char **)argv);
    printNPPInfo();

    CommandLineArgs args = processCommandLine(argc, argv);
    std::cout << "args were parsed successfully starting main logic" << std::endl;
    switch (args.filter)
    {
    case GAUSS:
      std::cout << "applying Gauss filter to the changed image" << std::endl;
      applyGaussFilter(args.inputFilePath, args.outputFilePath);
      break;
    case BOX:
      std::cout << "applying Box filter to the original image" << std::endl;
      applyBoxFilter(args.inputFilePath, args.outputFilePath);
      break;
    default:
      std::cout << "Unknown command!" << std::endl;
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "Filter was applied successfully and image was saved!" << std::endl;
  return EXIT_SUCCESS;
}