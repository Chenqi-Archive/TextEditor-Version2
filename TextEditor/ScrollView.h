#pragma once

#pragma once

#include "WndDesign/frame/ScrollFrame.h"
#include "WndDesign/control/Scrollbar.h"
#include "WndDesign/layout/SplitLayout.h"


BEGIN_NAMESPACE(WndDesign)


class ScrollView : public SplitLayout<Horizontal, Second> {
protected:
	using ScrollFrame = ScrollFrame<Vertical>;
	using Scrollbar = Scrollbar<Vertical>;

public:
	ScrollView();

protected:
	class Frame : public ScrollFrame {
	private:
		using ScrollFrame::ScrollFrame;
		ScrollView& GetScrollView() const { return static_cast<ScrollView&>(GetParent()); }
		virtual void OnFrameOffsetUpdate() override { GetScrollView().GetScrollbar().UpdateScrollOffset(); }
	public:
		void ScrollIntoView(WndObject& wnd, Rect region) {
			region.point = child->ConvertPoint(wnd, region.point);
			ScrollFrame::ScrollIntoView(region.point.y, region.size.height);
		}
	};

protected:
	Frame& GetScrollFrame() const { return static_cast<Frame&>(*first); }
	Scrollbar& GetScrollbar() const { return static_cast<Scrollbar&>(*second); }

public:
	void Scroll(float wheel_delta) {
		GetScrollFrame().Scroll(wheel_delta);
	}
	void ScrollIntoView(WndObject& wnd, Rect region) { GetScrollFrame().ScrollIntoView(wnd, region); }
};


END_NAMESPACE(WndDesign)