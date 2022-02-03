#include "caret.h"
#include "TextView.h"

#include "WndDesign/message/timer.h"


BEGIN_NAMESPACE(WndDesign)


ref_ptr<TextView> caret_focus = nullptr;
Rect caret_region = region_empty;

constexpr ushort caret_blink_period = 500;  // 500ms
constexpr ushort caret_blink_expire_time = 20000;  // 20s
enum class CaretState : ushort { Hide, Show, BlinkShow, BlinkHide };

void BlinkCaret();

Timer caret_timer = Timer([&]() { BlinkCaret(); });
CaretState caret_state = CaretState::Hide;
ushort caret_blink_time = 0;

void RedrawCaretRegion() {
	caret_focus->Redraw(caret_region);
}

void BlinkCaret() {
	caret_blink_time += caret_blink_period;
	if (caret_blink_time >= caret_blink_expire_time) {
		caret_state = CaretState::Show;
		caret_timer.Stop();
		return;
	}
	switch (caret_state) {
	case CaretState::Hide: caret_timer.Stop(); return;
	case CaretState::Show:
	case CaretState::BlinkShow: caret_state = CaretState::BlinkHide; break;
	case CaretState::BlinkHide: caret_state = CaretState::BlinkShow; break;
	}
	RedrawCaretRegion();
}


bool Caret::IsActive() { return caret_state != CaretState::Hide; }

bool Caret::IsVisible() { return caret_state == CaretState::Show || caret_state == CaretState::BlinkShow; }

ref_ptr<TextView> Caret::FocusWnd() { return caret_focus; }

Rect Caret::Region() { return caret_region; }

void Caret::Show() { caret_state = CaretState::Show; }

void Caret::Hide() {
	if (caret_state != CaretState::Hide) {
		caret_state = CaretState::Hide;
		RedrawCaretRegion();
	}
}

void Caret::Blink() {
	if (caret_state != CaretState::Hide) {
		if (!caret_timer.IsSet()) {
			caret_timer.Set(caret_blink_period);
		}
		caret_blink_time = 0;
	}
}

void Caret::UpdateRegion(TextView& wnd, Rect region) {
	if (IsActive()) { RedrawCaretRegion(); }
	caret_focus = &wnd;
	caret_region = region;
	RedrawCaretRegion();
}


END_NAMESPACE(WndDesign)