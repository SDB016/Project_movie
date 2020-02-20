#include "pch.h"
#include <stdio.h>
#include "tipsware.h"



NOT_USE_MESSAGE{}
// 컨트롤을 조작할 때 호출할 함수를 설정한다.
//CMD_MESSAGE(OnCommand)

int main()
{
    int width = 1200, height = 800;
    ChangeWorkSize(width, height); // 작업 영역을 설정한다.

    Clear(0, RGB(72, 87, 114));


    CreateEdit(10, 10, 500, 30, 1006,NULL);
    CreateButton("검색", 520, 10,50,28,1007 );


    CreateButton("1",   550, 60, 30, 28, 1010);
    CreateButton("2", 600, 60, 30, 28, 1011);
    CreateButton("3", 650, 60, 30, 28, 1012);
    CreateButton("4", 700, 60, 30, 28, 1013);
    CreateButton("5", 750, 60, 30, 28, 1014);

    CreateListBox(10, 50, 500, 250, 1015);

    FILE* p_file = NULL;
    char drink_name[64];

    SelectFontObject("굴림", 12); // 글꼴을 '굴림', 12 크기로 변경한다.
    

    CreateButton("메모장 윈도우 찾기", 10, 320, 160, 28, 1000);
    // 파일 열기, 저장 버튼을 생성한다.
    CreateButton("파일 열기", 5, 730, 110, 28, 1001);
    CreateButton("파일 저장", 118, 730, 110, 28, 1002);


    ShowDisplay(); // 정보를 윈도우에 출력한다.
    return 0;
}