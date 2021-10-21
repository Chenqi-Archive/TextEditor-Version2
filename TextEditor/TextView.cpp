#include "TextView.h"
#include "data_format.h"

#include "BlockStore/block_manager.h"
#include "BlockStore/file_manager.h"


using namespace WndDesign;
using namespace BlockStore;


TextView::TextView() : EditBox(Style()) {
	try {
		BlockManager manager(std::make_unique<FileManager>(file));
		BlockRef<TextData> root; manager.LoadRootRef(root);
		SetText(root.Read()->text);
	} catch (std::exception&) {}
}

TextView::~TextView() {
	try {
		BlockManager manager(std::make_unique<FileManager>(file));
		BlockRef<TextData> root(manager);
		root.Write()->text = text;
		manager.Format(); manager.SaveRootRef(root);
	} catch (std::exception&) {}
}