#include <windows.h>
#include <stdlib.h>
#include "utility.cpp"

global_variable bool running = true;

struct Render_State {
	int width, height;
	void* memory;
	BITMAPINFO bitmap_info;
};

global_variable Render_State render_state;
#include "renderer.cpp"
#include "platform_common.cpp"
#include "game.cpp"

LRESULT CALLBACK window_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (uMsg)
	{
		case WM_CLOSE:
		//Daca user-ul inchide jocul running devine false astfel intrerupand while-ul care repeta programul
		case WM_DESTROY: 
		{
			running = false;
		} break;
		case WM_SIZE: 
		{
			//functia de mai jos afla daca user-ul a modificat rezolutia jocului
			RECT rect;
			GetClientRect(hwnd, &rect);
			render_state.width = rect.right - rect.left;
			render_state.height = rect.bottom - rect.top;

			//buffer-ul are marimea de width ori height ori marime pixel(32 biti - 8 rosu, 8 albastru, 8 verde)
			int size = render_state.width * render_state.height * sizeof(unsigned int);

			//Verificam daca exista ceva in buffer si il stergem in caz afirmativ

			if (render_state.memory)
				VirtualFree(render_state.memory, 0, MEM_RELEASE);

			//Accesam memoria Heap pentru a afisa cadre cu functia VirtuAlalloc
			render_state.memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			
			//Oferim "hartii" care va sti cum sa citeasca datele culorilor parametri necesari
			render_state.bitmap_info.bmiHeader.biSize = sizeof(render_state.bitmap_info.bmiHeader);
			render_state.bitmap_info.bmiHeader.biWidth = render_state.width;
			render_state.bitmap_info.bmiHeader.biHeight = render_state.height;
			render_state.bitmap_info.bmiHeader.biPlanes = 1; //Conform google, habar n-am de ce
			render_state.bitmap_info.bmiHeader.biBitCount = 32;
			render_state.bitmap_info.bmiHeader.biCompression = BI_RGB;

		}break;
		default: 
		{
			result = DefWindowProc(hwnd, uMsg, wParam, lParam); 
		}
	}
	return result;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	ShowCursor(FALSE);

	//Creaza clasa pentru fereastra
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpszClassName = "Game";
	window_class.lpfnWndProc = window_callback;

	//Inregistreaza clasa 
	RegisterClass(&window_class);

	//Creaza fereastra efectiv
	HWND window = CreateWindow(
		window_class.lpszClassName,
		"FONG (Fake Pong)",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1270, 720,
		0,
		0,
		hInstance,
		0
	); 
	{
		//Fullscreen
		SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi);
		SetWindowPos(window, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}

	//Obtine componentele grafice
	HDC hdc = GetDC(window);

	Input input = {};

	float delta_time = 0.016666f; //60 fps
	LARGE_INTEGER frame_begin_time;
	QueryPerformanceCounter(&frame_begin_time);

	float performance_frequency;
	{
		LARGE_INTEGER perf;
		QueryPerformanceFrequency(&perf);
		performance_frequency = (float)perf.QuadPart;
	}

	while (running)
	{
		//Aici programul primeste mesaje din mediul exterior, PeekMessage le citeste, PM_REMOVE le sterge dupa o citire, iar DispatchMessage trimite mesaj inapoi la functia callback
		MSG message;

		for (int i = 0; i < BUTTON_COUNT; i++)
			input.buttons[i].changed = true;

		while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
		{
			//luam inputul jucatorului (verificam ce tasta apasa)
			switch (message.message)
			{
				case WM_KEYUP:
				case WM_KEYDOWN:
				{
					u32 vk_code = (u32)message.wParam;
					bool is_down = ((message.lParam & (1 << 31)) == 0);

#define process_button(b, vk)\
case vk: {\
input.buttons[b].changed = is_down != input.buttons[b].is_down;\
input.buttons[b].is_down = is_down;\
} break;

					switch (vk_code)
					{
						process_button(BUTTON_UP, VK_UP);
						process_button(BUTTON_DOWN, VK_DOWN);
						process_button(BUTTON_W, 'W');
						process_button(BUTTON_S, 'S');
						process_button(BUTTON_LEFT, VK_LEFT);
						process_button(BUTTON_RIGHT, VK_RIGHT);
						process_button(BUTTON_ENTER, VK_RETURN);
						process_button(BUTTON_ESC, VK_ESCAPE);
					}
				} break;
				default: 
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}
			
		}

		//Apelam functia care controleaza aplicatia (ex. input, afisare, etc)
		simulate_game(&input, delta_time);

		//Copiaza datele culorilor de la destinatie si le trimite la sursa unde sunt marite sau micsorate pentru a se potrivi cu destinatia
		StretchDIBits(hdc, 0, 0, render_state.width, render_state.height, 0, 0, render_state.width, render_state.height, render_state.memory, &render_state.bitmap_info, DIB_RGB_COLORS, SRCCOPY);

		LARGE_INTEGER frame_end_time;
		QueryPerformanceCounter(&frame_end_time);
		delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency;
		frame_begin_time = frame_end_time;
	}
}
	
