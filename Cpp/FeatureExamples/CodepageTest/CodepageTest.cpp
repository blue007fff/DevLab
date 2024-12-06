#include <iostream>
#include <string>
#include <windows.h>
#include "../../helpers.h"

void CheckWcout()
{
	//출력 실패하면 wcout 가 깨질수 있음.
	//std::wstring wstr = L"안녕하세요! Hello! 𐍈";
	//std::wcout << wstr << std::endl;

	bool fail = std::wcout.bad();
	if (fail)
	{
		std::wcout.clear();
		std::wcout << std::endl << L"std::wcout.bad()\n";
	}
}

void TestCodepage1()
{
	helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;

	// CP949
	// CP65001(UTF-8)
	// CP1252(Windows-1252): 서유럽(영어, 독일어, 프랑스어 등)
	// CP850(DOS Latin-1): 라틴
	// CP936 (GBK, Simplified Chinese): 중국어 간체

	// 현재 cpp 파일은 CP65001(UTF-8) 기준.
	// str 에 문자열 메모리 저장 시 CP_ACP 기준으로 동작하여, CP949 로 저장.
	std::string str = "Codepage Test, 코드페이지 테스트"; // CP949

	// str 이 codepage 에 따라 다르게 해석 됨.

	//SetConsoleOutputCP(CP_ACP);
	UINT cp = GetConsoleOutputCP();
	std::cout << "CP(" << cp << "): " << str << std::endl;

	SetConsoleOutputCP(1252);
	cp = GetConsoleOutputCP();
	std::cout << "CP(" << cp << "): " << str << std::endl;

	SetConsoleOutputCP(936);
	cp = GetConsoleOutputCP();
	std::cout << "CP(" << cp << "): " << str << std::endl;

	//SetConsoleOutputCP(CP_ACP); //949 로 해석되어 제데로 출력되지 않음.
	SetConsoleOutputCP(CP_UTF8); //65501

	// utf8 로 해석되어 제대로 출력 되지 않음.
	cp = GetConsoleOutputCP();
	std::cout << "CP(" << cp << "): " << str << std::endl;

	cp = GetConsoleOutputCP();
	std::u8string u8str = u8"Codepage Test, 코드페이지 테스트";
	std::cout << "CP(" << cp << "): " << (char*)u8str.c_str() << std::endl;
}

void TestCodepage2()
{
	helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;
	SetConsoleOutputCP(CP_ACP); //949 복구

	// locale 를 변경하지 않으면 wcout 에서 제대로 출력되지 않음.
	std::wstring wstr = L"Codepage Test, 코드페이지 테스트";

	UINT cp = GetConsoleOutputCP();
	std::wcout << L"CP(" << cp << L"): " << wstr << std::endl;
	CheckWcout(); //출력 실패하면 버퍼가 깨질 수 있음.

	// print default locale
	{
		//std::setlocale(LC_ALL, "kor");
		char* locName = std::setlocale(LC_ALL, nullptr);
		std::cout << "Default Locale: " << locName << std::endl;
		std::cout << "Default Locale(std::locale().name()): "
			<< std::locale().name() << std::endl;
	}
	
	std::locale loc("ko_KR.UTF-8"); // "kor"(구버전)
	std::wcout.imbue(loc);
	std::wcout << L"CP(" << cp << L"): " << wstr << std::endl;
	// u16 -> u8 로 변환되어 출력되는 듯.
	// "C" 로케일은 ASCII 문자만 출력 가능. 변환이 안되면 무시 되는듯.
}

void TestCodepage3()
{
	helpers::PrintRepeatedChar('-', 50);
	std::cout << __FUNCTION__ << std::endl;
	SetConsoleOutputCP(CP_ACP); //949 복구
	// 949로도 잘 출력되는데 이유를 모르겠음.

	SetConsoleOutputCP(CP_UTF8); //65501
	std::wcout.imbue(std::locale("ko_KR.UTF-8"));

	const std::wstring texts[] =
	{
		L"한국: 안녕하세요",            // Korean
		L"스페인: Ñá",                // Spanish
		L"프랑스: forêt intérêt",     // French
		L"중국: 你好",                // Chinese
		L"일본: 日本人のビット",      // Japanese
		L"러시아: немного русский",   // Russian
		L"아랍어: مرحبا",            // Arabic
		L"히브리어: שלום",           // Hebrew
		L"베트남어: Xin chào",       // Vietnamese
		//L"이모지: 😀🌍"               // Emoji
	};
	for (auto& s : texts)
	{
		std::wcout << s << std::endl;
		CheckWcout();
	}
}

int main()
{
	TestCodepage1();

	TestCodepage2();

	TestCodepage3();	
}