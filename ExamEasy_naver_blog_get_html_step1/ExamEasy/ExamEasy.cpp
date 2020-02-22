#include "pch.h"
#include "tipsware.h"
#include<string.h>
#include <atlstr.h>

// HTTP 프로토콜을 사용하는 함수들이 정의된 wininet.h 헤더 파일을 추가한다.
#include <wininet.h>  
// wininet.h 헤더에 정의된 함수들을 사용하려면 wininet.lib 라이브러리를 추가해야 한다.
#pragma comment (lib, "wininet.lib")

#define IDC_STR_EDIT         1002    // 에디트 컨트롤이 사용할 아이디
#define IDC_OPEN_FILE_BTN    1003    // [파일 열기] 버튼 아이디
#define IDC_SAVE_FILE_BTN    1004    // [파일 저장] 버튼 아이디




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
			else WaitForSingleObject(h_wait_event, 5); // 50ms 지연하도록 구성한다.
		} else error_count = 0;
	}

	*(ap_html_string + total_bytes) = 0;  // 문자열의 끝에 NULL 문자를 추가함!
	CloseHandle(h_wait_event);  // 지연 효과를 위해 생성한 이벤트 객체를 제거한다.
	return total_bytes;  // 읽어들인 전체 바이트 수를 반환한다.
}

