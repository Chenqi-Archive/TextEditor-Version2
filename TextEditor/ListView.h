#pragma once

#include "WndDesign/layout/ListLayout.h"
#include "WndDesign/wrapper/Padding.h"
#include "WndDesign/wrapper/Background.h"

#include "data_format.h"

#include "BlockStore/block_manager.h"


using namespace BlockStore;


BEGIN_NAMESPACE(WndDesign)

class ScrollView;
class TextView;


class ListView : public ListLayout<Vertical> {
public:
	ListView(ScrollView& scroll_view);
	~ListView();

	// parent
private:
	ScrollView& scroll_view;

	// data
private:
	BlockManager manager;
	BlockRef<TextListData> block;

	// layout
protected:
	virtual ref_ptr<WndObject> HitTest(Point& point) override;

	// child
private:
	TextView& Get(size_t index);
public:
	void InsertAfter(TextView& child, size_t pos);
	void Delete(TextView& child);
	void DeleteAfter(TextView& child);

	// caret
public:
	void SetCaretBefore(TextView& child);
	void SetCaretAfter(TextView& child);
	void SetCaretAt(TextView& child, Point point);
};


class ListViewDecorated : public Decorate<ListView, Padding, SolidColorBackground> {
public:
	ListViewDecorated(ScrollView& scroll_view) : Base(scroll_view) {
		cursor = Cursor::Text;
		background = Color::White;
		padding = Margin(20, 10, 20, 10);
	}
};


END_NAMESPACE(WndDesign)