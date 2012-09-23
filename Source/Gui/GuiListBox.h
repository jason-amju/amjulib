#ifndef GUI_LIST_BOX_H_INCLUDED
#define GUI_LIST_BOX_H_INCLUDED

#include <set>
#include "GuiWindow.h"
#include "GuiText.h"

namespace Amju
{
// Not sure how best to do this. 
// GuiListBox wraps up the complicated innards of a list box.
// It's really a Window, containing a Scroll control, containing a List.
class GuiList;
class GuiScroll;

class GuiListBox : public GuiWindow
{
protected:
  GuiListBox();
  friend GuiElement* CreateListBox();

public:
  static const char* NAME;

  // Convenient access to innards
  GuiList* GetList();
  GuiScroll* GetScroll();

  virtual bool Load(File*);
  virtual void Clear(); // Clear list, not the child of this object
};

// Callback which can be called on double-click
typedef void (*DoubleClickFunc)(GuiList*, int selectedIndex);

// Not factory-creatable
class GuiList : public GuiComposite 
{
public:
  GuiList();

  // Use in preference to AddChild, as it can resize the listbox to fit the text
  void AddItem(GuiText* text);  

  virtual void Draw();
  virtual bool Load(File*);
  virtual bool OnMouseButtonEvent(const MouseButtonEvent& mbe);
  virtual bool OnDoubleClickEvent(const DoubleClickEvent& dce);

  void SetSelected(int child, bool selected);
  bool IsSelected(int child) const;
  int GetSelectedItem() const; // only for non-multi-select lists

  void SetIsMultiSel(bool isMulti);
  bool IsMultiSel() const;

  void SetDoubleClickFunc(DoubleClickFunc dcf);

protected:
  bool m_isMultiSelect;
  typedef std::set<int> SelSet;
  SelSet m_selset;
  DoubleClickFunc m_dcf;
};

GuiElement* CreateListBox();
}

#endif

