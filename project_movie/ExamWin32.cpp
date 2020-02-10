#include "pch.h"
#include <stdio.h>
#include "tipsware.h"

#define IDC_STR_EDIT 1005
#define IDC_OPEN_FILE_BTN 1000
#define IDC_SAVE_FILE_BTN 1001

struct AppData  // 프로그램에서 사용할 내부 데이터
{
    TargetData* p_target;  // 매크로 대상의 정보
};


void OpenTextFile()   // 사용자가 선택한 파일을 열어서 에디트 컨트롤에 보여주는 함수
{
    char path[MAX_PATH];  // 사용자가 선택한 텍스트 파일의 경로를 저장할 변수
    if (ChooseOpenFileName(path, MAX_PATH)) {  // 파일 열기 대화 상자를 출력해서 파일을 선택하게 한다.
        // 선택한 파일을 읽어서 에디트 컨트롤에 표시한다.
        Edit_ReadTextFromFile(FindControl(IDC_STR_EDIT), path);
    }
}

void SaveTextFile()  // 에디트 컨트롤에 적힌 문자열을 사용자가 선택한 파일에 저장한다.
{
    char path[MAX_PATH]; // 사용자가 선택한 텍스트 파일의 경로를 저장할 변수
    if (ChooseSaveFileName(path, MAX_PATH)) { // 저장할 파일의 이름을 사용자가 선택하게 한다.
        // 에디트 컨트롤에 적힌 문자열을 텍스트 파일에 저장한다.
        Edit_SaveTextToFile(FindControl(IDC_STR_EDIT), path);
    }
}


// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void* ap_ctrl)
{
    
    AppData* p_data = (AppData*)GetAppData();  // 응용 프로그램의 내부 데이터 주소를 가져온다.

    if (a_ctrl_id == IDC_OPEN_FILE_BTN) OpenTextFile();   // [파일 열기] 버튼을 선택한 경우
    else if (a_ctrl_id == IDC_SAVE_FILE_BTN) SaveTextFile();   // [파일 저장] 버튼을 선택한 경우



    if (a_ctrl_id == 1002) {
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

    CreateEdit(10, 10, 500, 30, 1006,NULL);
    CreateButton("검색", 520, 10,50,28,1007 );


    CreateButton("1", 550, 60, 30, 28, 1010);

    CreateButton("2", 600, 60, 30, 28, 1011);

    CreateButton("3", 650, 60, 30, 28, 1012);

    CreateButton("4", 700, 60, 30, 28, 1013);

    CreateButton("5", 750, 60, 30, 28, 1014);

    CreateListBox(10, 50, 500, 250, 1003);
    void *p = CreateListBox(530, 100, 600, 600, 1004,DrawDrinkItem);
    CreateEdit(10, 370, 500, 350, IDC_STR_EDIT,
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
    

    CreateButton("메모장 윈도우 찾기", 10, 320, 160, 28, 1002);
    // 파일 열기, 저장 버튼을 생성한다.
    CreateButton("파일 열기", 5, 730, 110, 28, IDC_OPEN_FILE_BTN);
    CreateButton("파일 저장", 118, 730, 110, 28, IDC_SAVE_FILE_BTN);


    ShowDisplay(); // 정보를 윈도우에 출력한다.
    return 0;
}