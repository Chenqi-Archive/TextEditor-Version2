#pragma once

#include "WndDesign/widget/ScrollBox.h"


BEGIN_NAMESPACE(WndDesign)


class ScrollView : public ScrollBox<Vertical> {
public:
	ScrollView();

public:
	void Scroll(float wheel_delta) {
		GetScrollFrame().Scroll(wheel_delta);
	}
	void ScrollIntoView(Rect region) {
		GetScrollFrame().ScrollIntoView(region.point.y, region.size.height);
	}
};


END_NAMESPACE(WndDesign)