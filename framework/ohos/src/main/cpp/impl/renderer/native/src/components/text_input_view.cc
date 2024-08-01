/*
 *
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2019 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "renderer/components/text_input_view.h"
#include "renderer/utils/hr_event_utils.h"
#include "renderer/utils/hr_value_utils.h"
#include "renderer/utils/hr_text_convert_utils.h"
#include "footstone/logging.h"


namespace hippy {
inline namespace render {
inline namespace native {

TextInputView::TextInputView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {}

TextInputView::~TextInputView() {}

StackNode &TextInputView::GetLocalRootArkUINode() { return stackNode_; }

TextInputBaseNode &TextInputView::GetTextNode() {
    if (this->multiline == false)
        return textInputNode_;
    else 
      return textAreaNode_;
}

void TextInputView::InitNode(){
    if (this->multiline == false) {
        stackNode_.AddChild(textInputNode_);
        textInputNode_.SetTextInputNodeDelegate(this);
    } else {
        stackNode_.AddChild(textAreaNode_);
        textAreaNode_.SetTextAreaNodeDelegate(this);
    }
    GetTextNode().SetBorderRadius(0, 0, 0, 0);
    GetTextNode().SetBackgroundColor(0x00000000);
}

bool TextInputView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" propkey = "<<propKey;
  if (propKey == "caret-color") {
    this->caretColor = HRValueUtils::GetUint32(propValue);
    return true;
  } else if (propKey == "color") {
    this->color = HRValueUtils::GetUint32(propValue);
    return true;
  } else if (propKey == "defaultValue") {
    this->value = HRValueUtils::GetString(propValue);
    return true;
  } else if (propKey == "fontFamily") {
    this->fontFamily = HRValueUtils::GetString(propValue);
    return true;
  } else if (propKey == "fontSize") {
    this->fontSize = HRValueUtils::GetFloat(propValue);
    return true;
  } else if (propKey == "fontStyle") {
    std::string style = HRValueUtils::GetString(propValue);
    if(style == "italic")
      this->fontStyle = ArkUI_FontStyle::ARKUI_FONT_STYLE_ITALIC;
    return true;
  } else if (propKey == "fontWeight") {
    SetFontWeight(propValue);
    return true;
  } else if (propKey == "maxLength") {
    this->maxLength = HRValueUtils::GetUint32(propValue);
    return true;
  } else if (propKey == "multiline") {
    this->multiline = HRValueUtils::GetBool(propValue, false);
    FOOTSTONE_DLOG(INFO)<<"multiline = "<< this->multiline;
    if(this->multiline)
      this->textAlignVertical = ArkUI_Alignment::ARKUI_ALIGNMENT_TOP;
    return true;
  } else if (propKey == "textAlign") {
    SetTextAlign(propValue);
    return true;
  } else if (propKey == "textAlignVertical") {
    SetTextAlignVertical(propValue);
    return true;
  } else if (propKey == "placeholder") {
    this->placeholder = HRValueUtils::GetString(propValue);
    return true;
  } else if (propKey == "placeholderTextColor") {
    this->placeholderTextColor = HRValueUtils::GetUint32(propValue);
    return true;
  } else if (propKey == "numberOfLines") {
    this->maxLines = HRValueUtils::GetInt32(propValue);
    return true;
  } else if (propKey == "keyboardType") {
    SetKeyBoardType(propValue);
    return true;
  } else if (propKey == "returnKeyType") {
    SetEntryKeyType(propValue);
    return true;
  } else if (propKey == "value") {
    this->value = HRValueUtils::GetString(propValue);
    return true;
  } else if (propKey == "changetext") {
    this->isListenChangeText = HRValueUtils::GetBool(propValue, false);    
    return true;
  } else if (propKey == "selectionchange") {
    this->isListenSelectionChange = HRValueUtils::GetBool(propValue, false);
    return true;
  } else if (propKey == "endediting") {
    this->isListenEndEditing = HRValueUtils::GetBool(propValue, false);
    return true;
  } else if (propKey == "focus") {
    this->isListenFocus = HRValueUtils::GetBool(propValue, false);
    return true;
  } else if (propKey == "blur") {
    this->isListenBlur = HRValueUtils::GetBool(propValue, false);
    return true;
  } else if (propKey == "keyboardwillshow") {
    this->isListenKeyboardWillShow = HRValueUtils::GetBool(propValue, false);
    return true;
  } else if (propKey == "keyboardwillhide") {
    this->isListenKeyboardWillHide = HRValueUtils::GetBool(propValue, false);
    return true;
  } else if (propKey == "contentSizeChange") {
    this->isListenContentSizeChange = HRValueUtils::GetBool(propValue, false);
    return true;
  }
  return BaseView::SetProp(propKey, propValue);
}

void TextInputView::OnSetPropsEnd(){
  FOOTSTONE_DLOG(INFO)<< __FUNCTION__;
  InitNode();
//   GetTextNode().SetCaretColor(this->caretColor);
  GetTextNode().SetFontWeight((ArkUI_FontWeight)this->fontWeight);
  GetTextNode().SetFontColor(this->color);
  GetTextNode().SetFontFamily(this->fontFamily);
  GetTextNode().SetFontSize(this->fontSize);
  GetTextNode().SetFontStyle((ArkUI_FontStyle)this->fontStyle);
  GetTextNode().SetMaxLength((int32_t)this->maxLength);
  GetTextNode().SetTextAlign((ArkUI_TextAlignment)this->textAlign);
  GetTextNode().SetTextAlignVertical((ArkUI_Alignment)this->textAlignVertical);
  GetTextNode().SetPlaceholder(this->placeholder);
  GetTextNode().SetPlaceholderColor(this->placeholderTextColor);
  GetTextNode().SetInputType((ArkUI_TextInputType)this->keyboardType);
  GetTextNode().SetEnterKeyType((ArkUI_EnterKeyType)this->returnKeyType);
  GetTextNode().SetTextContent(this->value);
  if(this->multiline){
    GetTextNode().SetMaxLines(this->maxLines);
  }
  return BaseView::OnSetPropsEnd();
}

void TextInputView::SetFontWeight(const HippyValue &propValue) {
  std::string font = HRValueUtils::GetString(propValue);
  this->fontWeight = HRTextConvertUtils::FontWeightToArk(font);
}

void TextInputView::SetTextAlign(const HippyValue &propValue) {
  this->textAlign = ArkUI_TextAlignment::ARKUI_TEXT_ALIGNMENT_START;
  std::string align = HRValueUtils::GetString(propValue);
  if(align == "center") {
    this->textAlign = ArkUI_TextAlignment::ARKUI_TEXT_ALIGNMENT_CENTER;
  } else if (align == "right") {
    this->textAlign = ArkUI_TextAlignment::ARKUI_TEXT_ALIGNMENT_END;
  }
}

void TextInputView::SetTextAlignVertical(const HippyValue &propValue){
  this->textAlignVertical = ArkUI_Alignment::ARKUI_ALIGNMENT_CENTER;
  std::string align = HRValueUtils::GetString(propValue);
  if (align == "top") {
    this->textAlign = ArkUI_Alignment::ARKUI_ALIGNMENT_TOP;
  } else if (align == "bottom") {
    this->textAlign = ArkUI_Alignment::ARKUI_ALIGNMENT_BOTTOM;
  }
}

void TextInputView::SetKeyBoardType(const HippyValue &propValue){
  this->keyboardType = ArkUI_TextInputType::ARKUI_TEXTINPUT_TYPE_NORMAL;
  std::string type = HRValueUtils::GetString(propValue);
  if(type == "numeric") {
    this->keyboardType = ArkUI_TextInputType::ARKUI_TEXTINPUT_TYPE_NUMBER;
  } else if (type == "password") {
    this->keyboardType = ArkUI_TextInputType::ARKUI_TEXTINPUT_TYPE_PASSWORD;
  } else if (type == "email") {
    this->keyboardType = ArkUI_TextInputType::ARKUI_TEXTINPUT_TYPE_EMAIL;
  } else if (type == "phone-pad") {
    this->keyboardType = ArkUI_TextInputType::ARKUI_TEXTINPUT_TYPE_PHONE_NUMBER;
  }
}

void TextInputView::SetEntryKeyType(const HippyValue &propValue){
  this->returnKeyType = ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_DONE;
  std::string type = HRValueUtils::GetString(propValue);
  if (type == "go") {
    this->returnKeyType = ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_GO;
  } else if (type == "next") {
    this->returnKeyType = ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_NEXT;
  } else if (type == "search") {
    this->returnKeyType = ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_SEARCH;
  } else if (type == "send") {
    this->returnKeyType = ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_SEND;
  } else if (type == "previous") {
    this->returnKeyType = ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_PREVIOUS;
  }
}

void TextInputView::Call(const std::string &method, const std::vector<HippyValue> params,
                   std::function<void(const HippyValue &result)> callback){
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" method = "<<method; 

  if (method == "focusTextInput") {
    FocusTextInput(params);
  } else if (method == "blurTextInput") {
    BlurTextInput(params);
  } else if (method == "hideInputMethod") {
    HideInputMethod(params);
  } else if (method == "clear") {
    HippyValueArrayType array;
    SetText(array);
  } else if (method == "setValue") {
    SetText(params);
  } else if (method == "getValue" && callback) {
    HippyValueObjectType result;
    result["text"] = HippyValue(this->value);
    const HippyValue obj = HippyValue(result);
    callback(obj);
  } else if (method == "isFocused" && callback) {
    HippyValueObjectType result;
    result["value"] = HippyValue(this->focus);
    const HippyValue obj = HippyValue(result);
    callback(obj);
  } else {
    BaseView::Call(method, params, callback);
  }
}

void TextInputView::SetText(const HippyValueArrayType &param){
  if(param.size() == 0){
    this->value = "";
  }else{
    std::string str = HRValueUtils::GetString(param[0]);
    this->value = HRValueUtils::GetString(param[0]);
    int32_t pos = (int32_t)str.length();
    GetTextNode().SetTextSelection(pos, pos);
  }
}

void TextInputView::FocusTextInput(const HippyValueArrayType &param){
  GetTextNode().SetFocusStatus(true);  
}

void TextInputView::BlurTextInput(const HippyValueArrayType &param){
  GetTextNode().SetTextEditing(false);
}

void TextInputView::HideInputMethod(const HippyValueArrayType &param){
  GetTextNode().SetTextEditing(false);
}

void TextInputView::UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding){
  BaseView::UpdateRenderViewFrame(frame, padding);
  GetTextNode().SetPadding(padding.paddingTop, padding.paddingRight, padding.paddingBottom, padding.paddingLeft);
  HRSize size(frame.width,frame.height);
  GetTextNode().SetSize(size);
}

void TextInputView::OnChange(std::string text){
  if(this->value == text)
    return;
  this->value = text;
  if(this->isListenChangeText){
    HippyValueObjectType params;
    params["text"] = HippyValue(text);
    const std::shared_ptr<HippyValue> obj = std::make_shared<HippyValue>(params);
    HREventUtils::SendComponentEvent(ctx_, tag_, "changetext", obj);
  }  
  if(this->isListenContentSizeChange){
     HRRect rect = GetTextNode().GetTextContentRect();
     if(this->previousContentWidth != rect.width || this->previousContentHeight != rect.height){
        this->previousContentWidth = rect.width; 
        this->previousContentHeight = rect.height;
        HippyValueObjectType contentSize;
        contentSize["width"] = rect.width;
        contentSize["height"] = rect.height;
        HippyValueObjectType eventData;
        eventData["contentSize"] = contentSize;
        const std::shared_ptr<HippyValue> obj = std::make_shared<HippyValue>(eventData);
        HREventUtils::SendComponentEvent(ctx_, tag_, "onContentSizeChange", obj);
     }
  }
}

void TextInputView::OnBlur() {
  this->focus = false;  
  if (!this->isListenBlur)
    return;
  HippyValueObjectType params;
  params["text"] = HippyValue(this->value);
  const std::shared_ptr<HippyValue> obj = std::make_shared<HippyValue>(params);
  HREventUtils::SendComponentEvent(ctx_, tag_, "blur", obj);
}

void TextInputView::OnFocus() { 
    this->focus = true;
    if(!this->isListenFocus) 
        return;
    HippyValueObjectType params;
    params["text"] = HippyValue(this->value);
    const std::shared_ptr<HippyValue> obj = std::make_shared<HippyValue>(params);
    HREventUtils::SendComponentEvent(ctx_, tag_, "focus", obj);
}

void TextInputView::OnSubmit() {
    OnEventEndEditing((ArkUI_EnterKeyType)this->returnKeyType);
}

void TextInputView::OnPaste() {

}

void TextInputView::OnTextSelectionChange(int32_t location, int32_t length) { 
  if(!this->isListenSelectionChange)
      return;
  HippyValueObjectType selection;
  selection["start"] = HippyValue(location);
  selection["end"] = HippyValue(location + length);
  HippyValueObjectType params;
  params["selection"] = HippyValue(selection);
  const std::shared_ptr<HippyValue> obj = std::make_shared<HippyValue>(params);
  HREventUtils::SendComponentEvent(ctx_, tag_, "selectionchange", obj);
}

void TextInputView::OnEventEndEditing(ArkUI_EnterKeyType enterKeyType){
  if(!this->isListenEndEditing)
      return;
  HippyValueObjectType params;
  params["text"] = this->value;
  const std::shared_ptr<HippyValue> obj = std::make_shared<HippyValue>(params);
  HREventUtils::SendComponentEvent(ctx_, tag_, "endediting", obj);
  
  params["actionCode"] = enterKeyType;
  switch (enterKeyType) { 
    case ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_GO:
      params["actionName"] = "go";
      break;
    case ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_NEXT:
      params["actionName"] = "next";
      break;
    case ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_PREVIOUS:
      params["actionName"] = "previous";
      break;
    case ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_SEARCH:
      params["actionName"] = "search";
      break;
    case ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_SEND:
      params["actionName"] = "send";
      break;
    case ArkUI_EnterKeyType::ARKUI_ENTER_KEY_TYPE_DONE:
      params["actionName"] = "done";
      break;
    default:
      params["actionName"] = "done";
      break;
  }
  const std::shared_ptr<HippyValue> objAction = std::make_shared<HippyValue>(params);
  HREventUtils::SendComponentEvent(ctx_, tag_, "onEditorAction", objAction);
}

} // namespace native
} // namespace render
} // namespace hippy
