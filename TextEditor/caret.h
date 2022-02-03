#pragma once

#include "WndDesign/window/WndObject.h"


BEGIN_NAMESPACE(WndDesign)

class TextView;


struct Caret {
	static constexpr float width = 1.0f;

	bool IsActive();
	bool IsVisible();
	ref_ptr<TextView> FocusWnd();
	Rect Region();

	void Show();
	void Hide();
	void Blink();
	void UpdateRegion(TextView& wnd, Rect region);
};

inline static Caret caret;


END_NAMESPACE(WndDesign)