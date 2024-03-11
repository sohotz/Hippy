#include "oh_napi/oh_measure_text.h"
#include "footstone/logging.h"


// Text父节点独有的属性
const static std::vector<std::string> propsStart = {"textAlign", "numberOfLines", "breakStrategy", "ellipsizeMode"};

// 已处理的TextSpan属性:
const static std::vector<std::string> propsSpan = {
    "text",       "color",         "fontSize",           "fontWeight",          "fontStyle",
    "fontFamily", "letterSpacing", "textDecorationLine", "textDecorationStyle", "textDecorationColor",
};

// 已处理的边框:
const static std::vector<std::string> propsEnd = {
    "marginLeft", "marginRight", "marginTop", "marginBottom", "margin", "lineHeight",
};

void CheckProps(std::vector<std::string> vec, std::map<std::string, std::string> propMap) {
    for (auto it = propMap.begin(); it != propMap.end(); it++) {
        if (std::find(vec.begin(), vec.end(), it->first) == vec.end()) {
            FOOTSTONE_DLOG(WARNING) << "Measure Text 未处理属性：" << it->first;
        }
    }
}

OhMeasureText::OhMeasureText() {}

OhMeasureText::~OhMeasureText() {}

void OhMeasureText::StartMeasure(std::map<std::string, std::string> propMap) {
    typoStyle_ = OH_Drawing_CreateTypographyStyle();
    OH_Drawing_SetTypographyTextDirection(typoStyle_, TEXT_DIRECTION_LTR); // 从左向右排版

    int textAlign = TEXT_ALIGN_START;
    if (propMap.find("textAlign") != propMap.end()) {
        if (propMap["textAlign"] == "center") {
            textAlign = TEXT_ALIGN_CENTER;
        } else if (propMap["textAlign"] == "end") {
            textAlign = TEXT_ALIGN_END;
        } else if (propMap["textAlign"] == "justify") {
            textAlign = TEXT_ALIGN_JUSTIFY;
        }
    }
    OH_Drawing_SetTypographyTextAlign(typoStyle_, textAlign); // ok8        textAlign

    int maxLines = 100000;
    if (propMap.find("numberOfLines") != propMap.end()) {
        maxLines = std::stoi(propMap["numberOfLines"]);
    }
    OH_Drawing_SetTypographyTextMaxLines(typoStyle_, maxLines); // okA        maxLines

    if (propMap.find("breakStrategy") != propMap.end()) {
        OH_Drawing_BreakStrategy bs = BREAK_STRATEGY_GREEDY;
        if (propMap["breakStrategy"] == "high_quality") {
            bs = BREAK_STRATEGY_HIGH_QUALITY;
        } else if (propMap["breakStrategy"] == "balanced") {
            bs = BREAK_STRATEGY_BALANCED;
        }
        OH_Drawing_SetTypographyTextBreakStrategy(typoStyle_, bs);
    }

    if (propMap.find("ellipsizeMode") != propMap.end()) {
        OH_Drawing_EllipsisModal em = ELLIPSIS_MODAL_TAIL;
        if (propMap["ellipsizeMode"] == "head") {
            em = ELLIPSIS_MODAL_HEAD;
        } else if (propMap["ellipsizeMode"] == "middle") {
            em = ELLIPSIS_MODAL_MIDDLE;
        }
        OH_Drawing_SetTypographyTextEllipsisModal(typoStyle_, em);
    }

    handler_ = OH_Drawing_CreateTypographyHandler(typoStyle_, OH_Drawing_CreateFontCollection());

    CheckProps(propsStart, propMap);
}

