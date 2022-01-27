//verificam daca tasta este apasata sau nu
struct Button_state
{
	bool is_down;
	bool changed;
};

enum
{
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_W,
	BUTTON_S,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_ENTER,
	BUTTON_ESC,

	BUTTON_COUNT,  //ultimul buton inregistrat pentru cazul in care adaugam taste suplimentare in viitor
};

struct Input
{
	Button_state buttons[BUTTON_COUNT];
};
