#pragma once

#include "BlockStore/block_layout.h"
#include "BlockStore/stl_helper.h"


using namespace BlockStore;


struct TextData {
	std::wstring text;
};

auto layout(layout_type<TextData>) { return declare(&TextData::text); }


constexpr wchar file[] = L"text.editor";