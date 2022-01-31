#include "TextView.h"
#include "ScrollView.h"
#include "WndDesign/figure/shape.h"
#include "WndDesign/system/clipboard.h"


BEGIN_NAMESPACE(WndDesign)


struct TextViewStyle : TextBlockStyle {
	struct EditStyle {
	public:
		Color _selection_color = Color(Color::DimGray, 0x7f);
		Color _caret_color = Color::DimGray;
	public:
		constexpr EditStyle& selection_color(Color selection_color) { _selection_color = selection_color; return *this; }
		constexpr EditStyle& caret_color(Color caret_color) { _caret_color = caret_color; return *this; }
	}edit;

	TextViewStyle() {
		paragraph.line_height(25px);
		font.family(L"Calibri", L"Segoe UI Emoji", L"DengXian").size(20);
	}
}edit_view_style;


TextView::TextView(BlockRef<TextData> block, ScrollView& scroll_view) :
	ImeApi(this), block(std::move(block)), scroll_view(scroll_view), text(Load()), text_block(edit_view_style, text) {
	cursor = Cursor::Text;
	word_break_iterator.SetText(text);
}

void TextView::TextUpdated() {
	text_block.SetText(edit_view_style, text);
	word_break_iterator.SetText(text);
	UpdateSize(); redraw_region = region_infinite;
	SizeUpdated(); Redraw();
}

void TextView::OnSizeRefUpdate(Size size_ref) {
	width_ref = size_ref.width; UpdateSize();
	if (IsCaretVisible()) { UpdateCaretRegion(text_block.HitTestTextPosition(caret_text_position)); }
	if (HasSelection()) { UpdateSelectionRegion(); }
}

void TextView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
	figure_queue.add(point_zero, new TextBlockFigure(text_block, edit_view_style.font._color));
	if (IsCaretVisible() && !caret_region.Intersect(draw_region).IsEmpty()) {
		figure_queue.add(caret_region.point, new Rectangle(caret_region.size, edit_view_style.edit._caret_color));
	}
	if (HasSelection()) {
		for (auto& it : selection_info) {
			auto& region = it.geometry_region; if (region.Intersect(draw_region).IsEmpty()) { continue; }
			figure_queue.add(region.point, new Rectangle(region.size, edit_view_style.edit._selection_color));
		}
	}
}

void TextView::HideCaret() {
	if (caret_state != CaretState::Hide) {
		caret_state = CaretState::Hide;
		RedrawCaretRegion();
	}
}

void TextView::StartBlinkingCaret() {
	if (caret_state != CaretState::Hide) {
		if (!caret_timer.IsSet()) {
			caret_timer.Set(caret_blink_period);
		}
		caret_blink_time = 0;
	}
}

