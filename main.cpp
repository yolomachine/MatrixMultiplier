#define BENCHPRESS_CONFIG_MAIN
#include <iostream>
#include <fstream>
#include "samples.h"
#include "benchpress.hpp"
#ifdef _WIN32
#include <Windows.h>
#elif  __APPLE__
	#ifdef __MACH__
	#include "TargetConditionals.h"
	#include <sys/param.h>
	#include <sys/sysctl.h>
	#endif
#elif __unix__
#include <unistd.h>
#endif

#define PAUSE \
getchar(); \
getchar()

double CpuLoad;

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;

void init() {
	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;

	GetSystemInfo(&sysInfo);
	numProcessors = sysInfo.dwNumberOfProcessors;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&lastCPU, &ftime, sizeof(FILETIME));

	self = GetCurrentProcess();
	GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
	memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
	memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
}

double getCpuLoad() {
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));

	GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));
	percent = (sys.QuadPart - lastSysCPU.QuadPart) +
		(user.QuadPart - lastUserCPU.QuadPart);
	percent /= (now.QuadPart - lastCPU.QuadPart);
	percent /= numProcessors;
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;

	return percent * 100;
}

size_t getMaxThreadsCount() {
#ifdef _WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#elif __APPLE__
	#ifdef __MACH__
		int nm[2];
		size_t len = 4;
		uint32_t count;

		nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
		sysctl(nm, 2, &count, &len, NULL, 0);

		if (count < 1) {
			nm[1] = HW_NCPU;
			sysctl(nm, 2, &count, &len, NULL, 0);
			if (count < 1) { count = 1; }
		}
		return count;
	#endif
#else
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

int main(int argc, char** argv) {

/*
	size_t max_threads = getMaxThreadsCount();
	std::cout << "Threads available: " << max_threads << std::endl;
	do {
		std::cin >> threads_number;
		if (threads_number < 1 || threads_number > max_threads) std::cout << "Unacceptable number" << std::endl;
		else std::cout << "Initiating process" << std::endl;
	} while (!threads_number || threads_number > max_threads);
	
*/
	std::ofstream ofs("benchmark_out_info.txt", std::ofstream::out | std::ofstream::trunc); ofs.close();
	std::fstream fout("benchmark_out_info.txt", std::fstream::app);

	threads_number = 2 + getMaxThreadsCount() / 2;
	std::cout << "Initiating process" << std::endl;
	sample::test();
	std::cout << std::endl;

	benchpress::options opts;
	opts.bench(".*");
	opts.benchtime(1);
	opts.cpu(4);

	std::cout << "Benchmark is in progress" << std::endl;

	for (threads_number = 1; threads_number <= getMaxThreadsCount(); ++threads_number) {
		run(argc, argv);
		fout << "Threads used: " << threads_number << std::endl;
		fout << "CPU load: " << CpuLoad << std::endl;
		fout << std::endl << std::endl << std::endl;
	}

	fout.close();
	std::cout << "Benchmark finished successfully";
	PAUSE;
}

BENCHMARK("Sample: hilb(10)", [](benchpress::context* ctx) {
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i)
		sample::_hilb(10);
});

BENCHMARK("Sample: no thread 50x50", [](benchpress::context* ctx) {
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i)
		sample::_no_thread_50x50();
});

BENCHMARK("Sample: 50x50", [](benchpress::context* ctx) {
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i)
		sample::_50x50();
});

BENCHMARK("Sample: 500x500", [](benchpress::context* ctx) {
	ctx->reset_timer();
	for (size_t i = 0; i < ctx->num_iterations(); ++i) {
		init();
		sample::_500x500();
		CpuLoad = getCpuLoad();
	}
});