void OhMeasureText::AddText(std::map<std::string, std::string> propMap) {
    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
    if (propMap.find("color") != propMap.end()) {
        unsigned long color = std::stoul(propMap["color"]);
        OH_Drawing_SetTextStyleColor(txtStyle, (uint32_t)color);
    }
    double fontSize = 18; // todo 默认的fontSize是多少？
    if (propMap.find("fontSize") != propMap.end()) {
        fontSize = std::stod(propMap["fontSize"]);
    }
    OH_Drawing_SetTextStyleFontSize(txtStyle, fontSize); // ok3 fontSize
    if (propMap.find("fontWeight") != propMap.end()) {
        int fontWeight = FONT_WEIGHT_400;
        if (propMap["fontWeight"] == "bold") {
            fontWeight = FONT_WEIGHT_700;
        }                                                        // todo...
        OH_Drawing_SetTextStyleFontWeight(txtStyle, fontWeight); // ok5 fontWeight
    }
    OH_Drawing_SetTextStyleBaseLine(txtStyle, TEXT_BASELINE_ALPHABETIC); // todoC baselineOffset
    if (propMap.find("textDecorationLine") != propMap.end()) {
        OH_Drawing_TextDecoration td = TEXT_DECORATION_NONE;
        if (propMap["textDecorationLine"] == "underline") {
            td = TEXT_DECORATION_UNDERLINE;
        } else if (propMap["textDecorationLine"] == "line-through") {
            td = TEXT_DECORATION_LINE_THROUGH;
        } else if (propMap["textDecorationLine"] == "overline") {
            td = TEXT_DECORATION_OVERLINE;
        }
        OH_Drawing_SetTextStyleDecoration(txtStyle, td);
    }
    if (propMap.find("textDecorationColor") != propMap.end()) {
        unsigned long color = std::stoul(propMap["textDecorationColor"]);
        OH_Drawing_SetTextStyleDecorationColor(txtStyle, (uint32_t)color);
    }
    if (propMap.find("textDecorationStyle") != propMap.end()) {
        OH_Drawing_TextDecorationStyle ds = TEXT_DECORATION_STYLE_SOLID;
        if (propMap["textDecorationStyle"] == "dotted") {
            ds = TEXT_DECORATION_STYLE_DOTTED;
        } else if (propMap["textDecorationStyle"] == "double") {
            ds = TEXT_DECORATION_STYLE_DOUBLE;
        } else if (propMap["textDecorationStyle"] == "dashed") {
            ds = TEXT_DECORATION_STYLE_DASHED;
        } else if (propMap["textDecorationStyle"] == "wavy") {
            ds = TEXT_DECORATION_STYLE_WAVY;
        }
        OH_Drawing_SetTextStyleDecorationStyle(txtStyle, ds);
    }
    OH_Drawing_SetTextStyleFontHeight(txtStyle, 1); // todo ?
    if (propMap.find("fontFamily") != propMap.end()) {
        const char *fontFamilies[] = {propMap["fontFamily"].c_str()};
        OH_Drawing_SetTextStyleFontFamilies(txtStyle, 1, fontFamilies); // ok6 fontFamily
    }
    int fontStyle = FONT_STYLE_NORMAL;
    if (propMap.find("fontStyle") != propMap.end() && propMap["fontStyle"] == "italic") {
        fontStyle = FONT_STYLE_ITALIC;
    }
    OH_Drawing_SetTextStyleFontStyle(txtStyle, fontStyle); // ok4 fontStyle
    OH_Drawing_SetTextStyleLocale(txtStyle, "en");
    // OH_Drawing_SetTextStyleDecorationStyle(txtStyle, );
    // OH_Drawing_SetTextStyleDecorationThicknessScale(txtStyle, );
    if (propMap.find("letterSpacing") != propMap.end()) {
        double letterSpacing = std::stod(propMap["letterSpacing"]);
        OH_Drawing_SetTextStyleLetterSpacing(txtStyle, letterSpacing);
    }
    // OH_Drawing_SetTextStyleWordSpacing(txtStyle,);
    // OH_Drawing_SetTextStyleHalfLeading(txtStyle, );
    // OH_Drawing_SetTextStyleEllipsis(txtStyle, );
    // OH_Drawing_SetTextStyleEllipsisModal(txtStyle, );
    // 将文本样式对象加入到handler中
    OH_Drawing_TypographyHandlerPushTextStyle(handler_, txtStyle);
    OH_Drawing_TypographyHandlerAddText(handler_, propMap["text"].c_str()); // ok1 textContent
    OH_Drawing_TypographyHandlerPopTextStyle(handler_);
    OH_Drawing_DestroyTextStyle(txtStyle);

    CheckProps(propsSpan, propMap);
}

void OhMeasureText::AddImage(std::map<std::string, std::string> propMap) {
    OH_Drawing_PlaceholderSpan span;
    span.width = std::stod(propMap["width"]);
    span.height = std::stod(propMap["height"]);
    OH_Drawing_TypographyHandlerAddPlaceholder(handler_, &span);
}

OhMeasureResult OhMeasureText::EndMeasure(std::map<std::string, std::string> propMap, int width, int widthMode,
                                          int height, int heightMode, float density) {
    OhMeasureResult ret;

    {
        auto typography = OH_Drawing_CreateTypography(handler_);
        double maxWidth = width;
        OH_Drawing_TypographyLayout(typography, maxWidth); // todo2 constraintWidth

        ret.height = OH_Drawing_TypographyGetHeight(typography);     // 高度
        ret.width = OH_Drawing_TypographyGetLongestLine(typography); // 宽度
        OH_Drawing_DestroyTypography(typography);
    }
    OH_Drawing_DestroyTypographyHandler(handler_);
    OH_Drawing_DestroyTypographyStyle(typoStyle_);

    ret.width *= density;
    ret.height *= density;
    if (propMap.find("marginLeft") != propMap.end()) {
        ret.width += std::stod(propMap["marginLeft"]);
    }
    if (propMap.find("marginRight") != propMap.end()) {
        ret.width += std::stod(propMap["marginRight"]);
    }
    if (propMap.find("marginTop") != propMap.end()) {
        ret.height += std::stod(propMap["marginTop"]);
    }
    if (propMap.find("marginBottom") != propMap.end()) {
        ret.height += std::stod(propMap["marginBottom"]);
    }
    if (propMap.find("margin") != propMap.end()) {
        ret.width += std::stod(propMap["margin"]) * 2;
        ret.height += std::stod(propMap["margin"]) * 2;
    }
    if (propMap.find("lineHeight") != propMap.end()) {
        ret.height = std::stod(propMap["lineHeight"]) * density;
    }
    CheckProps(propsEnd, propMap);
    return ret;
}
