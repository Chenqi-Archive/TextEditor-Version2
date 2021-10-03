#include "WndDesign/window/Global.h"
#include "WndDesign/widget/TitleBarFrame.h"
#include "WndDesign/widget/ScrollBox.h"
#include "WndDesign/control/EditBox.h"
#include "WndDesign/frame/ClipFrame.h"


using namespace WndDesign; 


struct MainFrameStyle : TitleBarFrame::Style {
	MainFrameStyle() {
		width.min(200px).normal(800px).max(100pct);
		height.min(200px).normal(500px).max(100pct);
		position.setHorizontalCenter().setVerticalCenter();
		border.width(4).radius(8).color(Color::DarkGray);
		title.assign(L"TextEditor-v2.0");
		title_format.font.size(16).color(Color::White);
	}
};


struct EditBoxStyle : EditBox::Style {
	EditBoxStyle() {
		paragraph.line_height(25px);
		font.family(L"Calibri", L"DengXian").size(20);
	}
};


int main() {
	global.AddWnd(
		new TitleBarFrame{
			MainFrameStyle(),
			new ScrollBox{
				new ClipFrame<Assigned, Auto>{
					new EditBox(EditBoxStyle())
				}
			}
		}
	);
	global.MessageLoop();
}