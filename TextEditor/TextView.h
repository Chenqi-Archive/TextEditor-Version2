#pragma once

#include "WndDesign/control/EditBox.h"
#include "WndDesign/figure/shape.h"


using namespace WndDesign;


class TextView : public EditBox {
private:
	struct Style : EditBox::Style {
		Style() {
			paragraph.line_height(25px);
			font.family(L"Calibri", L"DengXian").size(20);
		}
	};
public:
	TextView();
	~TextView();
private:
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override {
		figure_queue.add(draw_region.point, new Rectangle(draw_region.size, Color::White));
		EditBox::OnDraw(figure_queue, draw_region);
	}
};