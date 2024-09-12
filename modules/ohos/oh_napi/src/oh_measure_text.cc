#include "oh_napi/oh_measure_text.h"
#include "footstone/logging.h"

static bool sIs_louzhu_str = false;

OhMeasureText::OhMeasureText() {}

OhMeasureText::~OhMeasureText() {}

OH_Drawing_FontWeight OhMeasureText::FontWeightToDrawing(std::string &str) {
  if (str.length() == 0 || str == "normal") {
    return FONT_WEIGHT_400;
  } else if (str == "bold") {
    return FONT_WEIGHT_700;
  } else {
    auto w = std::atoi(str.c_str());
    if (std::isnan(w) || w == 0) {
      return FONT_WEIGHT_400;
    }
    if (w < 200) {
      return FONT_WEIGHT_100;
    } else if (w < 300) {
      return FONT_WEIGHT_200;
    } else if (w < 400) {
      return FONT_WEIGHT_300;
    } else if (w < 500) {
      return FONT_WEIGHT_400;
    } else if (w < 600) {
      return FONT_WEIGHT_500;
    } else if (w < 700) {
      return FONT_WEIGHT_600;
    } else if (w < 800) {
      return FONT_WEIGHT_700;
    } else if (w < 900) {
      return FONT_WEIGHT_800;
    } else {
      return FONT_WEIGHT_900;
    }
  }
}

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
            FOOTSTONE_DLOG(WARNING) << "hippy text - measure " << tag << " unused prop: " << it->first << " : " << it->second;
        }
    }
}
#endif

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
    OH_Drawing_SetTypographyTextAlign(typoStyle_, textAlign);

    int maxLines = 100000;
    if (HasProp(propMap, "numberOfLines") && propMap["numberOfLines"].size() > 0) {
        maxLines = std::stoi(propMap["numberOfLines"]);
    }
    OH_Drawing_SetTypographyTextMaxLines(typoStyle_, maxLines);

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
        auto fontFamilyName = propMap["fontFamily"];
        if (fontFamilyList_.find(fontFamilyName) != fontFamilyList_.end()) {
            uint32_t ret = OH_Drawing_RegisterFont(fontCollection_, fontFamilyName.c_str(),
                                                   fontFamilyList_[fontFamilyName].c_str());
            FOOTSTONE_DLOG(WARNING) << "Measure Text OH_Drawing_RegisterFont(" << fontFamilyName << ","
                                    << fontFamilyList_[fontFamilyName] << ") " << (ret == 0 ? "succ" : "fail");
        } else {
            FOOTSTONE_LOG(ERROR) << "Measure Text OH_Drawing_RegisterFont not found font:" << fontFamilyName;
        }
    }
    handler_ = OH_Drawing_CreateTypographyHandler(typoStyle_, fontCollection_);
  
    if (HasProp(propMap, "lineHeight") && propMap["lineHeight"].size() > 0) {
        lineHeight_ = std::stod(propMap["lineHeight"]);
    }
    if (HasProp(propMap, "paddingVertical") && propMap["paddingVertical"].size() > 0) {
        auto paddingValue = std::stod(propMap["paddingVertical"]);
        paddingTop_ = paddingValue;
        paddingBottom_ = paddingValue;
    }
    if (HasProp(propMap, "paddingHorizontal") && propMap["paddingHorizontal"].size() > 0) {
        auto paddingValue = std::stod(propMap["paddingHorizontal"]);
        paddingLeft_ = paddingValue;
        paddingRight_ = paddingValue;
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
  
    if (HasProp(propMap, "lineHeight") && propMap["lineHeight"].size() > 0) {
        lineHeight_ = std::stod(propMap["lineHeight"]);
    }
  
    if (HasProp(propMap, "color") && propMap["color"].size() > 0) {
        unsigned long color = std::stoul(propMap["color"]);
        OH_Drawing_SetTextStyleColor(txtStyle, (uint32_t)color);
    }
    double fontSize = 14; // 默认的fontSize是14，是否从系统读取？
    if (HasProp(propMap, "fontSize") && propMap["fontSize"].size() > 0) {
        fontSize = std::stod(propMap["fontSize"]);
    }
    OH_Drawing_SetTextStyleFontSize(txtStyle, fontSize);
    if (HasProp(propMap, "fontWeight")) {
        int fontWeight = FontWeightToDrawing(propMap["fontWeight"]);
        OH_Drawing_SetTextStyleFontWeight(txtStyle, fontWeight);
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
    if (HasProp(propMap, "textDecorationColor") && propMap["textDecorationColor"].size() > 0) {
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

    // If font height is set, measure results for some special char will be wrong.
    // For example, ε (utf8 code: e0bdbdceb5). Measured height is less than drawn height.
    // OH_Drawing_SetTextStyleFontHeight(txtStyle, 1.25);

    if (HasProp(propMap, "fontFamily")) {
        const char *fontFamilies[] = {propMap["fontFamily"].c_str()};
        OH_Drawing_SetTextStyleFontFamilies(txtStyle, 1, fontFamilies);
    }
    int fontStyle = FONT_STYLE_NORMAL;
    if (HasProp(propMap, "fontStyle") && propMap["fontStyle"] == "italic") {
        fontStyle = FONT_STYLE_ITALIC;
    }
    OH_Drawing_SetTextStyleFontStyle(txtStyle, fontStyle);
    
    // use default locale,
    // If en is set, measure results for Chinese characters will be inaccurate.
    // OH_Drawing_SetTextStyleLocale(txtStyle, "zh");
    
    // OH_Drawing_SetTextStyleDecorationStyle(txtStyle, ); 
    // OH_Drawing_SetTextStyleDecorationThicknessScale(txtStyle, );
    if (HasProp(propMap, "letterSpacing") && propMap["letterSpacing"].size() > 0) {
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
    
        if (sIs_louzhu_str) {
          sIs_louzhu_str = false;
          std::string louzhu_text = propMap["text"];
          const char *p = louzhu_text.c_str();
          FOOTSTONE_DLOG(INFO) << "xxx hippy, louzhu text, p: " << p << ", len: " << louzhu_text.length();
        }
    
        std::string span_text = propMap["text"];
        if (span_text == "sheng") {
          sIs_louzhu_str = true;
        }

    
        OH_Drawing_TypographyHandlerAddText(handler_, propMap["text"].c_str());

#ifdef MEASURE_TEXT_LOG_RESULT
        logTextContent_ += "[span]";
        logTextContent_ += propMap["text"];
#endif
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
    if (HasProp(propMap, "width") && propMap["width"].size() > 0) {
        span.width = std::stod(propMap["width"]);
    }
    if (HasProp(propMap, "height") && propMap["height"].size() > 0) {
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
    if (HasProp(propMap, "margin") && propMap["margin"].size() > 0) {
        spanH.marginTop = std::stod(propMap["margin"]);
        spanH.marginBottom = std::stod(propMap["margin"]);
    }
    if (HasProp(propMap, "top") && propMap["top"].size() > 0) {
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
  
    auto typography = OH_Drawing_CreateTypography(handler_);
    double maxWidth = double(width) / density;
    if (maxWidth == 0 || std::isnan(maxWidth)) {
        // fix text measure width wrong when maxWidth is nan or 0
        maxWidth = std::numeric_limits<double>::max();
    }
    
    OH_Drawing_TypographyLayout(typography, maxWidth); // todo2 constraintWidth

    // double realWidth = OH_Drawing_TypographyGetLongestLine(typography); // 实际有像素的宽度
    // ret.width = fmax(realWidth, maxWidth);                   // 宽度
    
    // MATE 60, beta5, "新品" "商店" text cannot be fully displayed. So add 0.5.
    ret.width = ceil(OH_Drawing_TypographyGetLongestLine(typography) + 0.5);
    ret.height = OH_Drawing_TypographyGetHeight(typography); // 高度
    lineCount = OH_Drawing_TypographyGetLineCount(typography);

    double realHeight = CalcSpanPostion(typography, ret);
    ret.height = fmax(ret.height, realHeight);
  
    ret.isEllipsized = OH_Drawing_TypographyDidExceedMaxLines(typography);

    OH_Drawing_DestroyTypography(typography);

    OH_Drawing_DestroyTypographyHandler(handler_);
    OH_Drawing_DestroyFontCollection(fontCollection_);
    OH_Drawing_DestroyTypographyStyle(typoStyle_);

    if (ret.height < minLineHeight_) {
        ret.height = minLineHeight_;
    }
  
#ifdef MEASURE_TEXT_LOG_RESULT
    FOOTSTONE_DLOG(INFO) << "hippy text - measure result, maxWidth: " << maxWidth
      << ", result: (" << ret.width << ", " << ret.height << "), "
      << logTextContent_.c_str() << ", lineCount: " << lineCount;
#endif
  
    ret.width *= density;
    ret.height *= density;
    if (lineHeight_ != 0) {
        ret.height = lineHeight_ * density * (double)lineCount;
    
#ifdef MEASURE_TEXT_LOG_RESULT
        FOOTSTONE_DLOG(INFO) << "hippy text - lineHeight fix result, result height: " << lineHeight_ *  (double)lineCount;
#endif
    }
  
    return ret;
}
