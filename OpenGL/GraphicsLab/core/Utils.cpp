#include "Utils.h"
#include <glad/gl.h>
#include <windows.h>

namespace utils
{
	std::filesystem::path GetExecutablePath()
	{
		// Windows에서 실행 파일 경로 얻기
		wchar_t buffer[MAX_PATH];

		// NULL은 현재 실행 중인 모듈(실행 파일)을 의미
		GetModuleFileName(NULL, buffer, MAX_PATH);
		return std::filesystem::path(buffer);
	}
}
