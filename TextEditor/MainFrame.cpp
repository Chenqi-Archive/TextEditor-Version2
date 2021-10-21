#include "MainFrame.h"
#include "TextView.h"

#include "WndDesign/frame/ClipFrame.h"
#include "WndDesign/frame/PaddingFrame.h"
#include "WndDesign/widget/ScrollBox.h"


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
		new ScrollBox{
			new PaddingFrame<Assigned, Auto>{
				Margin(20, 10, 20, 10),
				new ClipFrame<Assigned, Auto>{
					new TextView
				}
			}
		}
	) {
}