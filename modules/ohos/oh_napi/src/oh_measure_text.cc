#include "oh_napi/oh_measure_text.h"
#include "footstone/logging.h"

OhMeasureText::OhMeasureText() {}

OhMeasureText::~OhMeasureText() {}

bool OhMeasureText::HasProp(std::map<std::string, std::string> &propMap, const char *s) {
    if (propMap.find(s) == propMap.end()) {
        return false;
    }
#ifdef MEASURE_TEXT_CHECK_PROP
    usedProp_.push_back(s);
#endif
    return true;
}

#ifdef MEASURE_TEXT_CHECK_PROP
void OhMeasureText::StartCollectProp() { usedProp_.clear(); }

void OhMeasureText::CheckUnusedProp(const char *tag, std::map<std::string, std::string> &propMap) {
    for (auto it = propMap.begin(); it != propMap.end(); ++it) {
        if (std::find(usedProp_.begin(), usedProp_.end(), it->first) == usedProp_.end()) {
            FOOTSTONE_DLOG(WARNING) << "Measure Text " << tag << " unused prop : " << it->first << " : " << it->second;
        }
    }
}
#endif

std::map<std::string, std::string> OhMeasureText::fontFamilyList_ = {
    {"TTTGB", "/data/storage/el1/bundle/entry/resources/resfile/fonts/TTTGB.otf"}};
// todo 这里暂时写死了字体路径，实际用到了哪些字体需要ArkTS告知

void OhMeasureText::RegisterFont(std::string familyName, std::string familySrc) {
    fontFamilyList_[familyName] = familySrc;
}

