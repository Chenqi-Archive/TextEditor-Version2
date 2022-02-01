#include "ListView.h"
#include "TextView.h"
#include "ScrollView.h"

#include "WndDesign/system/clipboard.h"

#include "BlockStore/block_manager.h"
#include "BlockStore/file_manager.h"


BEGIN_NAMESPACE(WndDesign)


struct ListViewStyle {
	struct ListLayoutStyle {
	public:
		float _gap = 0.0f;
	public:
		ListLayoutStyle& gap(float gap) { _gap = gap; return *this; }
	}list_layout;

	struct EditStyle {
	public:
		Color _selection_color = Color(Color::DimGray, 0x7f);
		Color _caret_color = Color::DimGray;
	public:
		constexpr EditStyle& selection_color(Color selection_color) { _selection_color = selection_color; return *this; }
		constexpr EditStyle& caret_color(Color caret_color) { _caret_color = caret_color; return *this; }
	}edit;

	ListViewStyle() {
		list_layout.gap(5px);
	}
}list_view_style;


ListView::ListView(ScrollView& scroll_view) : ListLayout(list_view_style.list_layout._gap), manager(std::make_unique<FileManager>(file)), scroll_view(scroll_view) {
	try {
		manager.LoadRootRef(block);
		auto list = block.Read();
		if (list->empty()) {
			Initialize();
		} else {
			for (auto item : *list) {
				AppendChild(new TextView(item));
			}
		}
	} catch (std::runtime_error&) {
		block = manager;
		Initialize();
	}
}

ListView::~ListView() {
	auto data = block.Write();
	data->clear();
	for (auto& item : child_list) {
		data->push_back(static_cast<TextView&>(*item.child).Save());
	}
	child_list.clear();
	manager.Format(); manager.SaveRootRef(block);
}

TextView& ListView::Get(size_t index) { return static_cast<TextView&>(*child_list[index].child); }

void ListView::Initialize() { AppendChild(new TextView(BlockRef<TextData>(manager))); }

void ListView::InsertAfter(TextView& child) {
	size_t index = GetChildData(child) + 1;
	InsertChild(index, new TextView(BlockRef<TextData>(manager)));
	TextView& text_view = Get(index); size_t pos = child.GetCaretPosition();
	text_view.SetText(child.GetText().substr(pos)); child.DeleteText(pos, (size_t)-1);
	text_view.SetCaret(0);
}

void ListView::Delete(TextView& child) {
	size_t index = GetChildData(child); if (index == 0) { return; }
	TextView& prev = Get(--index); size_t pos = prev.GetText().length();
	child.InsertText(0, prev.GetText()); EraseChild(index, 1);
	child.SetCaret(pos);
}

void ListView::DeleteAfter(TextView& child) {
	size_t index = GetChildData(child); if (index == child_list.size() - 1) { return; }
	TextView& next = Get(index + 1);
	child.AppendText(next.GetText());
	EraseChild(index + 1, 1);
}

void ListView::InsertText(TextView& child, std::wstring text) {
	size_t index = GetChildData(child);
	size_t begin = 0, end = 0, pos = 0;
	bool line_break = false;
	for (; pos < text.size(); pos++) {
		auto commit_current = [&]() {
			Get(index).Insert(text.substr(begin, end - begin));
			InsertAfter(Get(index++));
		};
		if (text[pos] == L'\r') {
			commit_current(); line_break = true;
		} else if (text[pos] == L'\n') {
			if (line_break == false) { commit_current(); }
			begin = end = pos + 1; line_break = false;
		} else {
			if (line_break == true) { line_break = false; begin = pos; end = pos; }
			end++;
		}
	}
	Get(index).Insert(text.substr(begin, end - begin));
}

void ListView::OnSizeRefUpdate(Size size_ref) {
	ListLayout::OnSizeRefUpdate(size_ref);
	if (HasSelection()) { UpdateSelectionRegion(); }
}

void ListView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
	ListLayout::OnDraw(figure_queue, draw_region);
	if (IsCaretVisible() && !caret_region.Intersect(draw_region).IsEmpty()) {
		figure_queue.add(caret_region.point, new Rectangle(caret_region.size, list_view_style.edit._caret_color));
	}
	if (selection_range_end > selection_range_begin) {
		figure_queue.add(selection_region.point, new Rectangle(selection_region.size, list_view_style.edit._selection_color));
	}
}

void ListView::UpdateCaretRegion(TextView& child, Rect caret_region) {
	caret_region += GetChildRegion(child).point - point_zero;
	if (this->caret_region == caret_region) { return; }
	RedrawCaretRegion();
	this->caret_region = caret_region;
	scroll_view.ScrollIntoView(Rect(caret_region.point * GetTransform(Get(0)), caret_region.size));
	RedrawCaretRegion();
}

void ListView::ShowCaret() { caret_state = CaretState::Show; }

void ListView::HideCaret() {
	if (caret_state != CaretState::Hide) {
		caret_state = CaretState::Hide;
		RedrawCaretRegion();
	}
}

