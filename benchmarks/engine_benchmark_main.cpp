#include <benchmark/benchmark.h>

#include <iostream>
using namespace std;


int main(int argc, char** argv)
{
	::benchmark::Initialize(&argc, argv);
	if(::benchmark::ReportUnrecognizedArguments(argc, argv))
	{
		return 1;
	}
	::benchmark::RunSpecifiedBenchmarks();

	cin.get();
}
