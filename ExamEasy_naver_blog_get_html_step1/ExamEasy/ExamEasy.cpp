#include "pch.h"
#include "tipsware.h"
#include<string.h>

// HTTP 프로토콜을 사용하는 함수들이 정의된 wininet.h 헤더 파일을 추가한다.
#include <wininet.h>  
// wininet.h 헤더에 정의된 함수들을 사용하려면 wininet.lib 라이브러리를 추가해야 한다.
#pragma comment (lib, "wininet.lib")

DWORD ReadHtmlText(HINTERNET ah_http_file, char *ap_html_string)
{
	// 잠시 지연 효과를 메인 스레드에게만 사용하기 위해 이벤트 객체를 생성한다.
	HANDLE h_wait_event = CreateEvent(NULL, TRUE, 0, NULL);
	if (h_wait_event == NULL) return 0;  // 이벤트 객체 생성에 실패한 경우

	char buffer[1025];
	DWORD read_byte = 0, error_count = 0, total_bytes = 0;
	// 1024 바이트 단위로 HTML 코드를 가져옴
	while (InternetReadFile(ah_http_file, buffer, 1024, &read_byte)) {
		// 1024 단위로 가져온 HTML 소스를 ap_html_string에 계속 추가한다.
		memcpy(ap_html_string + total_bytes, buffer, read_byte);
		total_bytes += read_byte;  // 읽은 전체 바이트 수를 갱신한다.
		if (read_byte < 1024) {
			// 1024바이트씩 요청하더라도 해당 페이지의 HTML 소스가 1024보다 작거나 
			// 네트워크 지연으로 더 작은 크기가 전송될수 있음. (10번 정도 재시도함)
			error_count++;
			if (error_count > 10) break;
			else WaitForSingleObject(h_wait_event, 50); // 50ms 지연하도록 구성한다.
		} else error_count = 0;
	}

	*(ap_html_string + total_bytes) = 0;  // 문자열의 끝에 NULL 문자를 추가함!
	CloseHandle(h_wait_event);  // 지연 효과를 위해 생성한 이벤트 객체를 제거한다.
	return total_bytes;  // 읽어들인 전체 바이트 수를 반환한다.
}


void LoadDataFromWebPage()  // 웹 페이지를 구성하는 HTML 소스를 가져오는 함수
{
	char url[256];
	// 1001번 에디트 컨트롤에 입력된 웹 페이지 주소를 url 배열에 복사한다.
	GetCtrlName(FindControl(1001), url, 256);
	// 인터넷을 사용할 세션을 구성한다.
	HINTERNET h_session = InternetOpen("MovieScanner", PRE_CONFIG_INTERNET_ACCESS, NULL, INTERNET_INVALID_PORT_NUMBER, 0);
	// 어떤 사이트에 접속할지 구성한다. (네이버 영화를 사용하도록 구성한다.)
	HINTERNET h_connect = InternetConnect(h_session, "movie.naver.com", INTERNET_INVALID_PORT_NUMBER, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	// 네이버 영화에서 원하는 웹 페이지에 접속한다. (url 배열에 저장된 페이지를 연다.)
	HINTERNET h_http_file = HttpOpenRequest(h_connect, "GET", url, HTTP_VERSION, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);

	char *p_utf8_html_str = (char *)malloc(2*1024*1024);  // 2M Bytes 메모리 할당!
	if (p_utf8_html_str != NULL) {  // 메모리 할당에 성공했다면!
		if (HttpSendRequest(h_http_file, NULL, 0, 0, 0) == TRUE) { // 웹 페이지 소스를 요청한다.
			// 웹 페이지 전체 소스를 할당된 메모리에 복사한다.
			ReadHtmlText(h_http_file, p_utf8_html_str);

			char *p_ascii_str = NULL;
			// 가져온 웹 페이지 소스가 UTF8 형식의 문자열이라서 ASCII 형식의 문자열로 변환합니다.
			Utf8ToAscii(&p_ascii_str, p_utf8_html_str);
			if (p_ascii_str != NULL) {  // 변환에 성공했다면
				// 변환된 HTML 소스를 1000번 에디트 컨트롤에 표시한다.
				SetCtrlName(FindControl(1000), p_ascii_str);



				free(p_ascii_str);  // 변환에 사용한 메모리를 해제한다.
			}
		}
		free(p_utf8_html_str);  // 웹 페이지 소스를 저장하기 위해 할당했던 메모리를 해제한다.
	}

	// 웹 페이지를 사용하기 위해 할당했던 핸들을 모두 해제한다.
	if (h_http_file != NULL) InternetCloseHandle(h_http_file);
	if (h_connect != NULL) InternetCloseHandle(h_connect);
	if (h_session != NULL) InternetCloseHandle(h_session);
}

// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void *ap_ctrl)
{
	// 1010번 버튼("웹 페이지 소스 가져오기" 버튼)이 눌러진 경우
	if (a_ctrl_id == 1010) LoadDataFromWebPage();
}



// 컨트롤을 조작했을 때 호출할 함수 등록
CMD_MESSAGE(OnCommand)

int main()
{
	ChangeWorkSize(800, 600); // 작업 영역을 설정한다.
	Clear(0, RGB(82, 97, 124)); // 작업 영역을 RGB(82, 97, 124) 색으로 채운다!

	// 웹 페이지 소스를 보여줄 에디트 컨트롤을 생성한다. 
	// (여러 줄 사용 모드, 가로/세로 자동 스크롤, 수직 스크롤바 사용, 엔터키 사용 모드)
	CreateEdit(5, 5, 790, 250, 1000, ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL);


	CreateEdit(5, 280, 790, 250, 1002, ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL);

	// 웹 페이지 주소를 입력할 에디트 컨트롤을 생성한다.
	void *p = CreateEdit(210, 570, 385, 24, 1001, 0);
	// 소스를 가져올 샘플 웹페이지 URL을 에디트 컨트롤에 표시합니다!

	SetCtrlName(p, "movie/running/premovie.nhn?festival=N");

	CreateButton("웹 페이지 소스 가져오기", 5, 568, 200, 28, 1010);  // 버튼 생성!

	ShowDisplay(); // 정보를 윈도우에 출력한다.	
	return 0;
}