#ifndef _TIPSWARE_H_
#define _TIPSWARE_H_

/*  [ 기본 샘플 코드 ]
#include "pch.h"
#include "tipswre.h"

NOT_USE_MESSAGE

int main()
{
	// 구구단에서 2단을 출력한다.
	for (int i = 1; i < 10; i++) {
		// (20, 5+i*20) 좌표에  RGB(0, 200, 255) 색상으로 출력!
		printf(20, 5+i*20, RGB(0, 200, 255), "2 * %d = %d", i, 2 * i);
	}

	ShowDisplay(); // 정보를 윈도우에 출력한다.
	return 0;
}
*/

#include "framework.h"
#include "resource.h"
#include "EasyWin32.h"

#ifdef _DEBUG
	#pragma comment(lib, "DSH_EasyWin32.lib")
#else
	#pragma comment(lib, "RST_EasyWin32.lib")
#endif

// 아래의 두 변수 선언은 필수 사항입니다!! (응용 프로그램 이름과 창 제목 설정)
const char *gp_app_name = "EasyMyWindow";
const char *gp_window_title = "https://blog.naver.com/tipsware";

using namespace EasyAPI_Tipsware;

#define printf TextOut

#define NOT_USE_MESSAGE SystemMessageMap g_system_map(NULL, NULL, NULL, NULL);
#define MOUSE_MESSAGE(left_down, left_up, move) SystemMessageMap g_system_map(left_down, left_up, move, NULL);
#define MOUSE_CMD_MESSAGE(left_down, left_up, move, command) SystemMessageMap g_system_map(left_down, left_up, move, command);
#define CMD_MESSAGE(command) SystemMessageMap g_system_map(NULL, NULL, NULL, command);

#endif