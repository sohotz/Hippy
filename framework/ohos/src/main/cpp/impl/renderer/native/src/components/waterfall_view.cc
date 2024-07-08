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

#include "renderer/components/waterfall_view.h"
#include "renderer/utils/hr_value_utils.h"
#include "renderer/native_render_provider.h"
#include "renderer/components/waterfall_item_view.h"

namespace hippy {
inline namespace render {
inline namespace native {

WaterfallView::WaterfallView(std::shared_ptr<NativeRenderContext> &ctx) : BaseView(ctx) {
  stackNode_.RegisterAppearEvent();
  stackNode_.RegisterDisappearEvent();
  stackNode_.SetArkUINodeDelegate(this);    
  flowNode_.SetNodeDelegate(this);    
  refreshNode_.SetNodeDelegate(this);
    
  stackNode_.AddChild(colNode_);
  colNode_.AddChild(refreshNode_);
  refreshNode_.AddChild(rowNode_);  
  rowNode_.AddChild(scrollNode_);
  scrollNode_.AddChild(colInnerNode_);
  colInnerNode_.AddChild(flowNode_);
}

WaterfallView::~WaterfallView() {
  stackNode_.UnregisterAppearEvent();
  stackNode_.UnregisterDisappearEvent();
  if (!children_.empty()) {
    for (const auto &child : children_) {
      GetLocalRootArkUINode().RemoveChild(child->GetLocalRootArkUINode());
    }
    children_.clear();
  }
//  GetLocalRootArkUINode().RemoveChild(flowNode_);  
}

ArkUINode &WaterfallView::GetLocalRootArkUINode() { return stackNode_;}

bool WaterfallView::SetProp(const std::string &propKey, const HippyValue &propValue) {
  if (propKey == "bounces") {
    int32_t type = HRValueUtils::GetInt32(propValue, 0);
    if(type == 0)    
      this->edgeEffect_ = ArkUI_EdgeEffect::ARKUI_EDGE_EFFECT_SPRING;
    else if (type == 1)
      this->edgeEffect_ = ArkUI_EdgeEffect::ARKUI_EDGE_EFFECT_FADE;
    else if(type == 2)
      this->edgeEffect_ = ArkUI_EdgeEffect::ARKUI_EDGE_EFFECT_NONE;    
    return true;        
  } else if (propKey == "contentInset") {
    HippyValueObjectType data;
    if(propValue.ToObject(data)){
       this->padding_.paddingTop = HRValueUtils::GetFloat(data["top"]);
       this->padding_.paddingBottom = HRValueUtils::GetFloat(data["bottom"]);
       this->padding_.paddingLeft = HRValueUtils::GetFloat(data["left"]);
       this->padding_.paddingRight = HRValueUtils::GetFloat(data["right"]);   
    }else{
       this->padding_= HRPadding(0,0,0,0);   
    }
    return true;            
  } else if (propKey == "scrollEventThrottle") {
    this->scrollEventThrottle_ = HRValueUtils::GetFloat(propValue, 30);
    return true;  
  } else if (propKey == "preloadItemNumber") {
    this->preloadItemNumber_ = HRValueUtils::GetInt32(propValue);
    return true;  
  } else if (propKey == "interItemSpacing") {
    this->interItemSpacing_ = HRValueUtils::GetFloat(propValue,0);
    return true;  
  } else if (propKey == "columnSpacing") {
    this->columnSpacing_ = HRValueUtils::GetFloat(propValue, 0);
    return true;  
  } else if (propKey == "numberOfColumns") {
    int  columns = (int)HRValueUtils::GetDouble(propValue,2);
    this->columnsTemplate_ = "1fr";
    for(int i = 1 ; i < columns ; i++){
       this->columnsTemplate_ += " 1fr";    
    }
    return true;  
  } else if (propKey == "scroll") {
    scrollEnable_ = HRValueUtils::GetBool(propValue, false);
    return true;  
  } else if (propKey == "endreached") {
    return true;  
  }
  return BaseView::SetProp(propKey, propValue);
}

void WaterfallView::OnSetPropsEnd(){
  return BaseView::OnSetPropsEnd();  
}

void WaterfallView::Init() {
  BaseView::Init();
  auto weak_view = weak_from_this();
  auto render = NativeRenderProvider(GetCtx()->GetInstanceId(),"");
  this->cbID = render.GetNativeRenderImpl()->AddEndBatchCallback(GetCtx()->GetRootId(), [weak_view]() {
    auto view = weak_view.lock();
    if (view) {
      auto waterfallView = std::dynamic_pointer_cast<WaterfallView>(view); 
      waterfallView->build();  
    }
  });
}

void WaterfallView::build() {
  scrollNode_.SetNestedScroll(ARKUI_SCROLL_NESTED_MODE_PARENT_FIRST,ARKUI_SCROLL_NESTED_MODE_SELF_FIRST);
  scrollNode_.SetScrollEventThrottle(this->scrollEventThrottle_);  
  scrollNode_.SetScrollBarDisplayMode(ARKUI_SCROLL_BAR_DISPLAY_MODE_OFF);
  scrollNode_.SetHeightPercent(1.0);  
  scrollNode_.SetScrollEnableInteraction(true);  
  scrollNode_.SetScrollEdgeEffect(this->edgeEffect_);
  rowNode_.SetWidthPercent(1.0);
  rowNode_.SetAlignItem(ArkUI_VerticalAlignment::ARKUI_VERTICAL_ALIGNMENT_BOTTOM);
  colInnerNode_.SetPadding(this->padding_.paddingTop,this->padding_.paddingRight,this->padding_.paddingBottom,this->padding_.paddingLeft);
  flowNode_.SetWidthPercent(1.0);
  flowNode_.SetScrollEdgeEffect(this->edgeEffect_);
  flowNode_.SetColumnGap(this->columnSpacing_);
  flowNode_.SetRowGap(this->interItemSpacing_);
//  flowNode_.SetColumnsTemplate(this->columnsTemplate_); //TODO issue ,cannot list multi line
  flowNode_.SetCachedCount(4);  
  flowNode_.SetScrollEnableInteraction(true);  
  flowNode_.SetNestedScroll(ARKUI_SCROLL_NESTED_MODE_PARENT_FIRST, ARKUI_SCROLL_NESTED_MODE_SELF_FIRST);
  flowNode_.SetScrollBarDisplayMode(ARKUI_SCROLL_BAR_DISPLAY_MODE_OFF);  
  if(footerView)
    flowNode_.SetFooter(footerView->GetLocalRootArkUINode().GetArkUINodeHandle());
}

void WaterfallView::HandleOnChildrenUpdated() {
//    build();
  scrollNode_.SetScrollBarDisplayMode(ARKUI_SCROLL_BAR_DISPLAY_MODE_OFF);
  flowNode_.SetScrollBarDisplayMode(ARKUI_SCROLL_BAR_DISPLAY_MODE_OFF); 
  colInnerNode_.SetPadding(this->padding_.paddingTop,this->padding_.paddingRight,this->padding_.paddingBottom,this->padding_.paddingLeft);
    
  if(headerView){
    headerViewHeight_ = headerView->GetLocalRootArkUINode().GetSize().height;
    headerView->GetLocalRootArkUINode().SetPosition(HRPosition(0,-headerViewHeight_));
    HRPosition setPos(0,0);
    for(uint64_t i = 0 ; i < children_.size();i++){
      if(children_[i]->GetViewType() == "WaterfallItem"){
        auto flowItem = std::dynamic_pointer_cast<WaterfallItemView>(children_[i]);
        setPos = flowItem->GetLocalRootArkUINode().GetPostion();
        setPos.y -= headerViewHeight_;  
        flowItem->GetLocalRootArkUINode().SetPosition(setPos);
      }
    }
  }
}

void WaterfallView::OnChildInserted(std::shared_ptr<BaseView> const &childView, int32_t index) {
  if(childView->GetViewType() == "PullHeaderView") {
    this->headerView = std::dynamic_pointer_cast<PullHeaderView>(childView);
    rowNode_.InsertChild(childView->GetLocalRootArkUINode(),0);
  } else if(childView->GetViewType() == "PullFooterView") {
    this->footerView = std::dynamic_pointer_cast<PullFooterView>(childView);
    flowNode_.InsertChild(childView->GetLocalRootArkUINode(),0);
  } else if(childView->GetViewType() == "WaterfallItem"){
    auto flowItem = std::dynamic_pointer_cast<WaterfallItemView>(childView);
    flowItem->GetLocalRootArkUINode().SetNodeDelegate(this);
    flowItem->GetLocalRootArkUINode().SetItemIndex(index);
    flowItem->GetLocalRootArkUINode().SetBorderWidth(1, 1, 1, 1);    
    flowItem->GetLocalRootArkUINode().SetBorderColor(0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000);  
    flowNode_.AddChild(childView->GetLocalRootArkUINode());
  } else {
    FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" new child index = "<<index;    
  }
}

void WaterfallView::OnChildRemoved(std::shared_ptr<BaseView> const &childView, int32_t index) {
  BaseView::OnChildRemoved(childView, index);
  flowNode_.RemoveChild(childView->GetLocalRootArkUINode());
}

void WaterfallView::UpdateRenderViewFrame(const HRRect &frame, const HRPadding &padding) {
  BaseView::UpdateRenderViewFrame(frame, padding);
  width_ = frame.width;
  height_ = frame.height;
}

void WaterfallView::Call(const std::string &method, const std::vector<HippyValue> params,
              std::function<void(const HippyValue &result)> callback){
    FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" method = "<<method;  
    if(method == "scrollToIndex"){
        int32_t index = HRValueUtils::GetInt32(params[1]);
        bool animate = HRValueUtils::GetBool(params[2], false);
        flowNode_.ScrollToIndex(index, animate);
    } else if (method == "scrollToContentOffset") {
        
    } else if (method == "scrollToTop"){
        
    }
}

void WaterfallView::OnWaterFlowScrollIndex(int32_t firstIndex, int32_t lastIndex){
    this->lastIndex_ = lastIndex;
}

void WaterfallView::OnWaterFlowDidScroll(float_t offset, ArkUI_ScrollState state){

}

void WaterfallView::OnWaterFlowWillScroll(float_t offset, ArkUI_ScrollState state, int32_t source){

}

void WaterfallView::OnTouch(int32_t actionType){
//  FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" actionType = "<<actionType;  
  if (actionType == UI_TOUCH_EVENT_ACTION_DOWN || actionType == UI_TOUCH_EVENT_ACTION_MOVE) {
  } else if (actionType == UI_TOUCH_EVENT_ACTION_UP || actionType == UI_TOUCH_EVENT_ACTION_CANCEL) {
  }    
}

void WaterfallView::OnRefreshing(){
   FOOTSTONE_DLOG(INFO)<<__FUNCTION__;
  if(this->headerView){
    HREventUtils::SendComponentEvent(this->headerView->GetCtx(), this->headerView->GetTag(), HREventUtils::EVENT_PULL_HEADER_RELEASED, nullptr);    
  }else{
//    this->isRefreshing = false;    
  }
}

void WaterfallView::OnStateChange(int32_t state){
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" state = "<<state;
  //Inactive=0， Drag=1，OverDrag=2，Refresh=3，Done=4  TODO sdk will update
  if(this->headerView){
    if(state == 1){
      HippyValueObjectType params;
      params["contentOffset"] = HippyValue(this->headerView->GetHeight());
      const std::shared_ptr<HippyValue> obj = std::make_shared<HippyValue>(params);
      HREventUtils::SendComponentEvent(headerView->GetCtx(), headerView->GetTag(), HREventUtils::EVENT_PULL_HEADER_PULLING, obj);  
      if(!this->isRefreshing){
        this->refreshNode_.SetRefreshing(true);        
        this->isRefreshing = true;      
      }
    } else if(state == 4){
      HippyValueObjectType params;
      params["contentOffset"] = HippyValue(this->headerView->GetHeight());
      const std::shared_ptr<HippyValue> obj = std::make_shared<HippyValue>(params);
      HREventUtils::SendComponentEvent(headerView->GetCtx(), headerView->GetTag(), HREventUtils::EVENT_PULL_HEADER_RELEASED, obj);
      if(this->isRefreshing){
         this->refreshNode_.SetRefreshing(false);
         this->isRefreshing = false;       
      }
    }
  }
}

void WaterfallView::OnAppear() {
  HandleOnChildrenUpdated();
}

void WaterfallView::OnDisappear() {
  auto render = NativeRenderProvider(GetCtx()->GetInstanceId(),"");
  render.GetNativeRenderImpl()->RemoveEndBatchCallback(GetCtx()->GetRootId(), this->cbID);
}  

void WaterfallView::OnItemVisibleAreaChange(int32_t index, bool isVisible, float currentRatio){
//  FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" index = "<<index<<" isvisible = "<<isVisible<<" lastIndex_ = "<<lastIndex_;  
  if(index == this->lastIndex_ && footerView){
     if(isVisible)  {//reach end
        OnReachEnd();
     } else{
            
     }
  }
}

void WaterfallView::OnHeadRefreshFinish(int32_t delay){
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__<<" delay = "<<delay;
  if(delay > 0 ){
    //TODO setTimeout(delay)        
  } else{
    this->isRefreshing = false;
    this->refreshNode_.SetRefreshing(false);          
  }
}

void WaterfallView::OnHeadRefresh(){
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__;
//  this->isRefreshing = true;
//  this->refreshNode_.SetRefreshing(true);  
}

void WaterfallView::SendOnReachedEvent(){
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__; 
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_RECYCLER_END_REACHED, nullptr);            
  HREventUtils::SendComponentEvent(ctx_, tag_, HREventUtils::EVENT_RECYCLER_LOAD_MORE, nullptr);       
}

void WaterfallView::OnScrollStart() {
   FOOTSTONE_DLOG(INFO)<<__FUNCTION__;
}

void WaterfallView::OnScrollStop() {
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__;
}

void WaterfallView::OnReachStart() {
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__;
}

void WaterfallView::OnReachEnd() {
  FOOTSTONE_DLOG(INFO)<<__FUNCTION__;
  SendOnReachedEvent();
}


} // namespace native
} // namespace render
} // namespace hippy
