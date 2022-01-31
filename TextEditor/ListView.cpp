#include "ListView.h"
#include "TextView.h"
#include "ScrollView.h"

#include "BlockStore/block_manager.h"
#include "BlockStore/file_manager.h"


BEGIN_NAMESPACE(WndDesign)


ListView::ListView(ScrollView& scroll_view) : ListLayout(5px), manager(std::make_unique<FileManager>(file)), scroll_view(scroll_view) {
	try {
		manager.LoadRootRef(block);
	} catch (std::runtime_error&) {
		block = manager;
	}
	auto list = block.Read();
	if (list->empty()) {
		AppendChild(new TextView(BlockRef<TextData>(manager), scroll_view));
	} else {
		for (auto item : *list) {
			AppendChild(new TextView(item, scroll_view));
		}
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

ref_ptr<WndObject> ListView::HitTest(Point& point) {
	if (point.y < 0.0f) { return child_list.front().child; }
	auto it = HitTestItem(point.y); point.y -= it->offset;
	return it->child;
}

TextView& ListView::Get(size_t index) { return static_cast<TextView&>(*child_list[index].child); }

void ListView::InsertAfter(TextView& child, size_t pos) {
	size_t index = GetChildData(child) + 1;
	InsertChild(index, new TextView(BlockRef<TextData>(manager), scroll_view));
	TextView& text_view = Get(index);
	text_view.SetText(child.GetText().substr(pos)); child.DeleteText(pos, (size_t)-1);
	text_view.SetCaret(0);
}

void ListView::Delete(TextView& child) {
	size_t index = GetChildData(child); if (index == 0) { return; }
	TextView& prev = Get(index - 1);
	prev.SetCaret(prev.GetText().length()); prev.AppendText(child.GetText());
	EraseChild(index, 1);
}

void ListView::DeleteAfter(TextView& child) {
	size_t index = GetChildData(child); if (index == child_list.size() - 1) { return; }
	TextView& next = Get(index + 1);
	child.AppendText(next.GetText());
	EraseChild(index + 1, 1);
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
	point *= GetChildTransform(child);
	static_cast<TextView&>(*HitTest(point)).SetCaret(point);
}


END_NAMESPACE(WndDesign)