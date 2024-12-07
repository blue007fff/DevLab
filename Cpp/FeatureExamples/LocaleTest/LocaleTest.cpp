#include <iostream>
#include <random>
#include <format>
#include <iomanip>
#include <map>
#include <windows.h>
#include <locale>

#include "../../helpers.h"

namespace
{
	auto CheckStreamStatus = [](std::ostream& stream) 
	{
		// stream.good(), stream.eof(), stream.fail()
		if (stream.bad())
		{
			stream.clear();
			std::cout << std::endl;
			std::cerr << "stream.bad()" << std::endl;
		}
	};

	std::vector<std::locale> GetLocales()
	{
		std::vector<std::string> localeStrings	{
			"error",
			"C",			// default
			"ko_KR.UTF-8",  // 한국어
			"en_US.UTF-8",  // 영어 (미국)
			"fr_FR.UTF-8",  // 프랑스어
			"de_DE.UTF-8",  // 독일어
			//"zh_CN.UTF-8",  // 중국어 (중국)
			//"ja_JP.UTF-8",  // 일본어 (일본)
			"ar_AE.UTF-8"   // 아랍어 (아랍에미리트)
		};

		std::vector<std::locale> locales;
		for (auto& localeString : localeStrings)
		{
			try
			{
				const char* oldLocale = std::setlocale(LC_ALL, localeString.c_str());
				const char* curLocale = std::setlocale(LC_ALL, nullptr);
				if (oldLocale == nullptr)
				{
					throw std::exception(localeString.c_str());
				}
				std::locale loc(localeString);
				locales.push_back(loc);
			}
			catch (const std::exception& e)
			{
				std::cout << "invalid locale: " << e.what() << std::endl;
			}
		}
		return locales;
	}

	void PrintCurrency(double money)
	{
		//std::cout << std::left << std::setw(10) << std::locale().name() << ": ";
		std::cout << std::showbase << std::put_money(money) << std::endl;
	}

	void PrintNumeric(double number)
	{
		//std::cout << std::left << std::setw(10) << std::locale().name() << ": ";
		//std::cout << std::format("{:<8}: ", "Number");
		//std::cout << std::format("{:0.2f}", number) << std::endl;
		std::cout << std::fixed << std::setprecision(2) << number << std::endl;
	}

	void PrintLocalTime()
	{
		wchar_t buf[100];
		std::tm localTime = helpers::GetLocaleTime();
		std::wcsftime(buf, std::size(buf), L"%A %c", &localTime);
		//std::wprintf(L"Date: %Ls\n", buf);
		std::wcout << "Date: " << buf << std::endl;
	}
}


void LocaleTest1()
{
	helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;
	std::string oldLocaleName = std::setlocale(LC_ALL, nullptr);

	double number = 1234567.89;
	const wchar_t* strHi = L"안녕하세요";
	std::string currLocaleName;
	auto print = [&]() {
		printf("print: %s: %.2lf\n", currLocaleName.c_str(), number);
		std::cout << "std::cout: " << currLocaleName << ": "
			<< std::fixed << number << std::setprecision(2) << std::endl;
		wprintf(L"%s\n", strHi);
	};

	// setLocale, std::setLocale 동일.
	currLocaleName = std::setlocale(LC_ALL, "C");
	std::cout.imbue(std::locale("C"));
	print();

	currLocaleName = std::setlocale(LC_ALL, "de_DE.UTF-8");
	std::cout.imbue(std::locale("de_DE.UTF-8"));
	print();

	//std::locale::global(std::locale("de_DE.UTF-8"));
	std::ostringstream oss;
	//oss.imbue(std::locale("de_DE.UTF-8"));
	oss << std::fixed << std::setprecision(2) << number;
	std::cout << "std::ostringstream: " << oss.str() << std::endl;

	// 날짜 출력 테스트
	std::cout.imbue(std::locale("ko_KR.UTF-8"));
	std::tm localTime = helpers::GetLocaleTime();
	std::cout << std::put_time(&localTime, "%c") << std::endl;
}

void LocaleTest2()
{
	helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;

	helpers::PrintRepeatedChar('-', 30);
	auto locales = GetLocales();
	for (auto& locale : locales)
	{
		//std::setlocale(LC_ALL, locale.name().c_str());
		std::locale::global(locale);
		PrintLocalTime();
	}

	helpers::PrintRepeatedChar('-', 30);
	for (auto& locale : locales)
	{
		std::cout.imbue(locale);
		PrintCurrency(1234567.89);
		CheckStreamStatus(std::cout);
	}

	helpers::PrintRepeatedChar('-', 30);
	for (auto& locale : locales)
	{
		std::cout.imbue(locale);
		PrintNumeric(1234567.89);
		CheckStreamStatus(std::cout);
	}
}


void LocaleTest3()
{
	helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;

	// 로케일 특정 카테고리 변경.
	{
		std::string monetaryLocaleName = std::setlocale(LC_MONETARY, "en_US.UTF-8");
		std::string currLocaleName = std::setlocale(LC_ALL, nullptr);
		std::cout << "LC_MONETARY(C): " << monetaryLocaleName << std::endl;
		std::cout << "LC_ALL(C): " << currLocaleName << std::endl;
	}

	// 일부 카테고리 로케일 변경
	// TODO: 방법이 맞는지 모르겠음 나중에 확인 필요.
#if 0
	{
		std::cout << "LC_ALL(CPP): " << std::locale().name() << std::endl;
		std::locale::global(std::locale("ko_KR.UTF-8"));

		std::locale newloc = std::locale();
		newloc = std::locale(newloc, std::locale("de_DE.UTF-8"), std::locale::numeric);
		std::locale::global(newloc);
		std::cout << "LC_ALL(CPP): " << newloc.name() << std::endl;
		std::cout << "LC_ALL(CPP): " << std::locale().name() << std::endl;

		std::cout.imbue(newloc);
		PrintNumeric(123456.789);
		PrintCurrency(123456.789);
		PrintLocalTime();
	}
#endif	
}

int main()
{
	// 지역별 문화적 관습에 맞게 데이터를 처리하는 설정. 
	// 숫자, 날짜, 시간, 화폐, 문자 정렬 등.

	// setlocale(), std::setlocale() 은 동일.
	// std::setlocale(): C 함수에 영향(printf,...)
	// std::locale::global(loc) : Cpp 함수에 영향.
	// - 이후에 생성되는 스트림에 영향.
	// - cout, wcout 는 이미 객체가 생성된 상태라 영향이 없음.
	// - std::ostringstream 에는 영향을 미침.
	// std::cout.imbue(loc): local 로 설정 가능.

	LocaleTest1();
	std::cout << std::endl;


	SetConsoleOutputCP(CP_UTF8); // 출력에 UTF-8 인코딩 설정
	//SetConsoleCP(CP_UTF8);       // 입력에 UTF-8 인코딩 설정
	LocaleTest2();
	std::cout << std::endl;

	LocaleTest3();

	return 0;
}