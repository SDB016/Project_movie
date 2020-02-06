#include "pch.h"
#include <stdio.h>
#include "tipsware.h"

#define IDC_STR_EDIT 1005

struct AppData  // 프로그램에서 사용할 내부 데이터
{
    TargetData* p_target;  // 매크로 대상의 정보
};

// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void* ap_ctrl)
{
    AppData* p_data = (AppData*)GetAppData();  // 응용 프로그램의 내부 데이터 주소를 가져온다.

    if (a_ctrl_id == 1000) {
        // 메모장 프로그램의 'Window Class' 명칭이 'notepad' 입니다. 따라서
        // 'notepad' 이름으로 대상을 검색합니다. 대상을 찾았다면 NULL 아닌 값이 저장됩니다.
        p_data->p_target = FindTargetImage(0, "notepad", NULL);
    }
    else if (a_ctrl_id == 1001) {
        if (p_data->p_target != NULL) {
            char str[64];  // 에디트 컨트롤에 입력된 문자열을 저장할 배열 변수
            int x, y;
            // 메모장의 클라이언트 영역에서 (10, 10) 지점을 화면 좌표로 변환합니다.
            GetMousePosFromTarget(p_data->p_target, &x, &y, 10, 10);
            // 메모장에 글을 쓰기 위해서 (x, y)에 마우스를 클릭해서 메모장을 선택합니다.
            MouseClickWrite(x, y);
            // 에디트 컨트롤에 저장된 문자열을 str 배열에 복사한다.
            GetCtrlName(FindControl(1010), str, 64);
            // 한글 모드라고 가정하고 str 배열에 저장된 문자열을 메모장에 씁니다.
            InputNormalString(str, 1);
        }
    }
    else if (a_ctrl_id == 1002) {
        void* p_list_box = FindControl(1003);
        ListBox_ResetContent(p_list_box);

        char str[64];
        HWND h_cur_wnd = NULL;

        while (1) {
            h_cur_wnd = ::FindWindowEx(NULL, h_cur_wnd, "notepad", NULL);
            if (h_cur_wnd != NULL) {
                ::GetWindowText(h_cur_wnd, str, 64);
                ListBox_InsertString(p_list_box, -1, str, 1);
            }else break;
        }
    }
}

// 각 음료수별 칼로리를 출력한다.
void DrawDrinkItem(int index, char* ap_str, int a_str_len, void* ap_data, int a_is_selected, RECT* ap_rect)
{

    // 각 항목의 배경을 그린다.
    SelectBrushObject(RGB(62, 77, 104));
    Rectangle(ap_rect->left, ap_rect->top, ap_rect->right, ap_rect->bottom);

    SelectFontObject("굴림", 12);  // 글꼴을 '굴림', 12 크기로 설정한다.
    // 음료수 이름과 칼로리 수치를 출력한다.
    TextOut(ap_rect->left + 6, ap_rect->top + 3, RGB(255, 255, 255), ap_str);

}




// 컨트롤을 조작할 때 호출할 함수를 설정한다.
CMD_MESSAGE(OnCommand)

int main()
{
    int width = 1200, height = 800;
    ChangeWorkSize(width, height); // 작업 영역을 설정한다.

    Clear(0, RGB(72, 87, 114));

    AppData data = { NULL };  // 프로그램이 내부적으로 사용할 메모리를 선언한다.
    SetAppData(&data, sizeof(data));  // 지정한 변수를 내부 데이터로 저장한다.

   // 프로그램에서 사용할 버튼을 생성합니다.
    CreateButton("메모장 찾기", 10, 10, 105, 28, 1000);
    CreateButton("메모장에 글쓰기", 120, 10, 105, 28, 1001);
    CreateListBox(10, 100, 500, 250, 1003);
    void *p = CreateListBox(530, 100, 600, 600, 1004,DrawDrinkItem);
    CreateEdit(10, 420, 500, 350, IDC_STR_EDIT,
        ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL);

    FILE* p_file = NULL;
    char drink_name[64];
    // drink.txt 파일을 텍스트 읽기 모드로 연다.
    if (0 == fopen_s(&p_file, "테스트.txt", "rt")) {
        // 파일에서 문자열 1개를 읽는다.
        while (fscanf_s(p_file, "%s", drink_name, 64) == 1) {
            // 읽어들인 문자열과 정숫값을 리스트 박스에 추가한다.
            ListBox_SetItemDataEx(p, -1, drink_name, 0);
        }
        fclose(p_file);  // 파일을 닫는다!
    }

    SelectFontObject("굴림", 12); // 글꼴을 '굴림', 12 크기로 변경한다.
    

    CreateButton("메모장 윈도우 찾기", 10, 370, 160, 28, 1002);
    // 메모장에 쓸 문자열을 입력할 에디트 컨트롤을 생성한다.
    CreateEdit(10, 50, 500, 42, 1000, 0);
    ShowDisplay(); // 정보를 윈도우에 출력한다.
    return 0;
}