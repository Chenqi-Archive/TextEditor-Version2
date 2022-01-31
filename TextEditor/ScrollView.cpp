#include "ScrollView.h"
#include "ListView.h"


BEGIN_NAMESPACE(WndDesign)


ScrollView::ScrollView() : ScrollBox(new ListViewDecorated(*this)) {}


END_NAMESPACE(WndDesign)