void ListView::StartBlinkingCaret() {
	if (caret_state != CaretState::Hide) {
		if (!caret_timer.IsSet()) {
			caret_timer.Set(caret_blink_period);
		}
		caret_blink_time = 0;
	}
}

void ListView::BlinkCaret() {
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

void ListView::SetCaret(Point point) {
	caret_position = HitTestPoint(point);
	Get(caret_position).SetCaret(point);
	ClearSelection();
}

void ListView::SetCaretBefore(TextView& child) {
	size_t index = GetChildData(child); if (index == 0) { return; }
	Get(index - 1).SetCaret(-1);
}

void ListView::SetCaretAfter(TextView& child) {
	size_t index = GetChildData(child); if (index == child_list.size() - 1) { return; }
	Get(index + 1).SetCaret(0);
}

void ListView::SetCaretAt(TextView& child, Point point) {
	point += GetChildRegion(child).point - point_zero;
	Get(HitTestPoint(point)).SetCaret(point);
}

void ListView::UpdateSelectionRegion() {
	RedrawSelectionRegion();
	if (!HasSelection()) { selection_region = region_empty; return; }
	HideCaret(); SetFocus();
	selection_region.point.x = 0.0f;
	selection_region.point.y = child_list[selection_range_begin].BeginOffset();
	selection_region.size.width = size.width;
	selection_region.size.height = child_list[selection_range_end - 1].EndOffset() - selection_region.point.y;
	RedrawSelectionRegion();
}

void ListView::BeginSelection() {
	selection_begin = caret_position;
	Get(selection_begin).BeginSelection();
}

void ListView::DoSelection(Point point) {
	selection_range_begin = selection_begin;
	selection_range_end = HitTestPoint(point);
	if (selection_range_end == selection_begin) {
		ClearSelection();
		Get(selection_begin).DoSelection(point);
		return;
	}
	Get(selection_begin).ClearSelection();
	scroll_view.ScrollIntoView(Rect(point_zero * GetChildTransform(Get(selection_range_end)), Size(0.0f, child_list[selection_range_end].length)));
	if (selection_range_end < selection_range_begin) { std::swap(selection_range_begin, selection_range_end); }
	selection_range_end++;
	UpdateSelectionRegion();
}

void ListView::ClearSelection() {
	selection_range_begin = selection_range_end = 0;
	UpdateSelectionRegion();
}

void ListView::SelectWord() {
	Get(caret_position).SelectWord();
}

void ListView::SelectParagraph() {
	selection_range_begin = caret_position;
	selection_range_end = selection_range_begin + 1;
	UpdateSelectionRegion();
}

void ListView::SelectAll() {
	selection_range_begin = 0;
	selection_range_end = child_list.size();
	UpdateSelectionRegion();
}

void ListView::Insert(wchar ch) {
	Delete();
	Get(caret_position).Insert(ch);
}

void ListView::Delete() {
	if (!HasSelection()) { return; }
	size_t selection_begin = selection_range_begin, selection_end = selection_range_end;
	ClearSelection();
	EraseChild(selection_begin, selection_end - selection_begin);
	if (child_list.empty()) { Initialize(); }
	if (selection_begin >= child_list.size()) {
		caret_position = selection_begin - 1; Get(caret_position).SetCaret(-1);
	} else {
		caret_position = selection_begin; Get(caret_position).SetCaret(0);
	}
}

void ListView::Cut() {
	Copy();
	Delete();
}

void ListView::Copy() {
	if (!HasSelection()) { return; }
	std::wstring text;
	for (size_t pos = selection_range_begin; pos < selection_range_end; ++pos) {
		text.append(Get(pos).GetText()).append(1, L'\n');
	}
	SetClipboardData(text);
}

void ListView::Paste() {
	Delete();
	std::wstring str; GetClipboardData(str);
	if (!str.empty()) { InsertText(Get(caret_position), str); }
}

void ListView::OnMouseMsg(MouseMsg msg) {
	switch (msg.type) {
	case MouseMsg::LeftDown: SetCaret(msg.point); BeginSelection(); SetCapture(); break;
	case MouseMsg::LeftUp: ReleaseCapture(); break;
	case MouseMsg::WheelVertical: scroll_view.Scroll((float)-msg.wheel_delta); break;
	}
	switch (mouse_tracker.Track(msg)) {
	case MouseTrackMsg::LeftDrag: DoSelection(msg.point); break;
	case MouseTrackMsg::LeftDoubleClick: SelectWord(); break;
	case MouseTrackMsg::LeftTripleClick: SelectParagraph(); break;
	}
	StartBlinkingCaret();
}

void ListView::OnKeyMsg(KeyMsg msg) {
	switch (msg.type) {
	case KeyMsg::KeyDown:
		switch (msg.key) {
		case Key::Backspace: Delete(); break;
		case Key::Delete: Delete(); break;

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


END_NAMESPACE(WndDesign)