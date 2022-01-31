#include "MainFrame.h"
#include "ScrollView.h"


BEGIN_NAMESPACE(WndDesign)


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


MainFrame::MainFrame() :
	TitleBarFrame(
		MainFrameStyle(),
		new ScrollView()
	) {
}


END_NAMESPACE(WndDesign)