#include "pch.h"
#include "tipsware.h"

// 프로그램에서 사용할 내부 데이터
struct AppData
{
	void *p_select_btn;  // 선택한 버튼의 주소를 기억하는 변수
};

// 버튼 선택에 따라 버튼 이름을 버튼 아래쪽에 출력하는 함수
void DrawName(AppData *ap_data, int a_index)
{
	// 각 버튼을 눌렀을 때 사용할 색상을 테이블로 만든다!
	COLORREF color[3] = { RGB(100, 200, 255), RGB(200, 100, 255), RGB(200, 200, 255) };
	// 이름이 출력될 위치에 사각형을 그린다.
	Rectangle(10, 45, 308, 100, RGB(0, 0, 200), color[a_index]);

	SelectFontObject("굴림", 26, 1); // '굴림', 26크기, 강조 형식의 글꼴을 설정한다.
	char name[64]; // 컨트롤의 이름을 저장할 변수
	GetCtrlName(ap_data->p_select_btn, name, 64); // 선택한 컨트롤의 이름을 얻는다.
	TextOut(30, 60, RGB(0, 0, 255), name); // 이름을 출력한다.
}

// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void *ap_ctrl)
{
	// 응용 프로그램의 내부 데이터 주소를 가져온다.
	AppData *p = (AppData *)GetAppData();

	// 1000, 1001, 1002 아이디를 가진 버튼이 눌러진 경우 처리한다.
	// 만약, 동일한 버튼이 연속으로 눌러진 경우에는 처리하지 않는다. (ap_ctrl != p->p_select_btn)
	if (a_ctrl_id >= 1000 && a_ctrl_id <= 1002 && ap_ctrl != p->p_select_btn) {
		// 기존에 선택되어 색상이 다르게 표시된 버튼은 다시 기본 색상으로 복구한다.
		ChangeCtrlColor(p->p_select_btn, RGB(87, 101, 126), RGB(107, 121, 146), RGB(157, 171, 196), RGB(232, 248, 248));
		// 버튼을 갱신한다. (변경된 색상으로 다시 보여짐)
		Invalidate(p->p_select_btn);
		// 새롭게 선택된 버튼의 주소를 기록한다.
		p->p_select_btn = ap_ctrl;
		// 선택된 버튼의 색상을 파란색으로 변경한다.
		ChangeCtrlColor(p->p_select_btn, RGB(32, 64, 192), RGB(0, 0, 64), RGB(82, 112, 255), RGB(200, 255, 255));
		// 버튼을 갱신한다. (변경된 색상으로 다시 보여짐)
		Invalidate(p->p_select_btn);
		// 버튼의 선택에 따른 문자열을 출력한다. (a_ctrl_id - 1000)의 값은 0, 1, 2이다.
		DrawName(p, a_ctrl_id - 1000);
		ShowDisplay(); // 정보를 윈도우에 출력한다.
	} 
}

// 컨트롤을 조작했을 때 호출할 함수 등록
CMD_MESSAGE(OnCommand)

int main()
{
	// 응용 프로그램이 사용할 내부 데이터 변수 선언!
	AppData data;
	// 윈도우의 그리기 영역을 RGB(62, 77, 104) 색으로 채운다!
	Clear(0, RGB(62, 77, 104));

	// 그리고 첫 번째 버튼을 선택 버튼으로 저장한다. CreateButton 함수의 반환 값이 객체의 주소이다.
	data.p_select_btn = CreateButton("팁스웨어", 10, 10, 98, 28, 1000);
	CreateButton("팁스소프트", 110, 10, 98, 28, 1001);
	CreateButton("김성엽", 210, 10, 98, 28, 1002);

	// 선택된 버튼은 색상을 다르게 표시(파란색)한다.
	ChangeCtrlColor(data.p_select_btn, RGB(32, 64, 192), RGB(0, 0, 64), RGB(82, 112, 255), RGB(200, 255, 255));
	// 버튼 선택에 따른 버튼 이름을 출력하는 함수를 호출한다.
	DrawName(&data, 0);
	// 현재 상태를 응용 프로그램 내부 데이터에 기록한다.
	SetAppData(&data, sizeof(AppData));

	ShowDisplay(); // 정보를 윈도우에 출력한다.
	return 0;
}