void OhMeasureText::StartMeasure(std::map<std::string, std::string> &propMap) {
#ifdef MEASURE_TEXT_CHECK_PROP
    StartCollectProp();
#endif
    typoStyle_ = OH_Drawing_CreateTypographyStyle();
    OH_Drawing_SetTypographyTextDirection(typoStyle_, TEXT_DIRECTION_LTR); // 从左向右排版

    int textAlign = TEXT_ALIGN_START;
    if (HasProp(propMap, "textAlign")) {
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
    if (HasProp(propMap, "numberOfLines")) {
        maxLines = std::stoi(propMap["numberOfLines"]);
    }
    OH_Drawing_SetTypographyTextMaxLines(typoStyle_, maxLines); // okA        maxLines

    if (HasProp(propMap, "breakStrategy")) {
        OH_Drawing_BreakStrategy bs = BREAK_STRATEGY_GREEDY;
        if (propMap["breakStrategy"] == "high_quality") {
            bs = BREAK_STRATEGY_HIGH_QUALITY;
        } else if (propMap["breakStrategy"] == "balanced") {
            bs = BREAK_STRATEGY_BALANCED;
        }
        OH_Drawing_SetTypographyTextBreakStrategy(typoStyle_, bs);
    }

    if (HasProp(propMap, "ellipsizeMode")) {
        OH_Drawing_EllipsisModal em = ELLIPSIS_MODAL_TAIL;
        if (propMap["ellipsizeMode"] == "head") {
            em = ELLIPSIS_MODAL_HEAD;
        } else if (propMap["ellipsizeMode"] == "middle") {
            em = ELLIPSIS_MODAL_MIDDLE;
        }
        OH_Drawing_SetTypographyTextEllipsisModal(typoStyle_, em);
    }

    fontCollection_ = OH_Drawing_CreateFontCollection();
    if (HasProp(propMap, "fontFamily")) {
        if (fontFamilyList_.find(propMap["fontFamily"]) != fontFamilyList_.end()) {
            uint32_t ret = OH_Drawing_RegisterFont(fontCollection_, propMap["fontFamily"].c_str(),
                                                   fontFamilyList_[propMap["fontFamily"]].c_str());
            FOOTSTONE_DLOG(WARNING) << "Measure Text OH_Drawing_RegisterFont(" << propMap["fontFamily"] << ","
                                    << fontFamilyList_[propMap["fontFamily"]] << ") " << (ret == 0 ? "succ" : "fail");
        }
    }
    handler_ = OH_Drawing_CreateTypographyHandler(typoStyle_, fontCollection_);

    if (HasProp(propMap, "lineHeight")) {
        lineHeight_ = std::stod(propMap["lineHeight"]);
    }
    if (HasProp(propMap, "paddingVertical")) {
        paddingTop_ = std::stod(propMap["paddingVertical"]);
        paddingBottom_ = std::stod(propMap["paddingVertical"]);
    }
    if (HasProp(propMap, "paddingHorizontal")) {
        paddingLeft_ = std::stod(propMap["paddingHorizontal"]);
        paddingRight_ = std::stod(propMap["paddingHorizontal"]);
    }

#ifdef MEASURE_TEXT_CHECK_PROP
    const static std::vector<std::string> dropProp = {
        "text",        "backgroundColor", "color",           "marginLeft",       "marginRight",
        "marginTop",   "marginBottom",    "textShadowColor", "textShadowOffset", "borderColor",
        "borderWidth", "breakStrategy",   "textShadowRadius"};
    for (uint32_t i = 0; i < dropProp.size(); i++) {
        usedProp_.push_back(dropProp[i]);
    }
    CheckUnusedProp("Start", propMap);
#endif
}

void OhMeasureText::AddText(std::map<std::string, std::string> &propMap) {
#ifdef MEASURE_TEXT_CHECK_PROP
    StartCollectProp();
#endif
    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
    if (HasProp(propMap, "color")) {
        unsigned long color = std::stoul(propMap["color"]);
        OH_Drawing_SetTextStyleColor(txtStyle, (uint32_t)color);
    }
    double fontSize = 14; // 默认的fontSize是14，是否从系统读取？
    if (HasProp(propMap, "fontSize")) {
        fontSize = std::stod(propMap["fontSize"]);
    }
    OH_Drawing_SetTextStyleFontSize(txtStyle, fontSize); // ok3 fontSize
    if (HasProp(propMap, "fontWeight")) {
        int fontWeight = FONT_WEIGHT_400;
        if (propMap["fontWeight"] == "bold") {
            fontWeight = FONT_WEIGHT_700;
        }                                                        // todo...
        OH_Drawing_SetTextStyleFontWeight(txtStyle, fontWeight); // ok5 fontWeight
    }
    OH_Drawing_SetTextStyleBaseLine(txtStyle, TEXT_BASELINE_ALPHABETIC); // todoC baselineOffset

    if (HasProp(propMap, "textDecorationLine")) {
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
    if (HasProp(propMap, "textDecorationColor")) {
        unsigned long color = std::stoul(propMap["textDecorationColor"]);
        OH_Drawing_SetTextStyleDecorationColor(txtStyle, (uint32_t)color);
    }
    if (HasProp(propMap, "textDecorationStyle")) {
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

    OH_Drawing_SetTextStyleFontHeight(txtStyle, 1.25);
    // 猜测行高=fontSize*1.25，lineSpacingMultiplier，1.25是猜的，也可能是1.3或其他

    if (HasProp(propMap, "fontFamily")) {
        const char *fontFamilies[] = {propMap["fontFamily"].c_str()};
        OH_Drawing_SetTextStyleFontFamilies(txtStyle, 1, fontFamilies); // ok6 fontFamily
    }
    int fontStyle = FONT_STYLE_NORMAL;
    if (HasProp(propMap, "fontStyle") && propMap["fontStyle"] == "italic") {
        fontStyle = FONT_STYLE_ITALIC;
    }
    OH_Drawing_SetTextStyleFontStyle(txtStyle, fontStyle); // ok4 fontStyle
    OH_Drawing_SetTextStyleLocale(txtStyle, "en");
    // OH_Drawing_SetTextStyleDecorationStyle(txtStyle, );
    // OH_Drawing_SetTextStyleDecorationThicknessScale(txtStyle, );
    if (HasProp(propMap, "letterSpacing")) {
        double letterSpacing = std::stod(propMap["letterSpacing"]);
        OH_Drawing_SetTextStyleLetterSpacing(txtStyle, letterSpacing);
    }
    // OH_Drawing_SetTextStyleWordSpacing(txtStyle,);
    // OH_Drawing_SetTextStyleHalfLeading(txtStyle, );
    // OH_Drawing_SetTextStyleEllipsis(txtStyle, );
    // OH_Drawing_SetTextStyleEllipsisModal(txtStyle, );
    // 将文本样式对象加入到handler中
    OH_Drawing_TypographyHandlerPushTextStyle(handler_, txtStyle);
    if (HasProp(propMap, "text")) {
        OH_Drawing_TypographyHandlerAddText(handler_, propMap["text"].c_str()); // ok1 textContent
    }
    OH_Drawing_TypographyHandlerPopTextStyle(handler_);
    OH_Drawing_DestroyTextStyle(txtStyle);

#ifdef MEASURE_TEXT_CHECK_PROP
    const static std::vector<std::string> dropProp = {
        "backgroundColor",  "lineHeight",        "margin",          "marginLeft",      "marginRight",
        "marginTop",        "marginBottom",      "textAlign",       "textShadowColor", "textShadowOffset",
        "ellipsizeMode",    "numberOfLines",     "borderColor",     "borderWidth",     "breakStrategy",
        "textShadowRadius", "paddingHorizontal", "paddingVertical", "verticalAlign"};
    for (uint32_t i = 0; i < dropProp.size(); i++) {
        usedProp_.push_back(dropProp[i]);
    }
    CheckUnusedProp("Text", propMap);
#endif
}

void OhMeasureText::AddImage(std::map<std::string, std::string> &propMap) {
#ifdef MEASURE_TEXT_CHECK_PROP
    StartCollectProp();
#endif
    OH_Drawing_PlaceholderSpan span;
    if (HasProp(propMap, "width")) {
        span.width = std::stod(propMap["width"]);
    }
    if (HasProp(propMap, "height")) {
        span.height = std::stod(propMap["height"]);
    }
    span.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_CENTER_OF_ROW_BOX;
    OH_Drawing_TypographyHandlerAddPlaceholder(handler_, &span);
    if (minLineHeight_ < span.height) {
        minLineHeight_ = span.height;
    }

    OhImageSpanHolder spanH;
    spanH.width = span.width;
    spanH.height = span.height;
    spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_CENTER_OF_ROW_BOX;

    if (HasProp(propMap, "verticalAlign")) {
        if (propMap["verticalAlign"] == "top") {
            spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_TOP_OF_ROW_BOX;
        } else if (propMap["verticalAlign"] == "middle") {
            spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_CENTER_OF_ROW_BOX;
        } else if (propMap["verticalAlign"] == "baseline") {
            spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_OFFSET_AT_BASELINE;
        } else if (propMap["verticalAlign"] == "bottom") {
            spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_BOTTOM_OF_ROW_BOX;
        }
    }
    if (HasProp(propMap, "verticalAlignment")) {
        // todo 安卓已弃用，没文档，这个取值0,1,2,3，不知道什么含义
        // double va = std::stod(propMap["verticalAlignment"]);
        // if (va == 0) {
        //     spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_TOP_OF_ROW_BOX;
        // } else if (va == 1) {
        //     spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_CENTER_OF_ROW_BOX;
        // } else if (va == 2) {
        //     spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_OFFSET_AT_BASELINE;
        // } else if (va == 3) {
        //     spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_BOTTOM_OF_ROW_BOX;
        // }
        spanH.alignment = OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_ABOVE_BASELINE;
    }
    if (HasProp(propMap, "margin")) {
        spanH.marginTop = std::stod(propMap["margin"]);
        spanH.marginBottom = std::stod(propMap["margin"]);
    }
    if (HasProp(propMap, "top")) {
        spanH.top = std::stod(propMap["top"]);
    }

    imageSpans_.push_back(spanH);
#ifdef MEASURE_TEXT_CHECK_PROP
    const static std::vector<std::string> dropProp = {"backgroundColor", "src", "tintColor"};
    for (uint32_t i = 0; i < dropProp.size(); i++) {
        usedProp_.push_back(dropProp[i]);
    }
    CheckUnusedProp("Image", propMap);
#endif
}

double OhMeasureText::CalcSpanPostion(OH_Drawing_Typography *typography, OhMeasureResult &ret) {
    double baseLine = 0;
    size_t lineCount;
    std::vector<double> lineHeights;    // 真实每行高度
    std::vector<double> measureHeights; // 测得每行高度

    lineCount = OH_Drawing_TypographyGetLineCount(typography); // 总行数
    for (uint32_t i = 0; i < lineCount; i++) {                 // 获取每行行高
        // 当前行没有文本时，或者指定了lineHeight，baseLine获取的就不对
        // baseLine = OH_Drawing_TypographyGetAlphabeticBaseline(typography); // =h*11/16
        // baseLine = OH_Drawing_TypographyGetIdeographicBaseline(typography); //=h
        double h = OH_Drawing_TypographyGetLineHeight(typography, (int)i);
        measureHeights.push_back(h);
        if (lineHeight_ != 0) {
            lineHeights.push_back(lineHeight_);
        } else {
            lineHeights.push_back(h);
        }
    }

    OH_Drawing_TextBox *tb = OH_Drawing_TypographyGetRectsForPlaceholders(typography); // 获取每个imageSpan的区域
    size_t textBoxCount = OH_Drawing_GetSizeOfTextBox(tb); // 获取到的数量，应该和 imageSpans_ 一样多
    double bottom = lineHeights[0];
    for (uint32_t i = 0; i < textBoxCount; i++) { // i 对应到 imageSpans_ 下标
        float boxTop = OH_Drawing_GetTopFromTextBox(tb, (int)i);
        float boxBottom = OH_Drawing_GetBottomFromTextBox(tb, (int)i);
        float boxLeft = OH_Drawing_GetLeftFromTextBox(tb, (int)i);
        // float boxRight = OH_Drawing_GetRightFromTextBox(tb, (int)i);
        double top = 0;
        double measureTop = 0;
        OhImageSpanPos pos;
        pos.x = boxLeft;
        pos.y = boxTop;
        for (uint32_t j = 0; j < lineCount; j++) {
            bottom = top + lineHeights[j];
            double measureBottom = measureTop + measureHeights[j];
            if (measureTop <= boxTop && boxBottom <= measureBottom) { // 根据测得的top和bottom定位到span所在行
                baseLine = lineHeights[j] * 0.6;                      // todo 猜的比例
                switch (imageSpans_[i].alignment) {
                case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_TOP_OF_ROW_BOX:
                    pos.y = top + imageSpans_[i].marginTop;
                    break;
                case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_CENTER_OF_ROW_BOX:
                    pos.y = top + lineHeights[j] / 2 - imageSpans_[i].height / 2;
                    break;
                case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_BOTTOM_OF_ROW_BOX:
                    pos.y = bottom - imageSpans_[i].height - imageSpans_[i].marginBottom;
                    break;
                case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_OFFSET_AT_BASELINE:
                    // todo         这里和安卓不同，安卓没有 / 2
                    pos.y = top + baseLine - imageSpans_[i].height / 2 - imageSpans_[i].marginBottom;
                    break;
                case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_ABOVE_BASELINE:
                    // todo 有verticalAlignment属性时，不知如何处理
                    pos.y = top + lineHeights[j] * 0.7 - imageSpans_[i].height;
                    break;
                case OH_Drawing_PlaceholderVerticalAlignment::ALIGNMENT_BELOW_BASELINE:
                    pos.y = top + baseLine;
                    break;
                }
                pos.y += imageSpans_[i].top;
                if (pos.y < top) {
                    pos.y = top;
                }
                if (pos.y + imageSpans_[i].height > bottom) {
                    pos.y = bottom - imageSpans_[i].height;
                }
                break;
            }
            top = bottom;
            measureTop = measureBottom;
        }
        ret.spanPos.push_back(pos);
    }
    return bottom;
}

OhMeasureResult OhMeasureText::EndMeasure(int width, int widthMode, int height, int heightMode, float density) {
    OhMeasureResult ret;
    size_t lineCount;
    {
        auto typography = OH_Drawing_CreateTypography(handler_);
        double maxWidth = float(width) / density;
        OH_Drawing_TypographyLayout(typography, maxWidth); // todo2 constraintWidth

        // double realWidth = OH_Drawing_TypographyGetLongestLine(typography); // 实际有像素的宽度
        // ret.width = fmax(realWidth, maxWidth);                   // 宽度
        ret.width = OH_Drawing_TypographyGetLongestLine(typography);
        ret.height = OH_Drawing_TypographyGetHeight(typography); // 高度
        lineCount = OH_Drawing_TypographyGetLineCount(typography);

        double realHeight = CalcSpanPostion(typography, ret);
        ret.height = fmax(ret.height, realHeight);

        OH_Drawing_DestroyTypography(typography);
    }
    OH_Drawing_DestroyTypographyHandler(handler_);
    OH_Drawing_DestroyFontCollection(fontCollection_);
    OH_Drawing_DestroyTypographyStyle(typoStyle_);

    if (ret.height < minLineHeight_) {
        ret.height = minLineHeight_;
    }
    ret.width *= density;
    ret.height *= density;
    if (lineHeight_ != 0) {
        ret.height = lineHeight_ * density * (double)lineCount;
    }

    return ret;
}
