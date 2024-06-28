#pragma once

#include <map>
#include <string>
#include <vector>

#include <native_drawing/drawing_color.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_text_declaration.h>
#include <native_drawing/drawing_types.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_drawing/drawing_register_font.h>

// Note: Do not open normally, it impacts performance.
// #define MEASURE_TEXT_CHECK_PROP

struct OhImageSpanHolder {
    double width;
    double height;
    OH_Drawing_PlaceholderVerticalAlignment alignment;
    double top = 0;

    double marginTop = 0;
    double marginBottom = 0;
};

struct OhImageSpanPos {
    double x;
    double y;
};

struct OhMeasureResult {
    double width;
    double height;
    std::vector<OhImageSpanPos> spanPos; // 指定imageSpan的位置
};

class OhMeasureText {
public:
    OhMeasureText();
    ~OhMeasureText();

    void StartMeasure(std::map<std::string, std::string> &propMap);
    void AddText(std::map<std::string, std::string> &propMap);
    void AddImage(std::map<std::string, std::string> &propMap);
    OhMeasureResult EndMeasure(int width, int widthMode, int height, int heightMode, float density);

    static void RegisterFont(std::string familyName, std::string familySrc);

private:
    static std::map<std::string, std::string> fontFamilyList_;

    bool HasProp(std::map<std::string, std::string> &propMap, const char *s);

#ifdef MEASURE_TEXT_CHECK_PROP
    void StartCollectProp();
    void CheckUnusedProp(const char *tag, std::map<std::string, std::string> &propMap);
    std::vector<std::string> usedProp_;
#endif

    OH_Drawing_TypographyStyle *typoStyle_;
    OH_Drawing_TypographyCreate *handler_;
    OH_Drawing_FontCollection *fontCollection_;
    double CalcSpanPostion(OH_Drawing_Typography *typography, OhMeasureResult &ret);
    std::vector<OhImageSpanHolder> imageSpans_;
    double lineHeight_ = 0; // 外部指定的行高，最高优先级
    double minLineHeight_ = 0; // 子组件中只有ImageSpan，没有TextSpan时，Placeholder不能撑大高度，使用ImageSpan的高度
    double paddingTop_ = 0;
    double paddingBottom_ = 0;
    double paddingLeft_ = 0;
    double paddingRight_ = 0;
};