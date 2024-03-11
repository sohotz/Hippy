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


private:
    OH_Drawing_TypographyStyle *typoStyle_;
    OH_Drawing_TypographyCreate *handler_;
};