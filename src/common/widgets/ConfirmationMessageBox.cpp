#include <compose/widgets/ConfirmationMessageBox.h>

#include <utility>

#include "reactive/Deferrer.h"

#include "src/widgets/msgbox/lv_msgbox.h"
#include "src/widgets/label/lv_label.h"

namespace
{
  constexpr uint32_t c_black = 0x000000;
  constexpr uint32_t c_white = 0xFFFFFF;
  constexpr uint32_t c_msgBoxBackground = 0x171717;
  constexpr uint32_t c_panelBackground = 0x1F1F1F;
  constexpr uint32_t c_separatorBorder = 0x3A3A3A;
  constexpr uint32_t c_closeButtonText = 0xB9B9B9;
  constexpr uint32_t c_closeButtonActive = 0x2B2B2B;
  constexpr uint32_t c_messageText = 0xD5D5D5;
  constexpr uint32_t c_yesButtonBackground = 0x3A3A3A;
  constexpr uint32_t c_yesButtonPressed = 0x4A4A4A;
  constexpr uint32_t c_noButtonBackground = 0x262626;
  constexpr uint32_t c_noButtonPressed = 0x333333;

  Compose::ConfirmationCallback s_callback;

  constexpr lv_style_selector_t buildStyleSelector(const lv_part_t part, const lv_state_t state)
  {
    return static_cast<lv_style_selector_t>(part) | static_cast<lv_state_t>(state);
  }
}

