#pragma once

#include "BlockStore/block_layout.h"
#include "BlockStore/stl_helper.h"
#include "BlockStore/block_ref.h"


BEGIN_NAMESPACE(BlockStore)

constexpr wchar file[] = L"text.editor";

using TextData = std::wstring;
using TextListData = std::vector<BlockRef<TextData>>;

END_NAMESPACE(BlockStore)