// 분석에 의미없는 문자들을 제거하는 함수!
void RemoveMeaninglessChar(char* ap_string)
{
	// 탭, 줄바꿈 문자가 나올때까지 반복한다.
	while (*ap_string) {
		if (*ap_string == '\t' || *ap_string == '\r' || *ap_string == '\n') break;
		ap_string++;
	}
	if (!*ap_string) return;  // 전체적으로 탭, 줄바꿈 문자가 없는 경우! 

	char* p_dest = ap_string++;
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
	char url[128],front_url[128];
	memcpy_s(front_url, 128, "search.naver?sm=top_hty&fbm=1&ie=utf8&query=", 128);
	// 1001번 에디트 컨트롤에 입력된 웹 페이지 주소를 url 배열에 복사한다.
	GetCtrlName(FindControl(1001), url, 128);
	strcat_s(front_url, url);

	// 인터넷을 사용할 세션을 구성한다.
	HINTERNET h_session = InternetOpen("MovieScanner", PRE_CONFIG_INTERNET_ACCESS, NULL, INTERNET_INVALID_PORT_NUMBER, 0);
	// 어떤 사이트에 접속할지 구성한다. (네이버 검색을 사용하도록 구성한다.)
	HINTERNET h_connect = InternetConnect(h_session, "search.naver.com", INTERNET_INVALID_PORT_NUMBER, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	// 네이버 영화에서 원하는 웹 페이지에 접속한다. (url 배열에 저장된 페이지를 연다.)
	HINTERNET h_http_file = HttpOpenRequest(h_connect, "GET", front_url, HTTP_VERSION, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
	

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

// ap_src_str 문자열에서 [ap_start_word]에서 시작해서 [ap_end_word]이 나올때까지의 문자열을 
// 복사하는 함수입니다. 공백 문자는 복사하지 않고 <  > 로 포함된 문자열도 제외시킨다.
char* CopyTextFromWebData(char* ap_dest_str, char* ap_src_str, const char* ap_start_word, const char* ap_end_word)
{
	char* p_pos = strstr(ap_src_str, ap_start_word);  // 시작 위치를 찾는다.
	if (p_pos != NULL) {
		char* p_limit_pos = strstr(p_pos, ap_end_word);  // 끝 위치를 찾는다.
		if (p_limit_pos != NULL) {
			p_pos += strlen(ap_start_word);  // 시작 단어에 해당하는 문자열은 제외 시킨다.
			char open_flag = 0;  // < > 괄호가 열려있는지 기억하는 변수. 0이면 닫힘, 1이면 열림.
			while (p_pos < p_limit_pos) { // 끝 위치까지 탐색한다.
				if (*p_pos == '<') open_flag = 1;  // 괄호가 열림
				else if (*p_pos == '>') open_flag = 0;  // 괄호가 닫힘
				else {
					if (!open_flag) { // 괄호가 닫힌 상태
						if (*p_pos == '|') {  // '|' 문자는 ", "로 변경해서 추가한다.
							*ap_dest_str++ = ',';  *ap_dest_str++ = ' ';
						}
						else {  // 공백은 연속으로 추가되지 않게 한다.
							if (*(ap_dest_str - 1) != ' ' || *p_pos != ' ') *ap_dest_str++ = *p_pos;
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



void ShowMovieData()  // HTML 정보를 사용해서 영화 정보 구성하기!
{
	void* p_edit = FindControl(1000);  // 에디트 컨트롤의 주소를 얻는다.
	int len = Edit_GetLength(p_edit) + 1;  // 에디트 컨트롤에 저장된 문자열의 길이를 얻는다.
	char title[16], grade[16], index[32], director[16];
	char* p_html_str = (char*)malloc(len); // 에디트 컨트롤에 저장된 문자열의 길이만큼 메모리 할당!
	if (p_html_str != NULL) {
		GetCtrlName(p_edit, p_html_str, len);  // 에디트 컨트롤에 저장된 문자열을 p_html_str에 복사한다.

		char* p_pos = strstr(p_html_str, "네이버가 운영하는 영화 서비스입니다.");  // 시작 위치를 찾는다!
		if (p_pos != NULL) {
			p_pos += 39;  

			p_pos = CopyTextFromWebData(title, p_pos, "<strong>", "</strong>");  // 영화 제목 복사!
			p_pos = CopyTextFromWebData(grade, p_pos, "<em>", "</em>");  // 평점 복사!

			p_pos = CopyTextFromWebData(index, p_pos, "<span >", "</span>");   // 개요 복사!
			p_pos = CopyTextFromWebData(director, p_pos, "<dt class=\"director\">", "</a>"); // 감독 복사!

			SelectFontObject("굴림", 16, 1);
			TextOut(10, 40, RGB(242, 242, 242), "%s %s", title, grade);  // 제목, 평점 출력!
			TextOut(10, 60, RGB(255, 255, 200), index); // 개요 출력!


			SelectFontObject("굴림", 12, 1);
			TextOut(10, 80, RGB(232, 232, 232), director);  // 감독 출력!
		}

		free(p_html_str);  // 에디트 컨트롤에 저장된 문자열을 복사하기 위해 할당했던 메모리 해제!
		ShowDisplay(); // 정보를 윈도우에 출력한다.	
	}
}


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


void AsciiToUTF8(CString parm_ascii_string, CString& parm_utf8_string)
{
	parm_utf8_string.Empty();

	// 아스키 코드를 UTF8형식의 코드로 변환해야 한다. 아스키 코드를 UTF8 코드로 변환할때는
	// 아스키 코드를 유니코드로 먼저 변환하고 변환된 유니코드를 UTF8 코드로 변환해야 한다.

	// 아스키 코드로된 문자열을 유니코드화 시켰을 때의 길이를 구한다.
	int temp_length = MultiByteToWideChar(CP_ACP, 0, (char*)(const char*)parm_ascii_string, -1, NULL, 0);
	// 변환된 유니코드를 저장할 공간을 할당한다.
	BSTR unicode_str = SysAllocStringLen(NULL, temp_length + 1);

	// 아스키 코드로된 문자열을 유니 코드 형식의 문자열로 변경한다.
	MultiByteToWideChar(CP_ACP, 0, (char*)(const char*)parm_ascii_string, -1, unicode_str, temp_length);

	// 유니코드 형식의 문자열을 UTF8 형식으로 변경했을때 필요한 메모리 공간의 크기를 얻는다.
	temp_length = WideCharToMultiByte(CP_UTF8, 0, unicode_str, -1, NULL, 0, NULL, NULL);

	if (temp_length > 0) {
		CString str;
		// UTF8 코드를 저장할 메모리 공간을 할당한다.
		char* p_utf8_string = new char[temp_length];
		memset(p_utf8_string, 0, temp_length);
		// 유니코드를 UTF8코드로 변환한다.
		WideCharToMultiByte(CP_UTF8, 0, unicode_str, -1, p_utf8_string, temp_length, NULL, NULL);

		// UTF8 형식으로 변경된 문자열을 각 문자의 코드값별로 웹 URL에 사용되는 형식으로 변환한다.
		for (int i = 0; i < temp_length - 1; i++) {
			if (p_utf8_string[i] & 0x80) {
				// 현재 코드가 한글인 경우..
				// UTF8 코드로 표현된 한글은 3바이트로 표시된다. "한글"  ->  %ED%95%9C%EA%B8%80
				for (int sub_i = 0; sub_i < 3; sub_i++) {
					str.Format("%%%X", p_utf8_string[i] & 0x00FF);
					parm_utf8_string += str;
					i++;
				}

				i--;
			}
			else {
				// 현재 코드가 영문인 경우, 변경없이 그대로 사용한다.
				parm_utf8_string += p_utf8_string[i];
			}
		}

		delete[] p_utf8_string;
	}

	// 유니코드 형식의 문자열을 저장하기 위해 생성했던 메모리를 삭제한다.
	SysFreeString(unicode_str);
}


// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void *ap_ctrl)
{
	// 1010번 버튼("검색" 버튼)이 눌러진 경우
	if (a_ctrl_id == 1010)
	{
		LoadDataFromWebPage();
		ShowMovieData();
	}
	// 컨트롤을 조작했을 때 호출할 함수 만들기
	// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
	else if (a_ctrl_id == IDC_OPEN_FILE_BTN) OpenTextFile();   // [파일 열기] 버튼을 선택한 경우
	else if (a_ctrl_id == IDC_SAVE_FILE_BTN) SaveTextFile();   // [파일 저장] 버튼을 선택한 경우
}


// 컨트롤을 조작했을 때 호출할 함수 등록
CMD_MESSAGE(OnCommand)

int main()
{
	ChangeWorkSize(800, 600); // 작업 영역을 설정한다.
	Clear(0, RGB(82, 97, 124)); // 작업 영역을 RGB(82, 97, 124) 색으로 채운다!

	// 웹 페이지 소스를 보여줄 에디트 컨트롤을 생성한다. 
	// (여러 줄 사용 모드, 가로/세로 자동 스크롤, 수직 스크롤바 사용, 엔터키 사용 모드)
	CreateEdit(-30, -30, 10, 10, 1000, ES_AUTOHSCROLL );



	// 웹 페이지 주소를 입력할 에디트 컨트롤을 생성한다.
	void *p = CreateEdit(5, 10, 280, 24, 1001, 0);
	// 소스를 가져올 샘플 웹페이지 URL을 에디트 컨트롤에 표시합니다!

	SetCtrlName(p, "기생충");
	
	CreateButton("검색", 290, 10, 40, 24, 1010);  // 버튼 생성!

	 // 에디트 컨트롤을 생성한다. (여러 줄 사용 모드, 가로/세로 자동 스크롤, 수직 스크롤바 사용, 엔터키 사용 모드)
	CreateEdit(350, 30, 440, 450, IDC_STR_EDIT,
		ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL);

	// 파일 열기, 저장 버튼을 생성한다.
	CreateButton("파일 열기", 350, 490, 70, 28, IDC_OPEN_FILE_BTN);
	CreateButton("파일 저장", 423, 490, 70, 28, IDC_SAVE_FILE_BTN);


	ShowDisplay(); // 정보를 윈도우에 출력한다.	
	return 0;
}