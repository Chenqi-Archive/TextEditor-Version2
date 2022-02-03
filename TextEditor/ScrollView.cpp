#include "ScrollView.h"
#include "ListView.h"


BEGIN_NAMESPACE(WndDesign)


ScrollView::ScrollView() : SplitLayout{ new Frame(new ListViewDecorated(*this)), new Scrollbar() } {
	GetScrollbar().SetScrollFrame(GetScrollFrame());
}


END_NAMESPACE(WndDesign)