static void styleFooterButton(lv_obj_t *button, lv_color_t baseBgColor, lv_color_t pressedBgColor)
{
  lv_obj_remove_style_all(button);
  lv_obj_set_style_bg_color(button, baseBgColor, buildStyleSelector(LV_PART_MAIN , LV_STATE_DEFAULT));
  lv_obj_set_style_bg_grad_color(button, baseBgColor, buildStyleSelector(LV_PART_MAIN , LV_STATE_DEFAULT));
  lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_NONE, buildStyleSelector(LV_PART_MAIN , LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(button, LV_OPA_COVER, buildStyleSelector(LV_PART_MAIN , LV_STATE_DEFAULT));
  lv_obj_set_style_border_width(button, 0, buildStyleSelector(LV_PART_MAIN , LV_STATE_DEFAULT));
  lv_obj_set_style_outline_width(button, 0, buildStyleSelector(LV_PART_MAIN , LV_STATE_DEFAULT));
  lv_obj_set_style_shadow_width(button, 0, buildStyleSelector(LV_PART_MAIN , LV_STATE_DEFAULT));
  lv_obj_set_style_radius(button, 3, LV_PART_MAIN);
  lv_obj_set_style_text_color(button, lv_color_hex(c_white), buildStyleSelector(LV_PART_MAIN , LV_STATE_DEFAULT));
  lv_obj_set_style_pad_left(button, 16, LV_PART_MAIN);
  lv_obj_set_style_pad_right(button, 16, LV_PART_MAIN);
  lv_obj_set_style_pad_top(button, 6, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(button, 6, LV_PART_MAIN);
  lv_obj_set_style_bg_color(button, pressedBgColor, buildStyleSelector(LV_PART_MAIN , LV_STATE_PRESSED));
  lv_obj_set_style_bg_color(button, pressedBgColor, buildStyleSelector(LV_PART_MAIN , LV_STATE_FOCUSED));
  lv_obj_set_style_bg_grad_color(button, pressedBgColor, buildStyleSelector(LV_PART_MAIN , LV_STATE_PRESSED));
  lv_obj_set_style_bg_grad_color(button, pressedBgColor, buildStyleSelector(LV_PART_MAIN , LV_STATE_FOCUSED));
  lv_obj_set_style_text_color(button, lv_color_hex(c_white), buildStyleSelector(LV_PART_MAIN , LV_STATE_PRESSED));
  lv_obj_set_style_text_color(button, lv_color_hex(c_white), buildStyleSelector(LV_PART_MAIN , LV_STATE_FOCUSED));
  lv_obj_set_style_border_width(button, 0, buildStyleSelector(LV_PART_MAIN , LV_STATE_PRESSED));
  lv_obj_set_style_border_width(button, 0, buildStyleSelector(LV_PART_MAIN , LV_STATE_FOCUSED));
  lv_obj_set_style_outline_width(button, 0, buildStyleSelector(LV_PART_MAIN , LV_STATE_PRESSED));
  lv_obj_set_style_outline_width(button, 0, buildStyleSelector(LV_PART_MAIN , LV_STATE_FOCUSED));
  lv_obj_set_style_shadow_width(button, 0, buildStyleSelector(LV_PART_MAIN , LV_STATE_PRESSED));
  lv_obj_set_style_shadow_width(button, 0, buildStyleSelector(LV_PART_MAIN , LV_STATE_FOCUSED));
  lv_obj_remove_flag(button, LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t *createFooterButton(lv_obj_t *footer, const char *labelText)
{
  auto *button = lv_obj_create(footer);
  auto *label = lv_label_create(button);
  lv_label_set_text(label, labelText);
  lv_obj_center(label);
  return button;
}

static void styleMessageBox(lv_obj_t *msgBox, lv_obj_t *closeButton, lv_obj_t *textLabel, lv_obj_t *yesButton, lv_obj_t *noButton)
{
  auto *backdrop = lv_obj_get_parent(msgBox);
  lv_obj_set_style_bg_color(backdrop, lv_color_hex(c_black), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(backdrop, LV_OPA_70, LV_PART_MAIN);
  lv_obj_set_style_border_width(backdrop, 0, LV_PART_MAIN);

  lv_obj_set_style_bg_color(msgBox, lv_color_hex(c_msgBoxBackground), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(msgBox, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(msgBox, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(msgBox, lv_color_hex(c_separatorBorder), LV_PART_MAIN);
  lv_obj_set_style_radius(msgBox, 4, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(msgBox, 0, LV_PART_MAIN);
  lv_obj_set_style_outline_width(msgBox, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(msgBox, 0, LV_PART_MAIN);

  if(auto *header = lv_msgbox_get_header(msgBox))
  {
    lv_obj_set_style_bg_color(header, lv_color_hex(c_panelBackground), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_border_color(header, lv_color_hex(c_separatorBorder), LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_left(header, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_right(header, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_top(header, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(header, 6, LV_PART_MAIN);
  }

  if(auto *title = lv_msgbox_get_title(msgBox))
    lv_obj_set_style_text_color(title, lv_color_hex(c_white), LV_PART_MAIN);

  lv_obj_remove_style_all(closeButton);
  lv_obj_set_style_bg_opa(closeButton, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_bg_grad_dir(closeButton, LV_GRAD_DIR_NONE, LV_PART_MAIN);
  lv_obj_set_style_border_width(closeButton, 0, LV_PART_MAIN);
  lv_obj_set_style_outline_width(closeButton, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(closeButton, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(closeButton, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(closeButton, 2, LV_PART_MAIN);
  lv_obj_set_style_text_color(closeButton, lv_color_hex(c_closeButtonText), LV_PART_MAIN);
  lv_obj_set_style_bg_color(closeButton, lv_color_hex(c_closeButtonActive), buildStyleSelector(LV_PART_MAIN , LV_STATE_PRESSED));
  lv_obj_set_style_bg_color(closeButton, lv_color_hex(c_closeButtonActive), buildStyleSelector(LV_PART_MAIN , LV_STATE_FOCUSED));
  lv_obj_set_style_text_color(closeButton, lv_color_hex(c_white), buildStyleSelector(LV_PART_MAIN , LV_STATE_PRESSED));
  lv_obj_set_style_text_color(closeButton, lv_color_hex(c_white), buildStyleSelector(LV_PART_MAIN , LV_STATE_FOCUSED));
  lv_obj_set_size(closeButton, 20, 20);
  lv_obj_remove_flag(closeButton, LV_OBJ_FLAG_SCROLLABLE);

  if(auto *content = lv_msgbox_get_content(msgBox))
  {
    lv_obj_set_style_bg_color(content, lv_color_hex(c_msgBoxBackground), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_left(content, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_right(content, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_top(content, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(content, 10, LV_PART_MAIN);
  }

  lv_obj_set_style_text_color(textLabel, lv_color_hex(c_messageText), LV_PART_MAIN);

  if(auto *footer = lv_msgbox_get_footer(msgBox))
  {
    lv_obj_set_style_bg_color(footer, lv_color_hex(c_panelBackground), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(footer, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_side(footer, LV_BORDER_SIDE_TOP, LV_PART_MAIN);
    lv_obj_set_style_border_color(footer, lv_color_hex(c_separatorBorder), LV_PART_MAIN);
    lv_obj_set_style_border_width(footer, 1, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(footer, LV_OPA_TRANSP, buildStyleSelector(LV_PART_ITEMS , LV_STATE_ANY));
    lv_obj_set_style_border_width(footer, 0, buildStyleSelector(LV_PART_ITEMS , LV_STATE_ANY));
    lv_obj_set_style_outline_width(footer, 0, buildStyleSelector(LV_PART_ITEMS , LV_STATE_ANY));
    lv_obj_set_style_shadow_width(footer, 0, buildStyleSelector(LV_PART_ITEMS , LV_STATE_ANY));
    lv_obj_set_style_bg_grad_dir(footer, LV_GRAD_DIR_NONE, buildStyleSelector(LV_PART_ITEMS , LV_STATE_ANY));
    lv_obj_set_style_pad_left(footer, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_right(footer, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_top(footer, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(footer, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_column(footer, 8, LV_PART_MAIN);
  }

  styleFooterButton(yesButton, lv_color_hex(c_yesButtonBackground), lv_color_hex(c_yesButtonPressed));
  styleFooterButton(noButton, lv_color_hex(c_noButtonBackground), lv_color_hex(c_noButtonPressed));
}

static void setResultAndClose(lv_event_t *e)
{
  if(auto *msgBox = static_cast<lv_obj_t *>(lv_event_get_user_data(e)))
    lv_msgbox_close(msgBox);
}

static void yes(lv_event_t *e)
{
  Reactive::Deferrer deferrer;
  if(s_callback)
    s_callback();
  s_callback = nullptr;
  setResultAndClose(e);
}

static void no(lv_event_t *e)
{
  Reactive::Deferrer deferrer;
  setResultAndClose(e);
}

void Compose::confirm(const std::string &title, const std::string &text, ConfirmationCallback callback)
{
  s_callback = std::move(callback);

  auto *mbox1 = lv_msgbox_create(nullptr);

  lv_msgbox_add_title(mbox1, title.c_str());
  auto *textLabel = lv_msgbox_add_text(mbox1, text.c_str());
  auto *closeButton = lv_msgbox_add_close_button(mbox1);

  lv_msgbox_add_footer_button(mbox1, nullptr);
  auto *footer = lv_msgbox_get_footer(mbox1);
  lv_obj_clean(footer);

  auto *yesButton = createFooterButton(footer, "Yes");
  auto *noButton = createFooterButton(footer, "No");
  lv_obj_add_event_cb(yesButton, yes, LV_EVENT_CLICKED, mbox1);
  lv_obj_add_event_cb(noButton, no, LV_EVENT_CLICKED, mbox1);

  styleMessageBox(mbox1, closeButton, textLabel, yesButton, noButton);
}
