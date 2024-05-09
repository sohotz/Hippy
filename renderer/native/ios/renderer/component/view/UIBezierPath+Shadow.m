//
//  UIBezierPath+VN.m
//  VideoNativeiPhone
//
//  Created by Norcy on 2018/4/13.
//  Copyright © 2018年 Norcy. All rights reserved.
//

#import "UIBezierPath+Shadow.h"

@implementation UIBezierPath (Shadow)
// Approximation of control point positions on a bezier to simulate a quarter of a circle.
// This is 1-kappa, where kappa = 4 * (sqrt(2) - 1) / 3
static const float kCircleControlPoint = 0.447715;

+ (instancetype)shadow_bezierPathWithRoundedRect:(CGRect)rect
                                         topLeft:(CGFloat)topLeftRadius
                                        topRight:(CGFloat)topRightRadius
                                      bottomLeft:(CGFloat)bottomLeftRadius
                                     bottomRight:(CGFloat)bottomRightRadius
{
    UIBezierPath *path = [UIBezierPath bezierPath];
    [path setLineWidth:0];
    [path moveToPoint:CGPointMake(rect.origin.x + topLeftRadius, rect.origin.y)];
    
    // +------------------+
    //  \\      top     //
    //   \\+----------+//
    [path addLineToPoint:CGPointMake(CGRectGetMaxX(rect) - topRightRadius, rect.origin.y)];
    if (topRightRadius > 0) {
        [path addCurveToPoint:CGPointMake(CGRectGetMaxX(rect), rect.origin.y + topRightRadius)
                controlPoint1:CGPointMake(CGRectGetMaxX(rect) - topRightRadius * kCircleControlPoint, rect.origin.y)
                controlPoint2:CGPointMake(CGRectGetMaxX(rect), rect.origin.y + topRightRadius * kCircleControlPoint)];
    }
    
    // +------------------+
    //  \\     top      //|
    //   \\+----------+// |
    //                |   |
    //                |rig|
    //                |ht |
    //                |   |
    //                 \\ |
    //                  \\|
    [path addLineToPoint:CGPointMake(CGRectGetMaxX(rect), CGRectGetMaxY(rect) - bottomRightRadius)];
    if (bottomRightRadius > 0) {
        [path addCurveToPoint:CGPointMake(CGRectGetMaxX(rect) - bottomRightRadius, CGRectGetMaxY(rect))
                controlPoint1:CGPointMake(CGRectGetMaxX(rect), CGRectGetMaxY(rect) - bottomRightRadius * kCircleControlPoint)
                controlPoint2:CGPointMake(CGRectGetMaxX(rect) - bottomRightRadius * kCircleControlPoint, CGRectGetMaxY(rect))];
    }
    
    // +------------------+
    //  \\     top      //|
    //   \\+----------+// |
    //                |   |
    //                |rig|
    //                |ht |
    //                |   |
    //   //+----------+\\ |
    //  //    bottom    \\|
    // +------------------+
    [path addLineToPoint:CGPointMake(rect.origin.x + bottomLeftRadius, CGRectGetMaxY(rect))];
    if (bottomLeftRadius > 0) {
        [path addCurveToPoint:CGPointMake(rect.origin.x, CGRectGetMaxY(rect) - bottomLeftRadius)
                controlPoint1:CGPointMake(rect.origin.x + bottomLeftRadius * kCircleControlPoint, CGRectGetMaxY(rect))
                controlPoint2:CGPointMake(rect.origin.x, CGRectGetMaxY(rect) - bottomLeftRadius * kCircleControlPoint)];
    }
    
    // +------------------+
    // |\\     top      //|
    // | \\+----------+// |
    // |   |          |   |
    // |lef|          |rig|
    // |t  |          |ht |
    // |   |          |   |
    // | //+----------+\\ |
    // |//    bottom    \\|
    // +------------------+
    [path addLineToPoint:CGPointMake(rect.origin.x, rect.origin.y + topLeftRadius)];
    if (topLeftRadius > 0) {
        [path addCurveToPoint:CGPointMake(rect.origin.x + topLeftRadius, rect.origin.y)
                controlPoint1:CGPointMake(rect.origin.x, rect.origin.y + topLeftRadius * kCircleControlPoint)
                controlPoint2:CGPointMake(rect.origin.x + topLeftRadius * kCircleControlPoint, rect.origin.y)];
    }
    
    
    return path;
}

@end
