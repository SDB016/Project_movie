#include "pch.h"
#include "tipsware.h"

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
			else WaitForSingleObject(h_wait_event, 5); // 5ms 지연하도록 구성한다.
		}
		else error_count = 0;
	}

	*(ap_html_string + total_bytes) = 0;  // 문자열의 끝에 NULL 문자를 추가함!
	CloseHandle(h_wait_event);  // 지연 효과를 위해 생성한 이벤트 객체를 제거한다.
	return total_bytes;  // 읽어들인 전체 바이트 수를 반환한다.
}

// 분석에 의미없는 문자들을 제거하는 함수!
void RemoveMeaninglessChar(char *ap_string)
{
	// 탭, 줄바꿈 문자가 나올때까지 반복한다.
	while (*ap_string) {
		if (*ap_string == '\t' || *ap_string == '\r' || *ap_string == '\n') break;
		ap_string++;
	}
	if (!*ap_string) return;  // 전체적으로 탭, 줄바꿈 문자가 없는 경우! 

	char *p_dest = ap_string++;
	while (*ap_string) {
		// 탭, 줄바꿈 문자가 아닌 경우에만 복사를 진행한다.
		if (*ap_string != '\t' && *ap_string != '\r' && *ap_string != '\n') {
			*p_dest++ = *ap_string;
		}
		ap_string++;
	}
	*p_dest = 0;  // 문자열의 끝에 NULL 문자를 추가한다.
}

