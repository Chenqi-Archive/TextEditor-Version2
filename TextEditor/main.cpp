#include "MainFrame.h"

#include "WndDesign/window/Global.h"


using namespace WndDesign;


int main() {
	global.AddWnd(new MainFrame);
	global.MessageLoop();
}