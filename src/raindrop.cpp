#include "pch.h"

#include "Screen.h"
#include "Application.h"

#ifdef TESTS
#define CATCH_CONFIG_RUNNER
#include "ext/catch.hpp"
#endif 

// ErrorReporting.cpp
void RegisterSignals();

#ifdef _WIN32
void PrintTraceFromContext(CONTEXT &ctx);
#endif

void RunApplication(int argc, char** argv) {
#ifndef TESTS
	Application App(argc, argv);
	App.Init();
	App.Run();
	App.Close();
#else
	Catch::Session session;
	session.applyCommandLine(argc, argv);

	std::cout << session.run();

	std::cin.get();
#endif
}

int main(int argc, char *argv[])
{
	RegisterSignals();
	
#ifdef _WIN32
	int cd = 0;
	_EXCEPTION_POINTERS *ex_info;
	__try {
#endif
		RunApplication(argc, argv);
#ifdef _WIN32
	} 
	__except ( ex_info = GetExceptionInformation(),
		GetExceptionCode() == STATUS_ACCESS_VIOLATION ||
		GetExceptionCode() == STATUS_ARRAY_BOUNDS_EXCEEDED) {
		PrintTraceFromContext(*ex_info->ContextRecord);

		MessageBox(nullptr, 
			L"raindrop has crashed. Please see the log for details and maybe send it to github with a report of what you were doing to try and fix it.", 
			L"Crash!", MB_OK | MB_ICONERROR);
	}
#endif

    return 0;
}