#pragma once

#include <map>
#include <string>
#include <vector>

#include <native_drawing/drawing_color.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_text_declaration.h>
#include <native_drawing/drawing_types.h>
#include <native_drawing/drawing_text_typography.h>

struct OhMeasureResult {
    double width;
    double height;
};

class OhMeasureText {
public:
    OhMeasureText();
    ~OhMeasureText();

    void StartMeasure(std::map<std::string, std::string> propMap);
    void AddText(std::map<std::string, std::string> propMap);
    void AddImage(std::map<std::string, std::string> propMap);
    OhMeasureResult EndMeasure(std::map<std::string, std::string> propMap, int width, int widthMode, int height,
                               int heightMode, float density);

    static std::vector<std::string> textPropsOnly;   // 只有text有，span没有的属性
    static std::vector<std::string> textMarginProps; // 和边框相关的，在最后参与计算
    static std::vector<std::string> spanDropProps;   // 在OH_Drawing里面没有的属性，不知如何处理
    static std::vector<std::string> textSpanProps;   // 已处理的TextSpan属性

private:
    OH_Drawing_TypographyStyle *typoStyle_;
    OH_Drawing_TypographyCreate *handler_;
    double minLineHeight_ = 0; // 子组件中只有ImageSpan，没有TextSpan时，Placeholder不能撑大高度，使用ImageSpan的高度
};