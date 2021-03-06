#pragma once

#include "WndDesign/layout/ListLayout.h"
#include "WndDesign/wrapper/Padding.h"
#include "WndDesign/wrapper/Background.h"
#include "WndDesign/message/mouse_tracker.h"

#include "data_format.h"

#include "BlockStore/block_manager.h"


using namespace BlockStore;


BEGIN_NAMESPACE(WndDesign)

class ScrollView;
class TextView;


class ListView : public ListLayout<Vertical> {
public:
	static auto Create(ScrollView& scroll_view) {
		class ListViewDecorated : public Decorate<ListView, Padding, SolidColorBackground> {
		public:
			ListViewDecorated(ScrollView& scroll_view) : Base(scroll_view) {
				cursor = Cursor::Text;
				background = Color::White;
				padding = Margin(20, 10, 20, 10);
			}
		};
		return new ListViewDecorated(scroll_view);
	}

protected:
	ListView(ScrollView& scroll_view);
	~ListView();

	// parent
private:
	ScrollView& scroll_view;

	// data
private:
	BlockManager manager;
	BlockRef<TextListData> block;

	// child
private:
	TextView& Get(size_t index);
private:
	void Initialize();
public:
	void InsertAfter(TextView& child);
	void Delete(TextView& child);
	void DeleteAfter(TextView& child);
public:
	void InsertText(TextView& child, std::wstring text);

	// layout
protected:
	size_t HitTestPoint(Point& point) {
		if (point.y < 0.0f) { return 0; }
		auto it = HitTestItem(point.y); point.y -= it->offset;
		return it - child_list.begin();
	}
protected:
	virtual void OnSizeRefUpdate(Size size_ref) override;
	virtual ref_ptr<WndObject> HitTest(Point& point) override { return this; }

	// paint
protected:
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override;

	// caret
private:
	size_t caret_position = 0;
private:
	void SetCaret(Point point);
public:
	void SetCaretBefore(TextView& child);
	void SetCaretAfter(TextView& child);
	void SetCaretAt(TextView& child, Point point);

	// selection
private:
	size_t selection_begin = 0;
	size_t selection_range_begin = 0;
	size_t selection_range_end = 0;
	Rect selection_region;
private:
	bool HasSelection() const { return selection_range_end > selection_range_begin; }
	void UpdateSelectionRegion();
	void RedrawSelectionRegion() { redraw_region = selection_region; Redraw(); }
private:
	void BeginSelection();
	void DoSelection(Point point);
	void ClearSelection();
public:
	void SelectWord();
	void SelectParagraph();
	void SelectAll();

	// keyboard message
private:
	void Insert(wchar ch);
	void Delete();

	// clipboard
public:
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
};


END_NAMESPACE(WndDesign)