void TextView::BlinkCaret() {
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

void TextView::UpdateCaretRegion(const HitTestInfo& info) {
	RedrawCaretRegion();
	caret_text_position = info.text_position;
	caret_region.point = info.geometry_region.point;
	caret_region.size = Size(caret_width, info.geometry_region.size.height);
	if (info.is_trailing_hit) {
		caret_text_position += info.text_length;
		caret_region.point.x += info.geometry_region.size.width;
	}
	scroll_view.ScrollIntoView(Rect(GetListView().ConvertPoint(*this, caret_region.point), caret_region.size));
	RedrawCaretRegion();
}

void TextView::SetCaret(HitTestInfo info) {
	SetFocus();
	UpdateCaretRegion(info); caret_state = CaretState::Show;
	ClearSelection();
}

void TextView::MoveCaret(CaretMoveDirection direction) {
	switch (direction) {
	case CaretMoveDirection::Left:
		if (HasSelection()) {
			SetCaret(selection_begin);
		} else {
			if (caret_text_position > 0) {
				SetCaret(caret_text_position - 1);
			} else {
				GetListView().SetCaretBefore(*this);
			}
		}
		break;
	case CaretMoveDirection::Right:
		if (HasSelection()) {
			SetCaret(selection_end);
		} else {
			if (caret_text_position < text.length()) {
				SetCaret(caret_text_position + 1);
			} else {
				GetListView().SetCaretAfter(*this);
			}
		}
		break;
	case CaretMoveDirection::Up:
		GetListView().SetCaretAt(*this, caret_region.Center() - Vector(0, caret_region.size.height));
		break;
	case CaretMoveDirection::Down:
		GetListView().SetCaretAt(*this, caret_region.Center() + Vector(0, caret_region.size.height));
		break;
	case CaretMoveDirection::Home:
		SetCaret(0);
		break;
	case CaretMoveDirection::End:
		SetCaret((size_t)-1);
		break;
	}
}

void TextView::UpdateSelectionRegion() {
	selection_info = text_block.HitTestTextRange(selection_begin, selection_end - selection_begin);
	RedrawSelectionRegion();
	selection_region_union = region_empty;
	for (auto& it : selection_info) {
		selection_region_union = selection_region_union.Union(it.geometry_region);
	}
	RedrawSelectionRegion();
}

void TextView::DoSelection(Point mouse_move_position) {
	HitTestInfo info = text_block.HitTestPoint(mouse_move_position);
	UpdateCaretRegion(info);
	selection_begin = mouse_down_text_position;
	selection_end = caret_text_position;
	if (selection_begin == selection_end) { ClearSelection(); return; }
	if (selection_end < selection_begin) { std::swap(selection_begin, selection_end); }
	UpdateSelectionRegion(); HideCaret();
}

void TextView::SelectWord() {
	if (caret_text_position >= text.length()) { return; }
	TextRange word_range = word_break_iterator.Seek(caret_text_position);
	selection_begin = word_range.left(); selection_end = word_range.right();
	UpdateSelectionRegion(); HideCaret();
}

void TextView::SelectAll() {
	selection_begin = 0;
	selection_end = text.length();
	UpdateSelectionRegion(); HideCaret();
}

void TextView::ClearSelection() {
	selection_begin = selection_end = 0;
	if (!selection_region_union.IsEmpty()) {
		selection_info.clear();
		RedrawSelectionRegion();
		selection_region_union = region_empty;
	}
}

void TextView::Insert(wchar ch) {
	if (HasSelection()) {
		ReplaceText(selection_begin, selection_end - selection_begin, ch);
		SetCaret(selection_begin + 1);
	} else {
		InsertText(caret_text_position, ch);
		SetCaret(caret_text_position + 1);
	}
}

void TextView::Insert(std::wstring str) {
	if (HasSelection()) {
		ReplaceText(selection_begin, selection_end - selection_begin, str);
		SetCaret(selection_begin + str.length());
	} else {
		InsertText(caret_text_position, str);
		SetCaret(caret_text_position + str.length());
	}
}

void TextView::Delete(bool is_backspace) {
	if (HasSelection()) {
		DeleteText(selection_begin, selection_end - selection_begin);
		SetCaret(selection_begin);
	} else {
		if (is_backspace) {
			if (caret_text_position == 0) { return GetListView().Delete(*this); }
			size_t previous_caret_position = caret_text_position;
			SetCaret(caret_text_position - 1);
			DeleteText(caret_text_position, previous_caret_position - caret_text_position);
		} else {
			if (caret_text_position >= text.length()) { return GetListView().DeleteAfter(*this); }
			DeleteText(caret_text_position, GetCharacterLength(caret_text_position));
		}
	}
}

void TextView::OnImeCompositionBegin() {
	Point ime_position;
	if (HasSelection()) {
		ime_composition_begin = selection_begin;
		ime_composition_end = selection_end;
		ime_position = selection_info.front().geometry_region.RightBottom();
	} else {
		ime_composition_begin = caret_text_position;
		ime_composition_end = ime_composition_begin;
		ime_position = caret_region.RightBottom();
	}
	ImeSetPosition(ime_position * GetChildTransform(*this));
}

void TextView::OnImeComposition(std::wstring str) {
	ReplaceText(ime_composition_begin, ime_composition_end - ime_composition_begin, str);
	ime_composition_end = ime_composition_begin + str.length();
	SetCaret(ime_composition_end);
}

void TextView::Cut() {
	if (HasSelection()) {
		Copy();
		Delete(false);
	}
}

void TextView::Copy() {
	if (HasSelection()) {
		SetClipboardData(text.substr(selection_begin, selection_end - selection_begin));
	}
}

void TextView::Paste() {
	std::wstring str; GetClipboardData(str);
	if (!str.empty()) { Insert(str); }
}

void TextView::OnMouseMsg(MouseMsg msg) {
	switch (msg.type) {
	case MouseMsg::LeftDown: SetCaret(msg.point); mouse_down_text_position = caret_text_position; SetCapture(); break;
	case MouseMsg::LeftUp: ReleaseCapture(); break;
	case MouseMsg::WheelVertical: scroll_view.Scroll((float)-msg.wheel_delta); break;
	}
	switch (mouse_tracker.Track(msg)) {
	case MouseTrackMsg::LeftDoubleClick: SelectWord(); break;
	case MouseTrackMsg::LeftTripleClick: SelectAll(); break;
	case MouseTrackMsg::LeftDrag: DoSelection(msg.point); break;
	}
	StartBlinkingCaret();
}

void TextView::OnKeyMsg(KeyMsg msg) {
	switch (msg.type) {
	case KeyMsg::KeyDown:
		switch (msg.key) {
		case Key::Left: MoveCaret(CaretMoveDirection::Left); break;
		case Key::Right: MoveCaret(CaretMoveDirection::Right); break;
		case Key::Up: MoveCaret(CaretMoveDirection::Up); break;
		case Key::Down: MoveCaret(CaretMoveDirection::Down); break;
		case Key::Home: MoveCaret(CaretMoveDirection::Home); break;
		case Key::End: MoveCaret(CaretMoveDirection::End); break;

		case Key::Enter: GetListView().InsertAfter(*this, caret_text_position); break;
		case Key::Tab: Insert(L'\t'); break;

		case Key::Backspace: Delete(true); break;
		case Key::Delete: Delete(false); break;

		case Key::Ctrl: is_ctrl_down = true; break;
		case Key::Shift: is_shift_down = true; break;

		case CharKey('A'): if (is_ctrl_down) { SelectAll(); } break;
		case CharKey('X'): if (is_ctrl_down) { Cut(); } break;
		case CharKey('C'): if (is_ctrl_down) { Copy(); } break;
		case CharKey('V'): if (is_ctrl_down) { Paste(); } break;
		}
		break;
	case KeyMsg::KeyUp:
		switch (msg.key) {
		case Key::Ctrl: is_ctrl_down = false; break;
		case Key::Shift: is_shift_down = false; break;
		}
		break;
	case KeyMsg::Char:
		if (is_ctrl_down) { break; }
		if (!iswcntrl(msg.ch)) { Insert(msg.ch); };
		break;
	}
	StartBlinkingCaret();
}

void TextView::OnNotifyMsg(NotifyMsg msg) {
	switch (msg) {
	case NotifyMsg::LoseFocus: ClearSelection(); HideCaret(); break;
	}
}


END_NAMESPACE(WndDesign)