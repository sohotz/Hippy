#pragma once

#include "renderer/arkui/arkui_node.h"

namespace hippy {
inline namespace render {
inline namespace native {

class WaterFlowItemNode : public ArkUINode {
protected:
  WaterFlowItemNodeDelegate *WaterFlowItemNodeDelegate_ = nullptr;
  int32_t itemIndex_ = -1;

public:
  WaterFlowItemNode();
  ~WaterFlowItemNode();

  void AddChild(ArkUINode &child);
  void InsertChild(ArkUINode &child, int32_t index);
  void RemoveChild(ArkUINode &child);

  void OnNodeEvent(ArkUI_NodeEvent *event) override;
  void SetNodeDelegate(WaterFlowItemNodeDelegate *nodeDelegate);
  
  void SetItemIndex(int32_t index) { itemIndex_ = index; }
};

} // namespace native
} // namespace render
} // namespace hippy
