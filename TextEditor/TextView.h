#pragma once

#include "WndDesign/window/wnd_traits.h"
#include "WndDesign/figure/text_block.h"
#include "WndDesign/common/unicode_helper.h"
#include "WndDesign/message/ime.h"

#include "data_format.h"

#include "BlockStore/block_manager.h"


using namespace BlockStore;


BEGIN_NAMESPACE(WndDesign)

class ScrollView;
class ListView;


class TextView : public WndType<Assigned, Auto>, private ImeApi {
public:
	TextView(BlockRef<TextData> block);

	// parent
private:
	ListView& GetListView();

	// data
private:
	BlockRef<TextData> block;
public:
	std::wstring Load() { return *block.Read(); }
	BlockRef<TextData> Save() { *block.Write() = text; return block; }

	// text
private:
	using HitTestInfo = TextBlockHitTestInfo;
private:
	std::wstring text;
	TextBlock text_block;
	WordBreakIterator word_break_iterator;
public:
	const std::wstring& GetText() const { return text; }
public:
	void SetText(std::wstring str) { text.assign(std::move(str)); TextUpdated(); }
	void AppendText(std::wstring str) { text.append(str); TextUpdated(); }
	void InsertText(size_t pos, wchar ch) { text.insert(pos, 1, ch); TextUpdated(); }
	void InsertText(size_t pos, std::wstring str) { text.insert(pos, str); TextUpdated(); }
	void ReplaceText(size_t begin, size_t length, wchar ch) { text.replace(begin, length, 1, ch); TextUpdated(); }
	void ReplaceText(size_t begin, size_t length, std::wstring str) { text.replace(begin, length, str); TextUpdated(); }
	void DeleteText(size_t begin, size_t length) { text.erase(begin, length); TextUpdated(); }
private:
	size_t GetCharacterLength(size_t text_position) { return GetUTF16CharLength(text[text_position]); }

	// layout
private:
	float width_ref = 0.0f;
protected:
	void UpdateSize() { text_block.UpdateSizeRef(Size(width_ref, length_max)); }
	void TextUpdated();
protected:
	virtual void OnSizeRefUpdate(Size size_ref) override;
	virtual Size GetSize() override { return Size(width_ref, text_block.GetSize().height); }

	// paint
protected:
	Rect redraw_region = region_empty;
protected:
	virtual Rect GetRedrawRegion() { return redraw_region; }
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override;

	// caret
private:
	static constexpr float caret_width = 1.0f;
	enum class CaretMoveDirection { Left, Right, Up, Down, Home, End };
private:
	size_t caret_position = 0;
	Rect caret_region = region_empty;
private:
	bool HasCaret() const { return !caret_region.IsEmpty(); }
	void ClearCaret() { caret_region = region_empty; }
	void UpdateCaretRegion(const HitTestInfo& info);
public:
	size_t GetCaretPosition() const { return caret_position; }
	void SetCaret(HitTestInfo info);
	void SetCaret(Point point) { SetCaret(text_block.HitTestPoint(point)); }
	void SetCaret(size_t text_position) { SetCaret(text_block.HitTestTextPosition(text_position)); }
	void MoveCaret(CaretMoveDirection direction);

	// selection
private:
	size_t selection_begin = 0;
	size_t selection_range_begin = 0;
	size_t selection_range_end = 0;
	std::vector<HitTestInfo> selection_info;
	Rect selection_region_union;
private:
	bool HasSelection() const { return selection_range_end > selection_range_begin; }
	void UpdateSelectionRegion();
	void RedrawSelectionRegion() { redraw_region = selection_region_union; Redraw(); }
public:
	void BeginSelection() { selection_begin = caret_position; }
	void DoSelection(Point mouse_move_position);
	void ClearSelection();
	void SelectWord();

	// keyboard input
public:
	void Insert(wchar ch);
	void Insert(std::wstring str);
	void Delete(bool is_backspace);

	// ime input
private:
	size_t ime_composition_begin = 0;
	size_t ime_composition_end = 0;
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
	bool is_ctrl_down = false;
	bool is_shift_down = false;
protected:
	virtual void OnKeyMsg(KeyMsg msg) override;
	virtual void OnNotifyMsg(NotifyMsg msg) override;
};


END_NAMESPACE(WndDesign)