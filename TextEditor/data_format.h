#pragma once

#include "BlockStore/block_layout.h"
#include "BlockStore/stl_helper.h"


struct TextData {
	std::wstring text;
};


BEGIN_NAMESPACE(BlockStore)

auto layout(layout_type<TextData>) { return declare(&TextData::text); }

constexpr wchar file[] = L"text.editor";

END_NAMESPACE(BlockStore)