void LoadDataFromWebPage()  // 웹 페이지를 구성하는 HTML 소스를 가져오는 함수
{
	// 인터넷을 사용할 세션을 구성한다.
	HINTERNET h_session = InternetOpen("NaverWeatherScanner", PRE_CONFIG_INTERNET_ACCESS, NULL, INTERNET_INVALID_PORT_NUMBER, 0);
	// 어떤 사이트에 접속할지 구성한다. (네이버 날씨를 사용하도록 구성한다.)
	HINTERNET h_connect = InternetConnect(h_session, "weather.naver.com", INTERNET_INVALID_PORT_NUMBER, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	// 네이버 날씨 페이지에서 서울 지역(CT001013)과 관련된 정보를 얻는다.
	HINTERNET h_http_file = HttpOpenRequest(h_connect, "GET", "rgn/cityWetrCity.nhn?cityRgnCd=CT001013", HTTP_VERSION, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);

	char *p_utf8_html_str = (char *)malloc(1024*1024);  // 1M Bytes 메모리 할당!
	if (p_utf8_html_str != NULL) {  // 메모리 할당에 성공했다면!
		if (HttpSendRequest(h_http_file, NULL, 0, 0, 0) == TRUE) { // 웹 페이지 소스를 요청한다.
			// 웹 페이지 전체 소스를 할당된 메모리에 복사한다. (65Kbytes 정도의 내용이다)
			ReadHtmlText(h_http_file, p_utf8_html_str);
			RemoveMeaninglessChar(p_utf8_html_str);

			char *p_ascii_html_str = NULL;
			// 가져온 웹 페이지 소스가 UTF8 형식의 문자열이라서 ASCII 형식의 문자열로 변환합니다.
			Utf8ToAscii(&p_ascii_html_str, p_utf8_html_str);
			if (p_ascii_html_str != NULL) {  // 변환에 성공했다면
				// 변환된 HTML 소스를 1000번 에디트 컨트롤에 표시한다.
				SetCtrlName(FindControl(1000), p_ascii_html_str);
				free(p_ascii_html_str); // 변환에 사용했던 메모리를 해제한다.
			}
		}
		free(p_utf8_html_str);  // 웹 페이지 소스를 저장하기 위해 할당했던 메모리를 해제한다.
	}
	// 웹 페이지를 사용하기 위해 할당했던 핸들을 모두 해제한다.
	if (h_http_file != NULL) InternetCloseHandle(h_http_file);
	if (h_connect != NULL) InternetCloseHandle(h_connect);
	if (h_session != NULL) InternetCloseHandle(h_session);
}

// ap_src_str 문자열에서 [ap_start_word]에서 시작해서 [ap_end_word]이 나올때까지의 문자열을 
// 복사하는 함수입니다. 공백 문자는 복사하지 않고 <  > 로 포함된 문자열도 제외시킨다.
char *CopyTextFromWebData(char *ap_dest_str, char *ap_src_str, const char *ap_start_word, const char *ap_end_word)
{
	char *p_pos = strstr(ap_src_str, ap_start_word);  // 시작 위치를 찾는다.
	if (p_pos != NULL) {
		char *p_limit_pos = strstr(p_pos, ap_end_word);  // 끝 위치를 찾는다.
		if (p_limit_pos != NULL) {
			p_pos += strlen(ap_start_word);  // 시작 단어에 해당하는 문자열은 제외 시킨다.
			char open_flag = 0;  // < > 괄호가 열려있는지 기억하는 변수. 0이면 닫힘, 1이면 열림.
			while (p_pos < p_limit_pos) { // 끝 위치까지 탐색한다.
				if (*p_pos == '<') open_flag = 1;  // 괄호가 열림
				else if (*p_pos == '>') open_flag = 0;  // 괄호가 닫힘
				else {
					if (!open_flag) { // 괄호가 닫힌 상태
						if (*p_pos == '|') {  // '|' 문자는 ", "로 변경해서 추가한다.
							*ap_dest_str++  = ',';  *ap_dest_str++  = ' ';
						} else {  // 공백은 연속으로 추가되지 않게 한다.
							if (*(ap_dest_str - 1) != ' ' || *p_pos != ' ') *ap_dest_str++  = *p_pos;
						}
					}
				}
				p_pos++;
			}
			*ap_dest_str = 0;  // 문자열의 끝에 NULL 문자를 추가한다.
			return p_pos;  // 문자열 처리가 끝난 위치를 반환한다.
		}
	}
	return ap_src_str;  // 작업에 실패했으면 전달받은 위치를 그대로 반환한다.
}


void ShowWeatherInSeoul()  // HTML 정보를 사용해서 서울 날씨 정보 구성하기!
{
	void *p_edit = FindControl(1000);  // 에디트 컨트롤의 주소를 얻는다.
	int len = Edit_GetLength(p_edit) + 1;  // 에디트 컨트롤에 저장된 문자열의 길이를 얻는다.
	char time_str[16], temperature[16], state[64], state_ex[128];
	char *p_html_str = (char *)malloc(len); // 에디트 컨트롤에 저장된 문자열의 길이만큼 메모리 할당!
	if (p_html_str != NULL) {
		GetCtrlName(p_edit, p_html_str, len);  // 에디트 컨트롤에 저장된 문자열을 p_html_str에 복사한다.

		char *p_pos = strstr(p_html_str, "<h4 class=\"first\">");  // 시작 위치를 찾는다!
		if (p_pos != NULL) {
			p_pos += 18;   // +18은 <h4 class="first"> 문자열 Skip!!

			p_pos = CopyTextFromWebData(time_str, p_pos, "<h5>", "</h5>");  // 시간 복사!
			p_pos = CopyTextFromWebData(temperature, p_pos, "<em>", "</span>");  // 온도 복사!

			p_pos = CopyTextFromWebData(state, p_pos, "<strong>", "</strong>");   // 오늘 날씨 상태 복사!
			p_pos = CopyTextFromWebData(state_ex, p_pos, "</em>", "<br><a href"); // 추가 날씨 상태 복사!

			SelectFontObject("굴림", 36, 1);
			TextOut(32, 250, RGB(200, 255, 255), temperature);  // 온도 출력!

			SelectFontObject("굴림", 16, 1);
			TextOut(10, 220, RGB(232, 232, 232), time_str);  // 시간 출력!
			TextOut(10, 290, RGB(255, 255, 200), "%s, %s", state, state_ex); // 날씨 출력!
		}

		free(p_html_str);  // 에디트 컨트롤에 저장된 문자열을 복사하기 위해 할당했던 메모리 해제!
		ShowDisplay(); // 정보를 윈도우에 출력한다.	
	}
}

// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void *ap_ctrl)
{
	if (a_ctrl_id == 1010) LoadDataFromWebPage();  // "날씨 페이지 소스 가져오기" 버튼이 눌러짐!
	else if (a_ctrl_id == 1011) ShowWeatherInSeoul();  // "서울 날씨 보기" 버튼이 눌러짐!
}

// 컨트롤이 조작되면 OnCommand 함수를 호출한다.
CMD_MESSAGE(OnCommand)

int main()
{
	ChangeWorkSize(600, 400); // 작업 영역을 설정한다.
	Clear(0, RGB(82, 97, 124)); // 작업 영역을 RGB(82, 97, 124) 색으로 채운다!

	// 웹 페이지 소스를 보여줄 에디트 컨트롤을 생성한다. 
	// (여러 줄 사용 모드, 가로/세로 자동 스크롤, 수직 스크롤바 사용, 엔터키 사용 모드)
	CreateEdit(5, 5, 590, 200, 1000, ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL);

	CreateButton("날씨 페이지 소스 가져오기", 5, 368, 200, 28, 1010);  // 버튼 생성!
	CreateButton("서울 날씨 보기", 210, 368, 200, 28, 1011);  // 버튼 생성!

	ShowDisplay(); // 정보를 윈도우에 출력한다.	
	return 0;
}