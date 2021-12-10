#pragma once

#include "WndDesign/window/wnd_traits.h"
#include "WndDesign/figure/text_block.h"
#include "WndDesign/common/unicode_helper.h"
#include "WndDesign/message/timer.h"
#include "WndDesign/message/ime.h"
#include "WndDesign/message/mouse_tracker.h"


BEGIN_NAMESPACE(WndDesign)


class TextView : public WndType<Assigned, Auto>, private ImeApi {
public:
	TextView();
	~TextView();

	// text
private:
	std::wstring text;
	TextBlock text_block;
	WordBreakIterator word_break_iterator;
private:
	uint GetCharacterLength(uint text_position) { return GetUTF16CharLength(text[text_position]); }
private:
	void SetText(std::wstring str) { text.assign(std::move(str)); TextUpdated(); }
	void InsertText(uint pos, wchar ch) { text.insert(pos, 1, ch); TextUpdated(); }
	void InsertText(uint pos, std::wstring str) { text.insert(pos, str); TextUpdated(); }
	void ReplaceText(uint begin, uint length, wchar ch) { text.replace(begin, length, 1, ch); TextUpdated(); }
	void ReplaceText(uint begin, uint length, std::wstring str) { text.replace(begin, length, str); TextUpdated(); }
	void DeleteText(uint begin, uint length) { text.erase(begin, length); TextUpdated(); }
private:
	using HitTestInfo = TextBlockHitTestInfo;

	// layout and paint
private:
	uint width_ref = 0;
protected:
	Size UpdateSize();
	void TextUpdated();
protected:
	virtual Size OnSizeRefUpdate(Size size_ref) override;
protected:
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override;

	// caret
private:
	static constexpr ushort caret_blink_period = 500;  // 500ms
	static constexpr ushort caret_blink_expire_time = 20000;  // 20s
	enum class CaretState : ushort { Hide, Show, BlinkShow, BlinkHide };
private:
	Timer caret_timer = Timer(std::bind(&TextView::BlinkCaret, this));
	CaretState caret_state = CaretState::Hide;
	ushort caret_blink_time = 0;
private:
	bool IsCaretVisible() const { return caret_state == CaretState::Show || caret_state == CaretState::BlinkShow; }
	void RedrawCaretRegion() { Redraw(caret_region); }
private:
	void HideCaret();
	void StartBlinkingCaret();
	void BlinkCaret();

	// caret position
private:
	static constexpr uint caret_width = 1;
	enum class CaretMoveDirection { Left, Right, Up, Down, Home, End };
private:
	uint caret_text_position = 0;
	Rect caret_region = region_empty;
private:
	void UpdateCaretRegion(const HitTestInfo& info);
private:
	void SetCaret(Point point);
	void SetCaret(uint text_position, bool is_trailing_hit);
	void MoveCaret(CaretMoveDirection direction);

	// selection
private:
	uint mouse_down_text_position = 0;
	uint selection_begin = 0;
	uint selection_end = 0;
	std::vector<HitTestInfo> selection_info;
	Rect selection_region_union;
private:
	void UpdateSelectionRegion();
	void RedrawSelectionRegion() { Redraw(selection_region_union); }
private:
	bool HasSelection() const { return selection_end > selection_begin; }
	void DoSelection(Point mouse_move_position);
	void SelectWord();
	void SelectParagraph();
	void SelectAll();
	void ClearSelection();

	// keyboard input
private:
	void Insert(wchar ch);
	void Insert(std::wstring str);
	void Delete(bool is_backspace);

	// ime input
private:
	uint ime_composition_begin = 0;
	uint ime_composition_end = 0;
private:
	virtual void OnImeCompositionBegin() override;
	virtual void OnImeComposition(std::wstring str) override;
	virtual void OnImeCompositionEnd() override {}

	// clipboard
private:
	void Cut();
	void Copy();
	void Paste();

	// message
private:
	MouseTracker mouse_tracker;
	bool is_ctrl_down = false;
	bool is_shift_down = false;
protected:
	virtual void OnMouseMsg(MouseMsg msg) override;
	virtual void OnKeyMsg(KeyMsg msg) override;
	virtual void OnNotifyMsg(NotifyMsg msg) override;
};


END_NAMESPACE(WndDesign)