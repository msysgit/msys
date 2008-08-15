/** \file componentstree.cpp
 *
 * Created: JohnE, 2008-07-24
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#include "componentstree.hpp"

#include <stack>
#include "tdminst_res.h"
#include "install_manifest.hpp"
#include "config.h"


typedef RefType< TiXmlDocument >::Ref TiXmlDocumentRef;


static const char* NonEmptyAttribute(TiXmlElement* el, const char* attr)
{
	const char* val = el->Attribute(attr);
	return (val && val[0]) ? val : 0;
}


struct Item
{
	typedef RefType< Item >::Ref Ref;

	ItemType type;
	StringType text;
	TiXmlElement* element;
	HTREEITEM handle;
	bool prev_inst;
	bool selected;
	StringType description;
	int dlsize;
	int unsize;
	CheckState check;

	Item(ItemType t, TiXmlElement* el)
	 : type(t),
	 element(el),
	 handle(0),
	 prev_inst(false),
	 selected(false),
	 dlsize(0),
	 unsize(0),
	 check(CHECK_NONE)
	{
	}
};


ComponentsTree::ComponentsTree()
 : htreeview(0),
 nradio(0),
 ncheck(0),
 dl_size(0),
 inst_size(0),
 uninst_size(0)
{
}


void ComponentsTree::SetInnerArchive(const StringType& arc)
{
	inner_arcs.insert(arc);
}


bool ComponentsTree::BuildTreeView
 (HWND htv,
  int check_index,
  int radio_index,
  TiXmlElement* comp_man_root,
  TiXmlElement* prev_man_root)
{
	htreeview = htv;
	ncheck = check_index;
	nradio = radio_index;
	man_root = RefType< TiXmlElement >::Ref(
	 new TiXmlElement("TDMInstallManifest")
	 );
	index_items.clear();
	id_items.clear();
	dl_size = 0;
	inst_size = 0;
	uninst_size = 0;
	HTREEITEM hroot = AddItem(0, 0, ITEM_HEADER, StringType(),
	 "Components", 0);
	if (comp_man_root)
	{
		if (!ProcessManifest(comp_man_root, hroot))
			return false;
	}
	if (prev_man_root)
	{
		if (!ProcessManifest(prev_man_root, hroot))
			return false;
	}
	HTREEITEM hsmenu =
	 AddItem(0, 0, ITEM_EXTRA, "startmenu", "Start Menu items", 0);
	GetItemFromHandle(hsmenu)->description =
	 "Add an entry for " STR_SHORTNAME " to the Start Menu";
	OnStateToggle(hsmenu);
	TreeView_Expand(htreeview, hroot, TVE_EXPAND);
	return true;
}


bool ComponentsTree::OnStateToggle(HTREEITEM hitem)
{
	TVITEM tvitem;
	tvitem.hItem = hitem;
	tvitem.mask = TVIF_PARAM | TVIF_STATE;
	tvitem.stateMask = TVIS_STATEIMAGEMASK;
	if (!TreeView_GetItem(htreeview, &tvitem))
		return false;
	if (tvitem.lParam <= 0)
		return false;
	Item* item = RefGetPtr(index_items[tvitem.lParam - 1]);
	int cstate = item->check;
	if (item->type == ITEM_CATEGORY)
	{
		tvitem.mask = TVIF_STATE;
		tvitem.state = SetCheckState(item,
		 (cstate == CHECK_FULL) ? CHECK_NONE : CHECK_FULL);
		TreeView_SetItem(htreeview, &tvitem);
		UpdateCategoryChildren(hitem, (cstate != CHECK_FULL));
		UpdateAncestors(hitem, (cstate != CHECK_FULL));
		return true;
	}
	else if (item->type == ITEM_COMPONENT)
	{
		tvitem.mask = TVIF_STATE;
		tvitem.state = SetCheckState(item,
		 (cstate == CHECK_FULL) ? CHECK_NONE : CHECK_FULL);
		TreeView_SetItem(htreeview, &tvitem);
		Item* vhitem = 0;
		HTREEITEM vh = TreeView_GetChild(htreeview, hitem);
		if (vh)
			vhitem = GetItemFromHandle(vh);
		if (vh && (!vhitem || vhitem->type == ITEM_HEADER))
		{
			for (HTREEITEM hver = TreeView_GetChild(htreeview, vh);
			 hver;
			 hver = TreeView_GetNextSibling(htreeview, hver))
			{
				Item* vitem = GetItemFromHandle(hver);
				if (vitem && vitem->check == CHECK_FULL)
				{
					ToggleSel(vitem);
					break;
				}
			}
		}
		else
			ToggleSel(item);
		UpdateAncestors(hitem, (cstate != CHECK_FULL));
		return true;
	}
	else if (item->type == ITEM_VERSION && cstate != CHECK_FULL)
	{
		tvitem.mask = TVIF_STATE;
		tvitem.state = SetCheckState(item, CHECK_FULL);
		TreeView_SetItem(htreeview, &tvitem);
		HTREEITEM vh = TreeView_GetParent(htreeview, hitem);
		if (vh)
		{
			bool togglesel = false;
			HTREEITEM vparent = TreeView_GetParent(htreeview, vh);
			if (vparent)
			{
				Item* pitem = GetItemFromHandle(vparent);
				if (pitem && pitem->type == ITEM_COMPONENT && pitem->check == CHECK_FULL)
				{
					ToggleSel(item);
					togglesel = true;
				}
				else
					UpdateVersionedComps(vparent, hitem);
				if (pitem && pitem->handle && pitem->element)
				{
					const char* pname =
					 NonEmptyAttribute(pitem->element, "name");
					if (pname)
					{
						TVITEM ptvitem;
						ptvitem.hItem = pitem->handle;
						ptvitem.mask = TVIF_TEXT;
						char newtxt[200];
						snprintf(newtxt, 199, "%s (%s)", pname,
						 item->element->Attribute("name"));
						ptvitem.pszText = newtxt;
						ptvitem.cchTextMax = 199;
						TreeView_SetItem(htreeview, &ptvitem);
					}
				}
			}
			for (HTREEITEM vsib = TreeView_GetChild(htreeview, vh);
			 vsib;
			 vsib = TreeView_GetNextSibling(htreeview, vsib))
			{
				if (vsib == hitem)
					continue;
				TVITEM ctvitem;
				ctvitem.hItem = vsib;
				ctvitem.mask = TVIF_STATE | TVIF_PARAM;
				ctvitem.stateMask = TVIS_STATEIMAGEMASK;
				if (TreeView_GetItem(htreeview, &ctvitem))
				{
					if (ctvitem.lParam > 0)
					{
						Item* citem = RefGetPtr(index_items[ctvitem.lParam - 1]);
						if (citem->check == CHECK_FULL)
						{
							ctvitem.mask = TVIF_STATE;
							ctvitem.state = SetCheckState(citem, CHECK_NONE);
							TreeView_SetItem(htreeview, &ctvitem);
							if (togglesel)
								ToggleSel(citem);
							break;
						}
					}
				}
			}
		}
		return true;
	}
	else if (item->type == ITEM_EXTRA)
	{
		item->selected = !item->selected;
		tvitem.mask = TVIF_STATE;
		tvitem.state = SetCheckState(item,
		 (item->selected) ? CHECK_FULL : CHECK_NONE);
		TreeView_SetItem(htreeview, &tvitem);
		return false;
	}
	else
		return false;
}


int ComponentsTree::GetArchivesToInstall(ElementList& list) const
{
	int inst_count = 0;
	for (size_t i = 0; i < index_items.size(); ++i)
	{
		Item* item = RefGetPtr(index_items[i]);
		if (item->selected && !item->prev_inst && item->element)
		{
			for (TiXmlElement* ar_el =
			  item->element->FirstChildElement("Archive");
			 ar_el;
			 ar_el = ar_el->NextSiblingElement("Archive"))
			{
				++inst_count;
				list.push_back(ar_el);
			}
		}
	}
	return inst_count;
}


int ComponentsTree::GetComponentsToRemove
 (ElementList& list,
  TiXmlDocument* full_uninst) const
{
	int uninst_count = 0;
	if (!full_uninst)
	{
		for (size_t i = 0; i < index_items.size(); ++i)
		{
			Item* item = RefGetPtr(index_items[i]);
			if (!item->selected && item->prev_inst && item->element)
			{
				++uninst_count;
				list.push_back(item->element);
			}
		}
	}
	else
	{
		std::list< TiXmlElement* > search_els;
		search_els.push_back(full_uninst->RootElement());
		while (!search_els.empty())
		{
			TiXmlElement* el = search_els.front();
			search_els.pop_front();
			TiXmlElement* child = el->FirstChildElement();
			if (strcmp(child->Value(), "Entry") == 0)
			{
				++uninst_count;
				list.push_back(el);
			}
			else
			{
				for (; child; child = child->NextSiblingElement())
					search_els.push_back(child);
			}
		}
	}
	return uninst_count;
}


void ComponentsTree::WriteInstMan
 (const StringType& outpath,
  const InstallManifest& inst_man)
{
	TiXmlDocument out;
	TiXmlElement* root = new TiXmlElement("TDMInstallManifest");
	out.LinkEndChild(root);
	typedef std::map< std::string, TiXmlElement* > IDElementMap;
	IDElementMap linked_map;
	std::stack< TiXmlElement* > hierarchy;
	for (size_t i = 0; i < index_items.size(); ++i)
	{
		Item* item = RefGetPtr(index_items[i]);
		if (item->selected && item->element)
		{
			const char* item_id = NonEmptyAttribute(item->element, "id");
			if (item_id)
			{
				const TiXmlElement* entry_comp = inst_man.GetComponent(item_id);
				if (entry_comp)
				{
					TiXmlElement* link_el = root;
					TiXmlElement* new_el = entry_comp->Clone()->ToElement();
					new_el->SetValue(item->element->Value());
					const TiXmlAttribute* attribute = 0;
					for (attribute = item->element->FirstAttribute();
					 attribute;
					 attribute = attribute->Next())
					{
						new_el->SetAttribute(attribute->Name(),
						 attribute->Value());
					}
					new_el->SetAttribute("prev", "true");
					for (TiXmlElement* clone_el =
					  TiXmlHandle(item->element->Parent()).ToElement();
					 clone_el && strcmp(clone_el->Value(), "TDMInstallManifest") != 0;
					 clone_el = TiXmlHandle(clone_el->Parent()).ToElement())
					{
						IDElementMap::iterator found = linked_map.end();
						const char* el_id = NonEmptyAttribute(clone_el, "id");
						if (el_id)
							found = linked_map.find(el_id);
						if (found != linked_map.end())
						{
							link_el = found->second;
							break;
						}
						else
						{
							TiXmlElement* el = new TiXmlElement(clone_el->Value());
							for (attribute = clone_el->FirstAttribute();
							 attribute;
							 attribute = attribute->Next())
							{
								el->SetAttribute(attribute->Name(),
								 attribute->Value());
							}
							el->LinkEndChild(new_el);
							const char* new_id = NonEmptyAttribute(el, "id");
							if (new_id)
								linked_map.insert(std::make_pair(std::string(new_id), el));
							new_el = el;
						}
					}
					link_el->LinkEndChild(new_el);
				}
			}
		}
	}
	// MiscFiles node
	const TiXmlElement* mfiles = inst_man.GetComponent("MiscFiles");
	if (mfiles)
	{
		TiXmlElement* new_el = mfiles->Clone()->ToElement();
		root->LinkEndChild(new_el);
	}
	
	out.SaveFile(outpath.c_str());
}


int ComponentsTree::GetIndex(HTREEITEM hitem) const
{
	TVITEM tvitem;
	tvitem.hItem = hitem;
	tvitem.mask = TVIF_PARAM;
	if (TreeView_GetItem(htreeview, &tvitem))
	{
		if (tvitem.lParam > 0)
			return tvitem.lParam - 1;
	}
	return -1;
}


int ComponentsTree::GetIndex(const StringType& id) const
{
	IDIndexMap::const_iterator found = id_items.find(id);
	if (found != id_items.end())
		return found->second;
	return -1;
}


bool ComponentsTree::IsSelected(int index) const
{
	return index_items[index]->selected;
}


void ComponentsTree::SetSelected(int index, bool selected)
{
	Item* item = RefGetPtr(index_items[index]);
	if ((item->type == ITEM_COMPONENT || item->type == ITEM_VERSION)
	 && item->selected != selected)
	{
		if (selected)
			EnsureSelectable(item);
		if (!TreeView_GetChild(htreeview, item->handle))
			OnStateToggle(item->handle);
		if (item->type == ITEM_VERSION && !item->selected)
		{
			Item* vhitem;
			HTREEITEM vh = TreeView_GetParent(htreeview, item->handle);
			if (vh)
				vhitem = GetItemFromHandle(vh);
			if (vh && (!vhitem || vhitem->type == ITEM_HEADER))
			{
				HTREEITEM hparent = TreeView_GetParent(htreeview, vh);
				if (hparent)
				{
					Item* pitem = GetItemFromHandle(hparent);
					if (pitem && pitem->type == ITEM_COMPONENT)
						OnStateToggle(hparent);
				}
			}
		}
	}
}


void ComponentsTree::SetPrevInstSelected()
{
	for (size_t i = 0; i < index_items.size(); ++i)
	{
		Item* item = RefGetPtr(index_items[i]);
		if (item->prev_inst)
			SetSelected(i, true);
		else
			SetSelected(i, false);
	}
}


void ComponentsTree::DeselectAll()
{
	for (size_t i = 0; i < index_items.size(); ++i)
	{
		Item* item = RefGetPtr(index_items[i]);
		if (item->selected &&
		 (item->type == ITEM_COMPONENT || item->type == ITEM_VERSION))
		{
			ToggleSel(item);
			UINT st = SetCheckState(item, CHECK_NONE);
			if (item->handle)
			{
				TVITEM tvitem;
				tvitem.hItem = item->handle;
				tvitem.mask = TVIF_STATE;
				tvitem.stateMask = TVIS_STATEIMAGEMASK;
				tvitem.state = st;
				TreeView_SetItem(htreeview, &tvitem);
			}
		}
	}
}


const TiXmlElement* ComponentsTree::GetElement(int index) const
{
	return index_items[index]->element;
}


StringType ComponentsTree::GetDescription(int index) const
{
	return index_items[index]->description;
}


unsigned ComponentsTree::GetDownloadSize() const
{
	return dl_size;
}


unsigned ComponentsTree::GetInstallSize() const
{
	return inst_size;
}


unsigned ComponentsTree::GetUninstallSize() const
{
	return uninst_size;
}


struct InsParent
{
	TiXmlElement* ex;
	TiXmlElement* copy;
	HTREEITEM handle;

	InsParent(TiXmlElement* e, TiXmlElement* c, HTREEITEM h)
	 : ex(e),
	 copy(c),
	 handle(h)
	{
	}
};

bool ComponentsTree::ProcessManifest
 (TiXmlElement* mroot,
  HTREEITEM hroot)
{
	if (!mroot)
		return false;
	std::list< InsParent > search_children;
	search_children.push_back(InsParent(mroot, RefGetPtr(man_root), hroot));
	while (!search_children.empty())
	{
		TiXmlElement* ex_parent = search_children.front().ex;
		TiXmlElement* copy_parent = search_children.front().copy;
		HTREEITEM hparent = search_children.front().handle;
		search_children.pop_front();
		HTREEITEM vh = 0;
		for (TiXmlElement* el = ex_parent->FirstChildElement();
		 el;
		 el = el->NextSiblingElement())
		{
			const char* el_type = el->Value();
			if (strcmp(el_type, "Category") == 0
			 || strcmp(el_type, "Component") == 0
			 || strcmp(el_type, "Version") == 0)
			{
				HTREEITEM hitem = 0;
				TiXmlElement* linked_el = 0;
				const char* el_id = NonEmptyAttribute(el, "id");
				if (el_id)
				{
					IDIndexMap::iterator found = id_items.find(el_id);
					if (found != id_items.end())
					{
						Item* oitem = RefGetPtr(index_items[found->second]);
						hitem = oitem->handle;
						if (oitem->element)
							linked_el = oitem->element;
						if (NonEmptyAttribute(el, "prev"))
						{
							oitem->prev_inst = true;
							if (oitem->element)
							{
								oitem->element->SetAttribute("prev", "true");
								TiXmlNode* node = oitem->element->FirstChild();
								while (node)
								{
									TiXmlNode* temp = node;
									node = node->NextSibling();
									oitem->element->RemoveChild(temp);
								}
								node = el->FirstChild();
								while (node)
								{
									oitem->element->LinkEndChild(node->Clone());
									node = node->NextSibling();
								}
							}
							EnsurePrevText(oitem);
							UpdateItemAttrs(oitem, el, true);
						}
						else
							UpdateItemAttrs(oitem, el, false);
					}
				}
				if (!linked_el)
				{
					linked_el = new TiXmlElement(el->Value());
					const TiXmlAttribute* attribute = 0;
					for (attribute = el->FirstAttribute();
					 attribute;
					 attribute = attribute->Next())
					{
						linked_el->SetAttribute(attribute->Name(),
						 attribute->Value());
					}
					for (const TiXmlElement* cel = el->FirstChildElement();
					 cel;
					 cel = cel->NextSiblingElement())
					{
						if (strcmp(cel->Value(), "Description") == 0
						 || strcmp(cel->Value(), "Archive") == 0)
						{
							linked_el->LinkEndChild(cel->Clone());
						}
					}
					copy_parent->LinkEndChild(linked_el);
				}
				Item* pitem = GetItemFromHandle(hparent);
				if ((!pitem
				  || pitem->type == ITEM_CATEGORY
				  || pitem->type == ITEM_HEADER)
				 && strcmp(el_type, "Category") == 0)
				{
					if (!hitem)
					{
						hitem = AddItem(el, linked_el, ITEM_CATEGORY, StringType(),
						 StringType(), hparent);
					}
					search_children.push_back(InsParent(el, linked_el, hitem));
				}
				else if ((!pitem
				  || pitem->type == ITEM_HEADER
				  || pitem->type == ITEM_CATEGORY
				  || pitem->type == ITEM_VERSION)
				 && strcmp(el_type, "Component") == 0)
				{
					if (!hitem)
					{
						hitem = AddItem(el, linked_el, ITEM_COMPONENT, StringType(),
						 StringType(), hparent);
					}
					if (!pitem || pitem->type != ITEM_VERSION)
						search_children.push_back(InsParent(el, linked_el, hitem));
				}
				else if (pitem
				 && (pitem->type == ITEM_CATEGORY || pitem->type == ITEM_COMPONENT)
				 && strcmp(el_type, "Version") == 0)
				{
					if (!hitem)
					{
						if (!vh)
						{
							Item* vhitem = 0;
							vh = TreeView_GetChild(htreeview, hparent);
							if (vh)
								vhitem = GetItemFromHandle(vh);
							if (!vh || (vhitem && vhitem->type != ITEM_HEADER))
							{
								vh = AddItem(0, 0, ITEM_HEADER, StringType(),
								 "Version", hparent);
							}
						}
						hitem = AddItem(el, linked_el, ITEM_VERSION,
						 StringType(), StringType(), vh);
						TreeView_Expand(htreeview, vh, TVE_EXPAND);
					}
					if (pitem->type != ITEM_COMPONENT)
						search_children.push_back(InsParent(el, linked_el, hparent));
					const char* def = NonEmptyAttribute(el, "default");
					if (def)
						OnStateToggle(hitem);
				}
			}
		}
	}
	return true;
}


HTREEITEM ComponentsTree::AddItem
 (TiXmlElement* ex_el,
  TiXmlElement* store_el,
  ItemType type,
  const StringType& id,
  const StringType& txt,
  HTREEITEM hparent)
{
	int item_index = -1;
	Item* item = 0;
	const char* el_id = 0;
	if (id.length() > 0)
		el_id = id.c_str();
	else if (ex_el)
		el_id = NonEmptyAttribute(ex_el, "id");
	if (el_id)
	{
		IDIndexMap::iterator found = id_items.find(el_id);
		if (found != id_items.end())
		{
			item_index = found->second;
			item = RefGetPtr(index_items[item_index]);
		}
		else
		{
			item_index = index_items.size();
			Item::Ref new_item(new Item(type, store_el));
			item = RefGetPtr(new_item);
			index_items.push_back(new_item);
			id_items.insert(std::make_pair(std::string(el_id), item_index));
			if (ex_el)
			{
				if (NonEmptyAttribute(ex_el, "prev"))
					item->prev_inst = true;
				UpdateItemAttrs(item, ex_el, false);
			}
		}
	}

	if (item && item->handle)
		return item->handle;

	TVINSERTSTRUCT tvins;
	tvins.item.mask = TVIF_TEXT | TVIF_PARAM;
	const char* fmt = "%s";
	if (item && item->prev_inst)
		fmt = "[Installed] %s";
	char nstr[200];
	nstr[0] = 0;
	if (ex_el)
	{
		const char* el_name = NonEmptyAttribute(ex_el, "name");
		if (el_name)
			snprintf(nstr, 199, fmt, el_name);
	}
	if (!nstr[0] && txt.length() > 0)
		snprintf(nstr, 199, fmt, txt.c_str());
	if (!nstr[0])
		return 0;
	tvins.item.pszText = nstr;
	tvins.item.cchTextMax = 199;
	if (type == ITEM_COMPONENT
	 || type == ITEM_CATEGORY
	 || type == ITEM_VERSION
	 || type == ITEM_EXTRA)
	{
		tvins.item.mask |= TVIF_STATE;
		tvins.item.stateMask = TVIS_STATEIMAGEMASK;
		if (type == ITEM_VERSION)
			tvins.item.state = INDEXTOSTATEIMAGEMASK(nradio);
		else
			tvins.item.state = INDEXTOSTATEIMAGEMASK(ncheck);
	}
	tvins.item.lParam = (item_index >= 0) ? item_index + 1 : 0;
	if (type == ITEM_HEADER)
	{
		tvins.hInsertAfter = TVI_FIRST;
		if (hparent)
		{
			HTREEITEM sib = TreeView_GetChild(htreeview, hparent);
			while (sib)
			{
				Item* sibitem = GetItemFromHandle(sib);
				if (sibitem && sibitem->type != ITEM_HEADER)
					break;
				tvins.hInsertAfter = sib;
				sib = TreeView_GetNextSibling(htreeview, sib);
			}
		}
	}
	else if (item && item->prev_inst)
	{
		tvins.hInsertAfter = TVI_FIRST;
		if (hparent)
		{
			const char* ins_base = NonEmptyAttribute(ex_el, "base");
			HTREEITEM sib = TreeView_GetChild(htreeview, hparent);
			while (sib)
			{
				Item* sibitem = GetItemFromHandle(sib);
				if (sibitem)
				{
					if (ins_base
					 && sibitem->type == type
					 && sibitem->element)
					{
						const char* cmp_base =
						 NonEmptyAttribute(sibitem->element, "base");
						if (cmp_base && strcmp(cmp_base, ins_base) == 0)
						{
							tvins.hInsertAfter =
							 TreeView_GetPrevSibling(htreeview, sib);
							if (!tvins.hInsertAfter)
								tvins.hInsertAfter = TVI_FIRST;
							break;
						}
					}
					if (sibitem->prev_inst || sibitem->type == ITEM_HEADER)
						tvins.hInsertAfter = sib;
				}
				else
					tvins.hInsertAfter = sib;
				sib = TreeView_GetNextSibling(htreeview, sib);
			}
		}
	}
	else
		tvins.hInsertAfter = TVI_LAST;
	tvins.hParent = (hparent) ? hparent : TVI_ROOT;
	HTREEITEM hitem = (HTREEITEM)SendMessage(htreeview, TVM_INSERTITEM, 0,
	 (LPARAM)(LPTVINSERTSTRUCT)&tvins);

	if (item)
		item->handle = hitem;

	return hitem;
}


Item* ComponentsTree::GetItemFromHandle(HTREEITEM handle)
{
	TVITEM tvitem;
	tvitem.hItem = handle;
	tvitem.mask = TVIF_PARAM;
	if (!TreeView_GetItem(htreeview, &tvitem))
		return 0;
	if (tvitem.lParam <= 0)
		return 0;
	return RefGetPtr(index_items[tvitem.lParam - 1]);
}


void ComponentsTree::UpdateCategoryChildren(HTREEITEM hcat, bool checked)
{
	for (HTREEITEM hchild = TreeView_GetChild(htreeview, hcat);
	 hchild;
	 hchild = TreeView_GetNextSibling(htreeview, hchild))
	{
		TVITEM tvitem;
		tvitem.hItem = hchild;
		tvitem.mask = TVIF_PARAM | TVIF_STATE;
		tvitem.stateMask = TVIS_STATEIMAGEMASK;
		if (TreeView_GetItem(htreeview, &tvitem))
		{
			if (tvitem.lParam > 0)
			{
				Item* item = RefGetPtr(index_items[tvitem.lParam - 1]);
				if ((item->type == ITEM_COMPONENT
				  || item->type == ITEM_CATEGORY)
				 && item->check != ((checked) ? CHECK_FULL : CHECK_NONE))
				{
					tvitem.mask = TVIF_STATE;
					tvitem.state = SetCheckState(item,
					 (checked) ? CHECK_FULL : CHECK_NONE);
					TreeView_SetItem(htreeview, &tvitem);
					if (item->type == ITEM_CATEGORY)
						UpdateCategoryChildren(hchild, checked);
					else
						ToggleSel(item);
				}
			}
		}
	}
}


void ComponentsTree::UpdateAncestors(HTREEITEM hitem, bool checked)
{
	for (HTREEITEM parent = TreeView_GetParent(htreeview, hitem);
	 parent;
	 parent = TreeView_GetParent(htreeview, parent))
	{
		TVITEM tvitem;
		tvitem.hItem = parent;
		tvitem.mask = TVIF_PARAM | TVIF_STATE;
		tvitem.stateMask = TVIS_STATEIMAGEMASK;
		if (TreeView_GetItem(htreeview, &tvitem) && tvitem.lParam > 0)
		{
			Item* pitem = RefGetPtr(index_items[tvitem.lParam - 1]);
			if (pitem->type != ITEM_CATEGORY)
				break;
			tvitem.mask = TVIF_STATE;
			tvitem.state = SetCheckState(pitem,
			 (checked) ? CHECK_FULL : CHECK_NONE);
			for (HTREEITEM pchild = TreeView_GetChild(htreeview, parent);
			 pchild;
			 pchild = TreeView_GetNextSibling(htreeview, pchild))
			{
				TVITEM ctvitem;
				ctvitem.hItem = pchild;
				ctvitem.mask = TVIF_PARAM | TVIF_STATE;
				ctvitem.stateMask = TVIS_STATEIMAGEMASK;
				if (TreeView_GetItem(htreeview, &ctvitem) && ctvitem.lParam > 0)
				{
					Item* citem = RefGetPtr(index_items[ctvitem.lParam - 1]);
					if ((citem->type == ITEM_COMPONENT
					  || citem->type == ITEM_CATEGORY)
					 && citem->check != ((checked) ? CHECK_FULL : CHECK_NONE))
					{
						tvitem.state = SetCheckState(pitem, CHECK_PARTIAL);
						break;
					}
				}
			}
			TreeView_SetItem(htreeview, &tvitem);
		}
	}
}


void ComponentsTree::ToggleSizes(Item* item, bool selected)
{
	int mod = (selected) ? 1 : -1;
	if (item->prev_inst)
		uninst_size -= mod * item->unsize;
	else
	{
		dl_size += mod * item->dlsize;
		inst_size += mod * item->unsize;
	}
}


void ComponentsTree::UpdateVersionedComps(HTREEITEM hparent, HTREEITEM hver)
{
	typedef std::map< StringType, bool > IDSelMap;
	IDSelMap base_states;
	HTREEITEM hiter = TreeView_GetChild(htreeview, hparent);
	while (hiter)
	{
		HTREEITEM hchild = hiter;
		hiter = TreeView_GetNextSibling(htreeview, hiter);
		Item* citem = GetItemFromHandle(hchild);
		if (citem && citem->type == ITEM_COMPONENT)
		{
			if (citem->element)
			{
				const char* base = NonEmptyAttribute(citem->element, "base");
				if (base)
					base_states[base] = citem->selected;
			}
			if (citem->selected)
				ToggleSel(citem);
			citem->handle = 0;
			TreeView_DeleteItem(htreeview, hchild);
		}
	}
	Item* vitem = GetItemFromHandle(hver);
	if (vitem && vitem->element)
	{
		for (TiXmlElement* comp_el =
		  vitem->element->FirstChildElement("Component");
		 comp_el;
		 comp_el = comp_el->NextSiblingElement("Component"))
		{
			HTREEITEM hnewitem = AddItem(comp_el, comp_el, ITEM_COMPONENT,
			 StringType(), StringType(), hparent);
			Item* new_item = GetItemFromHandle(hnewitem);
			UINT st = SetCheckState(new_item, new_item->check);
			const char* base = NonEmptyAttribute(comp_el, "base");
			if (base)
			{
				IDSelMap::iterator found = base_states.find(base);
				if (found != base_states.end())
				{
					st = SetCheckState(new_item,
					 (found->second ? CHECK_FULL : CHECK_NONE));
				}
			}
			if (new_item->selected != (new_item->check == CHECK_FULL))
				ToggleSel(new_item);
			TVITEM ctvitem;
			ctvitem.hItem = new_item->handle;
			ctvitem.mask = TVIF_STATE;
			ctvitem.stateMask = TVIS_STATEIMAGEMASK;
			ctvitem.state = st;
			TreeView_SetItem(htreeview, &ctvitem);
			UpdateAncestors(new_item->handle, new_item->selected);
		}
	}
}


void ComponentsTree::EnsureSelectable(Item* item)
{
	if (!item || !item->element)
		return;
	std::stack< int > pstack;
	for (TiXmlElement* p_el = TiXmlHandle(item->element->Parent()).ToElement();
	 p_el;
	 p_el = TiXmlHandle(p_el->Parent()).ToElement())
	{
		if (strcmp(p_el->Value(), "Version") == 0)
		{
			const char* v_id = NonEmptyAttribute(p_el, "id");
			if (v_id)
			{
				int v_index = GetIndex(v_id);
				if (v_index >= 0)
					pstack.push(v_index);
			}
		}
	}
	while (!pstack.empty())
	{
		HTREEITEM hver = index_items[pstack.top()]->handle;
		if (hver)
			OnStateToggle(hver);
		pstack.pop();
	}
}


void ComponentsTree::ToggleSel(Item* item)
{
	if (!item)
		return;
	HTREEITEM vh = 0;
	Item* vhitem = 0;
	if (item->handle)
	{
		vh = TreeView_GetChild(htreeview, item->handle);
		if (vh)
			vhitem = GetItemFromHandle(vh);
	}
	if (vh && (!vhitem || vhitem->type == ITEM_HEADER))
	{
		for (HTREEITEM vchild = TreeView_GetChild(htreeview, vh);
		 vchild;
		 vchild = TreeView_GetNextSibling(htreeview, vchild))
		{
			TVITEM ctvitem;
			ctvitem.hItem = vchild;
			ctvitem.mask = TVIF_STATE | TVIF_PARAM;
			ctvitem.stateMask = TVIS_STATEIMAGEMASK;
			if (TreeView_GetItem(htreeview, &ctvitem) && ctvitem.lParam > 0)
			{
				Item* citem = RefGetPtr(index_items[ctvitem.lParam - 1]);
				if (citem->check == CHECK_FULL)
				{
					citem->selected = !citem->selected;
					ToggleSizes(citem, citem->selected);
					break;
				}
			}
		}
	}
	else
	{
		item->selected = !item->selected;
		ToggleSizes(item, item->selected);
	}
}


UINT ComponentsTree::SetCheckState(Item* item, CheckState state)
{
	item->check = state;
	return INDEXTOSTATEIMAGEMASK(
	 (item->type == ITEM_VERSION) ? (nradio + state) : (ncheck + state)
	 );
}


void ComponentsTree::UpdateItemAttrs(Item* item, TiXmlElement* attr_el,
 bool prev_inst)
{
	if (item && attr_el)
	{
		TiXmlText* txt = TiXmlHandle(attr_el->FirstChildElement("Description")).FirstChild().ToText();
		if (txt && strlen(txt->Value()) > 0)
			item->description = txt->Value();
		int size;
		if (attr_el->QueryIntAttribute("unsize", &size) == TIXML_SUCCESS)
		{
			if (prev_inst)
				uninst_size += size;
			item->unsize = size;
		}
		for (TiXmlElement* ar_el = attr_el->FirstChildElement("Archive");
		 ar_el;
		 ar_el = ar_el->NextSiblingElement("Archive"))
		{
			const char* ar_path = NonEmptyAttribute(ar_el, "path");
			if (ar_path)
			{
				if (ar_el->QueryIntAttribute("arcsize", &size) ==
				 TIXML_SUCCESS)
				{
					int at = strlen(ar_path) - 2;
					for (; at >= 0 && ar_path[at] != '/';
					 --at);
					if (inner_arcs.count(ar_path + at + 1) <= 0)
						item->dlsize += size;
				}
			}
		}
	}
}


void ComponentsTree::EnsurePrevText(Item* item)
{
	if (!item || !item->handle || !item->element)
		return;
	char nstr[200];
	nstr[0] = 0;
	const char* el_name = NonEmptyAttribute(item->element, "name");
	if (!el_name)
		return;
	snprintf(nstr, 199, "[Installed] %s", el_name);
	TVITEM tvitem;
	tvitem.hItem = item->handle;
	tvitem.mask = TVIF_TEXT;
	tvitem.pszText = nstr;
	tvitem.cchTextMax = 199;
	TreeView_SetItem(htreeview, &tvitem);
}

