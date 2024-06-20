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

#include "renderer/components/modal_view.h"
#include "renderer/utils/hr_value_utils.h"
#include "renderer/utils/hr_event_utils.h"
#include "renderer/native_render_provider.h"

namespace hippy {
inline namespace render {
inline namespace native {
//const int DURATION = 200;

ModalView::ModalView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {
  GetLocalRootArkUINode().SetStackNodeDelegate(this);
  GetLocalRootArkUINode().RegisterAppearEvent();
  GetLocalRootArkUINode().RegisterDisappearEvent();
  GetLocalRootArkUINode().RegisterAreaChangeEvent();
}

ModalView::~ModalView() {
  GetLocalRootArkUINode().UnregisterAppearEvent();
  GetLocalRootArkUINode().UnregisterDisappearEvent();
  GetLocalRootArkUINode().UnregisterAreaChangeEvent();
}

StackNode &ModalView::GetLocalRootArkUINode() { return stackNode_; }

bool ModalView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if(propKey == "animationType")
    FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" propkey = "<<propKey<<" value = "<<HRValueUtils::GetString(propValue);    
  else  
    FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" propkey = "<<propKey<<" value = "<<HRValueUtils::GetBool(propValue, true);
  if(propKey == "transparent"){
    this->transparent = HRValueUtils::GetBool(propValue, true);
  } else if(propKey == "animationType"){
    this->animationType = HRValueUtils::GetString(propValue);
  } else if(propKey == "darkStatusBarText"){
    this->darkStatusBarText = HRValueUtils::GetBool(propValue, false);
  } 
  return BaseView::SetProp(propKey, propValue);
}

void ModalView::OnSetPropsEnd(){
  BaseView::OnSetPropsEnd();
}

void ModalView::UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding){
//  BaseView::UpdateRenderViewFrame(frame, padding); size will change in OnAreaChange
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" frame("<<(int)frame.x<<","<<(int)frame.y<<","<<(int)frame.width<<","<<(int)frame.height<<")";
}

void ModalView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int index){
  BaseView::OnChildInserted(childView, index);
  if(childView){
     GetLocalRootArkUINode().InsertChild(childView->GetLocalRootArkUINode(), index);
  }
}

void ModalView::OnChildRemoved(std::shared_ptr<BaseView> const &childView){
  BaseView::OnChildRemoved(childView);
  if(childView)
    GetLocalRootArkUINode().RemoveChild(childView->GetLocalRootArkUINode());
}

void ModalView:: OnAppear(){
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__ ;
  if(this->transparent)
    dialog_.SetBackgroundColor(0x00000000);
  dialog_.EnableCustomAnimation(false);//TODO will add custom animation 
  dialog_.EnableCustomStyle(true);
  dialog_.SetAutoCancel(true);
  dialog_.SetContentAlignment(ArkUI_Alignment::ARKUI_ALIGNMENT_TOP_START, 0, 0);
  dialog_.SetCornerRadius(0, 0, 0, 0);
  dialog_.SetModalMode(true);
  dialog_.SetContent(GetLocalRootArkUINode().GetArkUINodeHandle());
  dialog_.Show();
  HREventUtils::SendComponentEvent(GetCtx(), GetTag(),HREventUtils::EVENT_MODAL_SHOW, nullptr);

  if(this->transparent)
    GetLocalRootArkUINode().SetBackgroundColor(0x00000000);
  GetLocalRootArkUINode().SetSizePercent(HRSize(1.f,1.f));
  GetLocalRootArkUINode().SetExpandSafeArea();//TODO will update when NODE_EXPAND_SAFE_AREA add in sdk  
}

void ModalView::OnDisappear(){
  dialog_.Close();
  HREventUtils::SendComponentEvent(GetCtx(), GetTag(),HREventUtils::EVENT_MODAL_REQUEST_CLOSE, nullptr);
}

void ModalView::OnAreaChange(ArkUI_NumberValue* data) {
  if(GetLocalRootArkUINode().GetTotalChildCount() == 0){
    FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" no child" ;    
    return;       
  }
  float_t width = data[6].f32;
  float_t height = data[7].f32;  
  auto render = NativeRenderProvider(GetCtx()->GetInstanceId());
  render.OnSize2(GetCtx()->GetRootId(), GetTag(), width, height, true);
}

} // namespace native
} // namespace render
} // namespace hippy
