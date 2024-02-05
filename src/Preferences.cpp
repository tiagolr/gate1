#include "Preferences.h"
#include <string>

void Preferences::showPopupMenu()
{
  IPopupMenu* menu = new IPopupMenu();
  IPopupMenu* optionsMenu = new IPopupMenu();
  IPopupMenu* triggerMenu = new IPopupMenu();
  IPopupMenu* loadMenu = new IPopupMenu();


  triggerMenu->AddItem("Off")->SetChecked(gate.triggerChannel == 0);
  for (int i = 0; i < 16; ++i) {
    triggerMenu->AddItem(std::to_string(i+1).c_str())->SetChecked(gate.triggerChannel == i + 1);
  }
  triggerMenu->AddItem("Omni")->SetChecked(gate.triggerChannel == 17);

  optionsMenu->AddItem("Trigger channel")->SetSubmenu(triggerMenu);
  optionsMenu->AddItem("Dual smooth")->SetChecked(gate.dualSmooth);
  optionsMenu->AddItem("Link edge points")->SetChecked(gate.linkEdgePoints);

  loadMenu->AddItem("Sine");
  loadMenu->AddItem("Triangle");
  loadMenu->AddItem("Random");

  menu->AddItem("Options")->SetSubmenu(optionsMenu);
  menu->AddItem("Draw wave")->SetChecked(gate.drawWave);
  menu->AddSeparator();
  menu->AddItem("Invert");
  menu->AddItem("Reverse");
  menu->AddItem("Clear");
  menu->AddItem("Copy");
  menu->AddItem("Paste");
  menu->AddSeparator();
  menu->AddItem("Load")->SetSubmenu(loadMenu);
  menu->AddItem("About");
  
  GetUI()->CreatePopupMenu(*this, *menu, GetWidgetBounds().GetHShifted(-110));
}

void Preferences::OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) {
  if (pSelectedMenu == nullptr)
    return;

  DBGMSG("OKAY %i %s", pSelectedMenu->GetChosenItemIdx(), pSelectedMenu->GetChosenItem()->GetText());
  int idx = pSelectedMenu->GetChosenItemIdx();

}